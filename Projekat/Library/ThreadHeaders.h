#pragma once
#include "header.h"

/**
 * Thread za slanje podataka sa kruznog buffera
 * @param param - Ocekuje se pokazivac na rSocket strukturu
 */
DWORD WINAPI SendThread(LPVOID param);

/**
* Thread za primanje podataka i smestanje na kruzni buffer
* @param param - Ocekuje se pokazivac na rSocket strukturu
*/
DWORD WINAPI RecvThread(LPVOID param);

/**
* Metoda za prebrojavanje ACK-ova
* @param s - Pokazivac na rSocket strukturu
* @param ackBuffer - Pokazivac na buffer koji je primio ACK-ove
* @param ackCount - Broj primljenih ACK-ova u poslednjoj iteraciji
*/
void CountACKs(rSocket* s, char* ackBuffer, int ackCount);

/**
* Metoda za prebrojavanje ACK-ova
* @param s - Pokazivac na rSocket strukturu
*/
int Algoritam(rSocket* s);