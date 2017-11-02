#pragma once
#include "header.h"

void rInitBuffer(Kruzni_Buffer* buffer, int size);
void rFreeBuffer(Kruzni_Buffer* buffer);
int rResize(Kruzni_Buffer* buffer, int size);

int rPush(Kruzni_Buffer * buffer, char* data, int size);
int rPop(Kruzni_Buffer * buffer, char* data, int size);
int rRead(Kruzni_Buffer * buffer, char* data, int size);
int rDelete(Kruzni_Buffer * buffer, int size);
int rResize(Kruzni_Buffer *buffer, int size);