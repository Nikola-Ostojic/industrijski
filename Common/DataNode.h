#pragma once

#ifndef DataNode_H
#define DataNode_H

#include "Limitations.h"

typedef struct Node {
	int processId;
	int senderType;
	char value[MAX_BUFFER];
	
}Node;

#endif  

