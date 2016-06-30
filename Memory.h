//
// Created by gabrielilharco on 17/06/16.
//

#ifndef MMS_MEMORY_H
#define MMS_MEMORY_H

#include <vector>
#include <math.h>

using namespace std;

// Write miss techniques
#define WRITE_ALLOCATE      1
#define NO_WRITE_ALLOCATE   2

// Write hit techniques
#define WRITE_TROUGH        1
#define WRITE_BACK          2

#define ADDRESS_SIZE       64

class Block {
public:
  bool dirty;
  long long address;

  Block(long long address);
  Block();
};

class Memory {
private:
  // last operation status and time
  bool last_op_status;
  long long last_op_time;
  // current index for each set (for block replacement) 
  vector<long long> current_set_index;
  // tag, set and offset sizes of the address, in bits
  int add_tag_size, add_set_size, add_offset_size;
  long long n_sets;

  // private nethods
  void substitute (Block block, bool& success, double& time, bool dirty);
  int getBlockFromCurrentMemory(long long block_address, bool& success, double& time);
  long long getTag(long long address);
  long long getSet(long long address);

public:
  // statistic variables
  int misses;
  int total_op;

  // memory blocks
  vector<Block> blocks;

  // memory size in bytes
  long long size;
  // block size in bytes
  long long block_size; 
  // main memory size in bytes
  long long main_memory_size;
  // N for a N-way set associative policy
  int associativity_set_size;  // 1 for diret mapping and <size> for fully associative
  // lower level memory
  Memory * lower_level_memory;
  // time necessary to access a word in the memory
  double word_access_time;
  // time necessary to compare two tags
  double tag_compare_time;
  // write miss policy
  int write_miss_policy;
  // write hit policy
  int write_hit_policy;

  Memory(
    long long size, 
    long long block_size, 
    int associativity_set_size,
    Memory * lower_level_memory,
    double word_access_time,
    double tag_compare_time,
    int write_miss_policy,
    int write_hit_policy
  );
  Block getBlock(long long address, bool& success, double& time);
  Block writePage(long long address, bool& success, double& time);
  
};

#endif //MMS_MEMORY_H
