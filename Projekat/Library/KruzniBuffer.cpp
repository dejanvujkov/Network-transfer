/**
 * KruzniBuffer.cpp Omogucuje metode za upravljanje kruznim bufferom
 */

#include "header.h"

void rInitBuffer(Kruzni_Buffer * buffer, int size)
{
	/* Alociranje memorije za buffer */
	buffer->buffer_start = (char*)malloc(size);

	/* Pomeranje pokazivaca */
	buffer->buffer_end = buffer->buffer_start + size;
	buffer->head = buffer->buffer_start;
	buffer->tail = buffer->buffer_start;

	/* Promena stanja buffera */
	buffer->free = size;
	buffer->taken = 0;
}

void rFreeBuffer(Kruzni_Buffer * buffer)
{
	/* Oslobadjanje memorije buffera */
	free(buffer->buffer_start);
}

int rResize(Kruzni_Buffer* buffer, int size)
{
	/* Pokusaj zauzimanja memorije za buffer */
	char* temp = (char*)malloc(size);
	if (temp == NULL)
		return -1;

	/* Ako nema prelamanja buffera kopiraju se postojeci podaci od jednom */
	if (buffer->buffer_end - buffer->tail > buffer->taken)
	{
		memcpy(temp, buffer->tail, buffer->taken);
	}
	/* Ako ima prelamanja buffera kopiraju se postojeci podaci iz dva puta */
	else
	{
		memcpy(temp, buffer->tail, buffer->buffer_end - buffer->tail);
		memcpy(temp + (buffer->buffer_end - buffer->tail), buffer->buffer_start, buffer->taken - (buffer->buffer_end - buffer->tail));
	}

	/* Oslobadjanje stare lokacije */
	free(buffer->buffer_start);

	/* Prebacivanje pokazivaca */
	buffer->buffer_start = temp;
	buffer->buffer_end = buffer->buffer_start + size;
	buffer->head = buffer->buffer_start + buffer->taken;
	buffer->tail = buffer->buffer_start;
	buffer->free = size - buffer->taken;

	return 0;
}

int rPush(Kruzni_Buffer * buffer, char * data, int size)
{
	/* Ako nema dovoljno prostora vrati gresku */
	if (size > buffer->free)
		return -1;

	/* Velicina podataka koja se ubacuje u buffer */
	int tempSize;
	if (size <= buffer->free)
		tempSize = size;
	else
		tempSize = buffer->free;

	/* Ako nema prelamanja podataka kopiraju se podaci od jednom */
	if ((buffer->buffer_end - buffer->head) >= tempSize)
	{
		memcpy(buffer->head, data, tempSize);

		buffer->head += tempSize;
	}
	/* Ako ima prelamanja podataka kopiraju se podaci iz dva puta */
	else
	{
		int temp = buffer->buffer_end - buffer->head;

		memcpy(buffer->head, data, temp);
		memcpy(buffer->buffer_start, data + temp, tempSize - temp);

		buffer->head = buffer->buffer_start + (tempSize - temp);
	}

	/* Promena stanja buffera */
	buffer->free -= tempSize;
	buffer->taken += tempSize;

	return tempSize;
}

int rPop(Kruzni_Buffer * buffer, char * data, int size)
{
	/* Velicina podataka koja se uzima iz buffera */
	int tempSize;
	if (size <= buffer->taken)
		tempSize = size;
	else
		tempSize = buffer->taken;

	/* Ako nema prelamanja podataka kopiraju se podaci od jednom */
	if ((buffer->buffer_end - buffer->tail) >= tempSize)
	{
		memcpy(data, buffer->tail, tempSize);
		buffer->tail += tempSize;
	}
	/* Ako ima prelamanja podataka kopiraju se podaci iz dva puta */
	else
	{
		int temp = buffer->buffer_end - buffer->tail;
		memcpy(data, buffer->tail, temp);
		memcpy(data + temp, buffer->buffer_start, tempSize - temp);
		buffer->tail = buffer->buffer_start + (tempSize - temp);
	}

	/* Promena stanja buffera */
	buffer->free += tempSize;
	buffer->taken -= tempSize;

	return tempSize;
}

int rRead(Kruzni_Buffer * buffer, char * data, int size)
{
	/* Velicina podataka koja se cita iz buffera */
	int tempSize;
	if (size <= buffer->taken)
		tempSize = size;
	else
		tempSize = buffer->taken;

	/* Ako nema prelamanja podataka kopiraju se podaci od jednom */
	if ((buffer->buffer_end - buffer->tail) >= tempSize)
	{
		memcpy(data, buffer->tail, tempSize);
	}
	/* Ako ima prelamanja podataka kopiraju se podaci iz dva puta */
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
	/* Velicina podataka koja se brise iz buffera */
	int tempSize;
	if (size <= buffer->taken)
		tempSize = size;
	else
		tempSize = buffer->taken;

	/* Ako nema prelamanja podataka brisu se podaci od jednom */
	if ((buffer->buffer_end - buffer->tail) >= tempSize)
	{
		buffer->tail += tempSize;
	}
	/* Ako ima prelamanja podataka brisu se podaci iz dva puta */
	else
	{
		int temp = buffer->buffer_end - buffer->tail;
		buffer->tail = buffer->buffer_start + (tempSize - temp);
	}

	/* Promena stanja buffera */
	buffer->free += tempSize;
	buffer->taken -= tempSize;

	return tempSize;
}
