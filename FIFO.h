//
// Created by gabrielilharco on 17/06/16.
//

#ifndef MMS_FIFO_H
#define MMS_FIFO_H

#include "PageReplacementManager.h"
#include "Memory.h"

// First in First out policy
// implemented with an index 
class FIFO : PageReplacementManager {
private:
	int current_index; 
	Memory * memory;
public:
	FIFO();
	~FIFO();
};

#endif //MMS_FIFO_H
