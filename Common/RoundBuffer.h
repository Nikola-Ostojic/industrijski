#pragma once

#ifndef Roundbuff_H
#define Roundbuff_H

#include "DataNode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct RoundBuffer {
	unsigned int head;
	unsigned int tail;
	bool isFull;
	unsigned int size;
	Node **entries;
	CRITICAL_SECTION criticalSection;
}RoundBuffer;

RoundBuffer *createRoundBuffer(void);
void deleteRBuffer(RoundBuffer *rBuffer);
void resizeRBuffer(RoundBuffer *rBuffer);
bool isEmpty(RoundBuffer *rBuffer);
bool insertInRBuffer(RoundBuffer *rBuffer, Node *node);
Node *removeFromRBuffer(RoundBuffer *rBuffer);
Node *lookHead(RoundBuffer *rBuffer);
unsigned int getSize(RoundBuffer *rBuffer);

#endif 

