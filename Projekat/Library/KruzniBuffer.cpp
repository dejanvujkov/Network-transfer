#include "KruzniBuffer.h"
#include <stdlib.h>
#include <string.h>

void rInitBuffer(Kruzni_Buffer * buffer, int size)
{
	buffer->buffer_start = (char*)malloc(size);
	buffer->buffer_end = buffer->buffer_start + size;

	buffer->head = buffer->buffer_start;
	buffer->tail = buffer->buffer_start;

	buffer->free = size;
	buffer->taken = 0;
}

void rFreeBuffer(Kruzni_Buffer * buffer)
{
	free(buffer->buffer_start);
}

int rPush(Kruzni_Buffer * buffer, char * data, int size)
{
	if (size <= buffer->free)
	{
		if ((buffer->buffer_end - buffer->head) >= size)
		{
			memcpy(buffer->head, data, size);

			buffer->head += size;
		}
		else
		{
			int temp = buffer->buffer_end - buffer->head;

			memcpy(buffer->head, data, temp);
			memcpy(buffer->buffer_start, data + temp, size - temp);

			buffer->head = buffer->buffer_start + (size - temp);
		}

		buffer->free -= size;
		buffer->taken += size;

		return 0;
	}

	return 1;
}

int rPop(Kruzni_Buffer * buffer, char * data, int size)
{
	if (size <= buffer->taken)
	{
		if ((buffer->buffer_end - buffer->tail) >= size)
		{
			memcpy(data, buffer->tail, size);

			buffer->tail += size;
		}
		else
		{
			int temp = buffer->buffer_end - buffer->tail;

			memcpy(data, buffer->tail, temp);
			memcpy(data + temp, buffer->buffer_start, size - temp);

			buffer->tail = buffer->buffer_start + (size - temp);
		}

		buffer->free += size;
		buffer->taken -= size;

		return 0;
	}

	return 1;
}

int rDelete(Kruzni_Buffer * buffer, int size)
{
	if (size <= buffer->taken)
	{
		if ((buffer->buffer_end - buffer->tail) >= size)
		{
			buffer->tail += size;
		}
		else
		{
			int temp = buffer->buffer_end - buffer->tail;
			buffer->tail = buffer->buffer_start + (size - temp);
		}

		buffer->free += size;
		buffer->taken -= size;

		return 0;
	}

	return 1;
}
