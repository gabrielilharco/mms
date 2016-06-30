//
// Created by gabrielilharco on 17/06/2016
// 

#include <stdio.h>
#include "Memory.h"

int main () {
  Memory * null = NULL;
  Memory * RAM = new Memory(
    (long long) 4294967296, // long long size (2^32) 
    64,                     // long long block_size
    4194304,                // long long associativity_set_size
    null,                   // Memory * lower_level_memory
    60,                     // double word_access_time
    0,                      // double tag_compare_time
    WRITE_BACK,             // int write_miss_policy
    WRITE_ALLOCATE          // int write_hit_policy
  );
  Memory * L2 = new Memory(
    4194304,                // long long size 
    64,                     // long long block_size
    16,                     // long long associativity_set_size
    RAM,                    // Memory * lower_level_memory
    4,                      // double word_access_time
    2,                      // double tag_compare_time
    WRITE_BACK,             // int write_miss_policy
    NO_WRITE_ALLOCATE       // int write_hit_policy
  );
  Memory * L1 = new Memory(
    32768,                  // long long size  
    64,                     // long long block_size
    8,                      // long long associativity_set_size
    L2,                     // Memory * lower_level_memory
    2,                      // double word_access_time
    1,                      // double tag_compare_time
    WRITE_TROUGH,           // int write_miss_policy
    WRITE_ALLOCATE          // int write_hit_policy
  );

  FILE * file = fopen("gcc.trace", "r");
  long long address;
  char rw;
  double time = 0;
  bool success;
  while (fscanf (file, "%llx %c", &address, &rw) != EOF) {
    if (rw == 'W' || rw == 'w') {
      L1->writePage(address, success, time);
    }
    else {
      L1->getBlock(address, success, time);
    }
  }
  printf ("\n*******************************\nTotal time: %g\n", time);
  printf ("L1 miss rate: %d of %d (%.2lf%%)\n", L1->misses, L1->total_op, 100*(1-(double)L1->misses/L1->total_op));
  printf ("L2 miss rate: %d of %d (%.2lf%%)\n", L2->misses, L2->total_op, 100*(1-(double)L2->misses/L2->total_op));
  return 0;
}
