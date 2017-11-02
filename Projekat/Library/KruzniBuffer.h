#pragma once
#include "header.h"

/**
* Inicijalizacija buffera
* @param buffer - Pokazivac na Kruzni_Buffer struct
* @param size - Inicijalna velicina buffera
*/
void rInitBuffer(Kruzni_Buffer* buffer, int size);

/**
* Oslobadjanje buffera
* @param buffer - Pokazivac na Kruzni_Buffer struct
*/
void rFreeBuffer(Kruzni_Buffer* buffer);

/**
* Pormena velicine buffera
* @param buffer - Pokazivac na Kruzni_Buffer struct
* @param size - Nova velicina buffera
*/
int rResize(Kruzni_Buffer* buffer, int size);

/**
* Kopira podatke iz data u slobodan prostor buffera
* @param buffer - Pokazivac na Kruzni_Buffer struct
* @param data - Pokazivac lokacije odakle se uzimaju podaci
* @param size - Velicina podataka za kopiranje
*/
int rPush(Kruzni_Buffer * buffer, char* data, int size);

/**
* Kopira podatke iz buffera na data i brise ih iz buffera
* @param buffer - Pokazivac na Kruzni_Buffer struct
* @param data - Pokazivac lokacije na koju se smestaju podaci
* @param size - Velicina podataka za kopiranje
*/
int rPop(Kruzni_Buffer * buffer, char* data, int size);

/**
* Kopira podatke iz buffera na data
* @param buffer - Pokazivac na Kruzni_Buffer struct
* @param data - Pokazivac lokacije na koju se smestaju podaci
* @param size - Velicina podataka za kopiranje
*/
int rRead(Kruzni_Buffer * buffer, char* data, int size);

/**
* Brise podatke iz buffera
* @param buffer - Pokazivac na Kruzni_Buffer struct
* @param size - Velicina podataka za kopiranje
*/
int rDelete(Kruzni_Buffer * buffer, int size);