#pragma once

#ifndef Serializer_H
#define Serializer_H

#define _CRT_SECURE_NO_WARNINGS

#include "DataNode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* Serialize(Node* node);
Node* Deserialize(char *buffer);

#endif // H