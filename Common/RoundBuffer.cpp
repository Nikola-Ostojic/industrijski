#include "RoundBuffer.h"


RoundBuffer* createRoundBuffer(void)
{
	unsigned int size = 4;
	RoundBuffer* rbuffer = (RoundBuffer*)malloc(sizeof(rbuffer));

	if (rbuffer != NULL)
	{
		rbuffer->entries = (Node**)malloc(sizeof(Node*) * size);
		if (rbuffer->entries != NULL)
		{
			rbuffer->size = size;
			rbuffer->head = 0;
			rbuffer->tail = 0;
			rbuffer->isFull = false;
			InitializeCriticalSection(&(rbuffer->criticalSection));
		}
		else
		{
			free(rbuffer);
			rbuffer = NULL;
		}
	}

	return rbuffer;
}

void deleteRBuffer(RoundBuffer* rBuffer)
{
	if (rBuffer != NULL)
	{
		free(rBuffer->entries);
		DeleteCriticalSection(&(rBuffer->criticalSection));
		free(rBuffer);
	}

	return;
}

void resizeRBuffer(RoundBuffer* rBuffer)
{
	EnterCriticalSection(&(rBuffer->criticalSection));

	Node** temp = (Node**)malloc(sizeof(Node*) * rBuffer->size * 2);

	if (temp != NULL)
	{
		unsigned int i = 0;
		unsigned int h = rBuffer->head;
		do {
			temp[i] = rBuffer->entries[h];
			h++;
			if (h == rBuffer->size)
			{
				h = 0;
			}

			i++;
		} while (h != rBuffer->tail);

		free(rBuffer->entries);
		rBuffer->entries = temp;
		rBuffer->head = 0;
		rBuffer->tail = rBuffer->size;
		rBuffer->size = rBuffer->size * 2;
		rBuffer->isFull = false;
	}

	LeaveCriticalSection(&(rBuffer->criticalSection));

	return;
}

bool isEmpty(RoundBuffer* rBuffer)
{
	EnterCriticalSection(&(rBuffer->criticalSection));

	if (rBuffer->head == rBuffer->tail)
	{
		if (rBuffer->isFull == false)
		{
			LeaveCriticalSection(&(rBuffer->criticalSection));
			return true;
		}
	}

	LeaveCriticalSection(&(rBuffer->criticalSection));

	return false;
}

bool insertInRBuffer(RoundBuffer* rBuffer, Node* node)
{
	EnterCriticalSection(&(rBuffer->criticalSection));

	bool result;

	if (rBuffer->isFull == true)
	{
		resizeRBuffer(rBuffer);
		if (rBuffer->isFull == true)
		{
			result = false;
		}
	}

	if (rBuffer->isFull == false)
	{
		rBuffer->entries[rBuffer->tail] = node;
		rBuffer->tail++;

		if (rBuffer->tail == rBuffer->size)
		{
			rBuffer->tail = 0;
		}

		if (rBuffer->tail == rBuffer->head)
		{
			rBuffer->isFull = true;
		}
	}

	LeaveCriticalSection(&(rBuffer->criticalSection));

	return true;
}

Node* removeFromRBuffer(RoundBuffer* rBuffer)
{
	EnterCriticalSection(&(rBuffer->criticalSection));

	Node* node = NULL;

	if (isEmpty(rBuffer) == false)
	{
		if (rBuffer->isFull == true)
		{
			rBuffer->isFull = false;
		}

		node = rBuffer->entries[rBuffer->head];
		rBuffer->head++;

		if (rBuffer->head == rBuffer->size)
		{
			rBuffer->head = 0;
		}
	}

	LeaveCriticalSection(&(rBuffer->criticalSection));

	return node;
}

Node* lookHead(RoundBuffer* rBuffer)
{
	EnterCriticalSection(&(rBuffer->criticalSection));

	Node* node = NULL;

	if (isEmpty(rBuffer) == false)
	{
		node = rBuffer->entries[rBuffer->head];
	}

	LeaveCriticalSection(&(rBuffer->criticalSection));

	return node;
}

unsigned int getSize(RoundBuffer* rBuffer)
{
	EnterCriticalSection(&(rBuffer->criticalSection));

	unsigned int count;

	if (isEmpty(rBuffer) == true)
	{
		count = 0;
	}
	else if (rBuffer->isFull == true)
	{
		count = rBuffer->size;
	}
	else if (rBuffer->tail > rBuffer->head)
	{
		count = rBuffer->tail - rBuffer->head;
	}
	else
	{
		count = rBuffer->size - rBuffer->head;

		if (rBuffer->tail > 0)
		{
			count = count + rBuffer->tail - 1;
		}
	}

	LeaveCriticalSection(&(rBuffer->criticalSection));

	return count;
}