//
// Created by gabrielilharco on 17/06/16.
//

#ifndef MMS_MEMORY_H
#define MMS_MEMORY_H

#include <vector>

using namespace std;

// Write miss techniques
#define WRITE_ALLOCATE 		1;
#define NO_WRITE_ALLOCATE 	2;

// Write hit techniques
#define WRITE_TROUGH 		1;
#define WRITE_BACK 			2;


class Memory {
private:
	// returns the block containing the requested page and updates a success variable
	vector<int> get_block(int page_address, bool& success);

public:
    // memory size in bytes
    int size; 
    // block size in bytes
    int block_size; 
    // N for a N-way set associative policy
    int associativity_set_size;  // 1 for diret mapping and <size> for fully associative
    // page substituition manager
    PageReplacementManager * replacement_manager;
    // lower level memory
    Memory * lower_level_memory;
};

#endif //MMS_MEMORY_H
