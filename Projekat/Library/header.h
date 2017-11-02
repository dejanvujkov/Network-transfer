#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 10*1024*1024	/* Inicijalna velicina kruznog buffera - 10MB */
#define MAX_UDP_SIZE 64*1024 - 29		/* Velicina DATA dela UDP paketa */
#define TIMEOUT_SEC 2 * 1000			/* Vreme koje socketi cekaju do timeout-a */
#define INITIAL_CWND 10					/* Inicijalni broj bajtova za slanje */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>

#include "enums.h"
#include "KruzniBuffer.h"
#include "ThreadHeaders.h"
#include "structs.h"

/**
* Inicijalizuje rSocket i vraca pokazivac na njega
*/
rSocket* rInitialize();

/**
* Deinicijalizuje rSocket i oslobadja memoriju
* @param s - Pokazivac na rSocket strukturu
*/
int rDeinitialize(rSocket* s);

/**
* Podesava IP adresu i Port servera, podesava timeout i salje REQUEST
* @param s - Pokazivac na rSocket strukturu
* @param serverAddress - Pokazivac na adresu servera
* @param port - Port servera
*/
int rConnect(rSocket* s, char* serverAddress, short port);

/**
* Podesava IP adresu servera na INADDR_ANY, podesava Port, binduje SOCKET i pdesava timeout
* @param s - Pokazivac na rSocket strukturu
* @param port - Port servera
*/
int rAccept(rSocket* s, short port);

/**
* Stavlja podatke sa data na send buffer
* @param s - Pokazivac na rSocket strukturu
* @param data - Pokazivac na podatke
* @param len - velicina podataka
*/
int rSend(rSocket* s, char* data, int len);

/**
* Stavlja podatke sa recv buffera na data
* @param s - Pokazivac na rSocket strukturu
* @param data - Pokazivac na buffer
* @param len - velicina podataka
*/
int rRecv(rSocket* s, char* data, int len);