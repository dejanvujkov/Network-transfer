#include "header.h"

// Inicijalizacija kruznog buffera
void rInitBuffer(Kruzni_Buffer * buffer, int size)
{
	buffer->buffer_start = (char*)malloc(size);
	buffer->buffer_end = buffer->buffer_start + size;

	buffer->head = buffer->buffer_start;
	buffer->tail = buffer->buffer_start;

	buffer->free = size;
	buffer->taken = 0;
}

// Oslobadjanje buffera
void rFreeBuffer(Kruzni_Buffer * buffer)
{
	free(buffer->buffer_start);
}

int rResize(Kruzni_Buffer* buffer, int size)
{
	char* temp = (char*)malloc(size);
	if (temp == NULL)
		return -1;

	if (buffer->buffer_end - buffer->tail > buffer->taken)
	{
		// Ako nema prelamanja
		memcpy(temp, buffer->tail, buffer->taken);

		free(buffer->buffer_start);
		buffer->buffer_start = temp;
	}
	else
	{
		// Ako ima prelamanja
		memcpy(temp, buffer->tail, buffer->buffer_end - buffer->tail);
		memcpy(temp + (buffer->buffer_end - buffer->tail), buffer->buffer_start, buffer->taken - (buffer->buffer_end - buffer->tail));

		free(buffer->buffer_start);
		buffer->buffer_start = temp;
	}

	buffer->head = buffer->buffer_start + buffer->taken;
	buffer->tail = buffer->buffer_start;
	buffer->free = size - buffer->taken;
}

int rPush(Kruzni_Buffer * buffer, char * data, int size)
{
	// Ako ne moze da stane vrati gresku
	if (size > buffer->free)
		return -1;

	// Velicina podataka koja se dodaje u buffer
	int tempSize;
	if (size <= buffer->free)
		tempSize = size;
	else
		tempSize = buffer->free;

	// Ako nema prelamanja podataka samo se kopira
	if ((buffer->buffer_end - buffer->head) >= tempSize)
	{
		memcpy(buffer->head, data, tempSize);

		buffer->head += tempSize;
	}
	// Ako ima prelamanja, podaci se dele na pocetak i kraj
	else
	{
		int temp = buffer->buffer_end - buffer->head;

		memcpy(buffer->head, data, temp);
		memcpy(buffer->buffer_start, data + temp, tempSize - temp);

		buffer->head = buffer->buffer_start + (tempSize - temp);
	}

	buffer->free -= tempSize;
	buffer->taken += tempSize;

	return tempSize;
}

int rPop(Kruzni_Buffer * buffer, char * data, int size)
{
	// Maksimalna velicina podataka koja se uzima
	int tempSize;
	if (size <= buffer->taken)			// Ako se trazi bar onoliko koliko ima / 100 : 200 -> 100
		tempSize = size;
	else								// Ako se trazi vise nego sto ima / 100 : 20 -> 20
		tempSize = buffer->taken;

	// Ako ima prelamanje buffera
	if ((buffer->buffer_end - buffer->tail) >= tempSize)
	{
		memcpy(data, buffer->tail, tempSize);
		buffer->tail += tempSize;
	}
	else
	{
		int temp = buffer->buffer_end - buffer->tail;
		memcpy(data, buffer->tail, temp);
		memcpy(data + temp, buffer->buffer_start, tempSize - temp);
		buffer->tail = buffer->buffer_start + (tempSize - temp);
	}

	buffer->free += tempSize;
	buffer->taken -= tempSize;

	return tempSize;
}

int rRead(Kruzni_Buffer * buffer, char * data, int size)
{
	// Maksimalna velicina podataka koja se uzima
	int tempSize;
	if (size <= buffer->taken)			// Ako se trazi bar onoliko koliko ima / 100 : 200 -> 100
		tempSize = size;
	else								// Ako se trazi vise nego sto ima / 100 : 20 -> 20
		tempSize = buffer->taken;

	// Ako ima prelamanje buffera
	if ((buffer->buffer_end - buffer->tail) >= tempSize)
	{
		memcpy(data, buffer->tail, tempSize);
	}
	else
	{
		int temp = buffer->buffer_end - buffer->tail;

		memcpy(data, buffer->tail, temp);
		memcpy(data + temp, buffer->buffer_start, tempSize - temp);
	}

	return tempSize;
}

int rDelete(Kruzni_Buffer * buffer, int size)
{
	// Maksimalna velicina podataka koja se brise
	int tempSize;
	if (size <= buffer->taken)			// Ako se trazi bar onoliko koliko ima / 100 : 200 -> 100
		tempSize = size;
	else								// Ako se trazi vise nego sto ima / 100 : 20 -> 20
		tempSize = buffer->taken;

	// Ako ima prelamanje buffera
	if ((buffer->buffer_end - buffer->tail) >= tempSize)
	{
		buffer->tail += tempSize;
	}
	else
	{
		int temp = buffer->buffer_end - buffer->tail;
		buffer->tail = buffer->buffer_start + (tempSize - temp);
	}

	buffer->free += tempSize;
	buffer->taken -= tempSize;

	return tempSize;
}
