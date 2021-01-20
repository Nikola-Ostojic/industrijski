#pragma once

#ifndef DataNode_H
#define DataNode_H

#include <time.h>
#include "Limitations.h"

typedef struct Node {
	int processId;
	tm timeStamp;
	char value[MAX_BUFFER];
}Node;

#endif // !ClientNode_H

