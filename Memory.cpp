//
// Created by gabrielilharco on 17/06/2016
// 

#include "Memory.h"
#include <stdio.h>

Block::Block(long long address) : address(address) {
  dirty = false;
}

Block::Block() : address(-1) {
  dirty = false;
}

Memory::Memory (
  long long size, 
  long long block_size, 
  int associativity_set_size,
  Memory * lower_level_memory,
  double word_access_time,
  double tag_compare_time,
  int write_miss_policy,
  int write_hit_policy
) :
  size(size),
  block_size(block_size),
  associativity_set_size(associativity_set_size),
  lower_level_memory(lower_level_memory),
  word_access_time(word_access_time),
  tag_compare_time(tag_compare_time),
  write_miss_policy(write_miss_policy),
  write_hit_policy(write_hit_policy)
{
  misses = 0;
  total_op = 0;
  // initializing blocks
  if (lower_level_memory) {
    blocks = vector<Block>(size/block_size);
    for (int i = 0; i < size/block_size; i++) {
        blocks[i] = Block(-1);
    }
  }
  // seting sizes of tag, set and offset
  // there are <block_size> bytes in a cache block, 
  // so offset size should be log2(<block_size>)
  add_offset_size = log2(block_size);
  // each set contains <associativity_set_size> cache blocks, 
  // so the size of a set is <associativity_set_size>*<block_size>
  // there are <size> bytes in the entire cache, so there are
  // <size>/(<associativity_set_size>*<block_size>) sets
  n_sets = size/(associativity_set_size*block_size);
  add_set_size = log2(n_sets);
  // finally, the tag is what remains
  add_tag_size = ADDRESS_SIZE-add_set_size-add_offset_size;
  // initializing indexes for substituition
  for (int i = 0; i < n_sets; i++)
    current_set_index.push_back(0);
}

Block Memory::getBlock(long long address, bool& success, double& time) {
  // update operations statistic
  total_op++;
 
  // if memory is a cache memory (not the lowest level)
  if (lower_level_memory) {
    // check if the block exists in this memory 
    int block_index = getBlockFromCurrentMemory(address, success, time);
    if (success) {
      // cache hit, just update variables and return block
      time += word_access_time + tag_compare_time;
      return blocks[block_index];
    }
    // cache miss, update misses statistic
    misses++;
    // cache miss, we need to update the memory
    // get block from lower level memory
    Block block = lower_level_memory->getBlock(address, success, time);
    // store block in current memory
    // also deals with the case of a block leaving current memory;
    substitute(block, success, time, false);
    // return the block
    return block;
  }
  // lowest level memory
  time += word_access_time;
  return Block(address);

}

Block Memory::writePage(long long address, bool& success, double& time) {
  // update operations statistic
  total_op++;
  if (lower_level_memory) {
    // check if the block exists in this memory 
    int block_index = getBlockFromCurrentMemory(address, success, time);
    if (success) {
      //cache hit 
      Block block = blocks[block_index];
      // make block dirty
      blocks[block_index].dirty = true;
      // if memory is write trough, we need to write it in lower level memory
      if (write_hit_policy == WRITE_TROUGH && lower_level_memory)
        lower_level_memory->writePage(address, success, time);
      else 
        time += word_access_time + tag_compare_time;
      // return the block
      return block;
    }
    // cache miss
    misses++;
    // get block from lower level memory
    Block block = lower_level_memory->writePage(address, success, time);
    // if write miss policy is write allocate, store block in current memory
    // also deals with the case of a block leaving current memory;
    if (write_miss_policy == WRITE_ALLOCATE) {
      // substitute
      substitute(block, success, time, true);
    }
    // return the block
    return block;
  }
  // lowest level memory
  time += word_access_time;
  return Block(address);
}

void Memory::substitute (Block block, bool& success, double& time, bool dirty) {
  // set number of block address
  long long set_number = getSet(block.address);
  // beginning of associativity set
  // the start address of the set is getSet(address) * set_size
  // to get the starting block index, we divide it by the block_size
  long long start = set_number*(size/n_sets)/block_size;
  // block index
  long long block_index = start + (current_set_index[set_number] % (associativity_set_size));
  // get block to be substituted
  Block old_block = blocks[block_index];

  // put new block 
  blocks[block_index].address = block.address;
  blocks[block_index].dirty = dirty;

  // if write hit policy is write-back, check if block is dirty
  if (write_hit_policy == WRITE_BACK && old_block.dirty && lower_level_memory && current_set_index[set_number] >= block_size) {
    // we need to make sure lower level memory updates it
    lower_level_memory->writePage(block.address, success, time);
  }

  // increment current_set_index;
  current_set_index[set_number]++;
}

// gets index of block (if it exists) on current memory
int Memory::getBlockFromCurrentMemory(long long address, bool& success, double& time) {
  // beginning of associativity set
  // the start address of the set is getSet(address) * set_size
  // to get the starting block index, we divide it by the block_size
  long long start = (getSet(address)*(size/n_sets))/block_size;
  // end of associativity set (there are <associativity_set_size> blocks in a set)
  long long end = start + associativity_set_size;
  // we need to check for every possible position 
  for (long long i = start; i < end; i++) {
    if (getTag(blocks[i].address) == getTag(address)) {
      // success, just return the block
      success = true;
      return i;
    }
  }
  // block not in current memory. Return dummy block
  success = false;
  return -1;
}

// returns the tag from an address
long long Memory::getTag(long long address) {
  return address >> (add_offset_size+add_set_size);
}

// returns the set from an address
long long Memory::getSet(long long address) {
  long long tag = (getTag(address) << (add_offset_size+add_set_size));
  address -= tag;
  return address >> add_offset_size;
}