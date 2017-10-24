#pragma once

typedef struct Kruzni_Buffer
{
	char* buffer_start;
	char* buffer_end;
	char* head;
	char* tail;
	int taken;
	int free;
}Kruzni_Buffer;

void rInitBuffer(Kruzni_Buffer* buffer, int size);
void rFreeBuffer(Kruzni_Buffer* buffer);

int rPush(Kruzni_Buffer * buffer, char* data, int size);
int rPop(Kruzni_Buffer * buffer, char* data, int size);
int rDelete(Kruzni_Buffer * buffer, int size);