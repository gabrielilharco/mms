//
// Created by gabrielilharco on 17/06/16.
//

#ifndef MMS_PAGE_REPLACEMENT_MANAGER_H
#define MMS_PAGE_REPLACEMENT_MANAGER_H

#define WRITE   1;
#define READ    2;

class PageReplacementManager {
public:
    // update function based on a page location on the memory and the reason for the update
    virtual void update(int page_index, int reason) = 0;
    // returns the location of the block to be replaced and does the necessary updates on the manager
    virtual int replace_block() = 0; 
};

#endif //MMS_PAGE_REPLACEMENT_MANAGER_H
