//
// Created by gabrielilharco on 17/06/2016
// 

#include "Memory.h"
#include <stdio.h>

Block::Block(long long address) : address(address) {
	dirty = false;
}

Block::Block() : address(0) {
	dirty = false;
}

Memory::Memory (
	long long size, 
	long long block_size, 
	long long main_memory_size, 
	int associativity_set_size,
	Memory * lower_level_memory,
	double word_access_time,
	double tag_compare_time,
	int write_miss_policy,
	int write_hit_policy
) :
	size(size),
	block_size(block_size),
	main_memory_size(main_memory_size),
	associativity_set_size(associativity_set_size),
	lower_level_memory(lower_level_memory),
	word_access_time(word_access_time),
	tag_compare_time(tag_compare_time),
	write_miss_policy(write_miss_policy),
	write_hit_policy(write_hit_policy)
{
	blocks = vector<Block>(size/block_size);
	// initializing blocks
	for (int i = 0; i < size/block_size; i++) {
		blocks[i] = Block(getBlockAddress(block_size*i));
	}
	// initializing indexes for substituition
	for (int i = 0; i < (size/block_size)/associativity_set_size; i++)
		current_set_index.push_back(0);
}

Block Memory::getBlock(long long page_address, bool& success, double& time) {
	//printf("getting block address\n");
	// get block address from page address
	long long block_address = getBlockAddress(page_address);
	//printf ("getting block\n"); fflush (stdin);
	// check if the block exists in this memory 
	Block block = getBlockFromCurrentMemory(block_address, success, time);
	//printf ("got block \n"); fflush (stdin);
	if (success) {
		// printf("cache hit\n"); fflush(stdin);
		// cache hit, just update variables and return block
		return block;
	}
	// if memory is a cache memory (not the lowest level)
	if (lower_level_memory) {
		//printf("cache miss\n"); fflush(stdin);
		// cache miss, we need to update the memory
		// get block from lower level memory
		Block block = lower_level_memory->getBlock(page_address, success, time);
		// store block in current memory
		// also deals with the case of a block leaving current memory;
		substitute(block, success, time);
		// return the block
		return block;
	}
	// lowest level memory
	// printf ("RAM hit\n"); fflush(stdin);
	return Block(block_address);

}

Block Memory::writePage(long long page_address, bool& success, double& time) {
	// get block address from page address
	long long block_address = getBlockAddress(page_address);
	// check if the block exists in this memory 
	Block block = getBlockFromCurrentMemory(block_address, success, time);
	if (success) {
		//printf("cache hit\n"); fflush(stdin);
		// cache hit, just update the page and mark it as dirty
		block.dirty = true;
		// if memory is write trough, we need to write it in lower level memory
		if (write_hit_policy == WRITE_TROUGH && lower_level_memory)
			lower_level_memory->writePage(page_address, success, time);
		// return the block
		return block;
	}
	// cache miss
	//printf("cache miss\n"); fflush(stdin);
	if (lower_level_memory) {
		Block block2 = lower_level_memory->writePage(page_address, success, time);
		// if write miss policy is write allocate, store block in current memory
		// also deals with the case of a block leaving current memory;
		if (write_miss_policy == WRITE_ALLOCATE) {
			// get block from lower level memory
			// substitute
			substitute(block2, success, time);
		}
		// return the block
		return block2;
	}
	//printf ("RAM hit\n"); fflush(stdin);
	return Block(block_address);
}

void Memory::substitute (Block block, bool& success, double& time) {
	//printf ("substituting block %llx\n", block.address);
	// number of sets on current memory
	//printf ("%llx, %llx, %llx\n", size, );
	long long n_sets = (size/block_size)/associativity_set_size;
	//printf ("number of sets: %lld\n", n_sets);
	// get in which set this blocks belong to
	long long set_address =  (block.address*block_size)/(main_memory_size/n_sets);
	//printf ("set addr: %lld\n", set_address);
	// beginning of associativity set
	long long start = set_address*associativity_set_size;
	//printf ("start: %lld\n", start);
	// block index
	long long block_index = start + current_set_index[set_address]%block_size;
	//printf ("block_index: %lld\n", block_index);
	//printf ("----------------------------------\n");
	// get block to be substituted
	Block old_block = blocks[block_index];

	// put new block 
	blocks[block_index] = block;
	// a new block is never dirty 
	blocks[block_index].dirty = false;
	// increment current_set_index;

	// if write hit policy is write-back, check if block is dirty
	if (write_hit_policy == WRITE_BACK && old_block.dirty && lower_level_memory && current_set_index[set_address] >= block_size) {
		// we need to make sure lower level memory updates it
		lower_level_memory->writePage(getSamplePage(old_block.address), success, time);
	}

	//printf ("putting new block here: %llx\n", block_index);
	current_set_index[set_address]++;

}

// gets block (if it exists) on current memory
Block Memory::getBlockFromCurrentMemory(long long block_address, bool& success, double& time) {
	// number of sets on current memory
	long long n_sets = (size/block_size)/associativity_set_size;
	// get in which set this blocks belong to
	long long set_address =  (block_address*block_size)/(main_memory_size/n_sets);
	// beginning of associativity set
	long long start = set_address*associativity_set_size;
	// end of associativity set
	long long end = start + block_size;
	//printf("\nchecking from %lld to %lld\n", start, end);
	// we need to check for every possible position 
	for (long long i = start; i < end; i++) {
		// update time
		time += tag_compare_time;
		if (blocks[i].address == block_address) {
			// success, just return the block
			//printf ("found\n\n");
			success = true;
			return blocks[i];
		}
	}
	//printf ("not found\n\n");
	// block not in current memory. Return dummy block
	success = false;
	Block block(block_size);
	return block;
}

// gets the address of the block that colong longains a page passed as parameter
long long Memory::getBlockAddress(long long page_address) {
	long long number_of_blocks = main_memory_size/block_size;
	int tag_size = log2(number_of_blocks);
	int memory_size = log2(main_memory_size);
	return page_address >> memory_size-tag_size;
}

long long Memory::getSamplePage(long long block_address) {
	long long n_sets = (size/block_size)/associativity_set_size;
	return (size/n_sets) * block_address/(main_memory_size/n_sets);
}