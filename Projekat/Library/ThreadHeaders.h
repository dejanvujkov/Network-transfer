#pragma once
#include "header.h"

DWORD WINAPI FromDataToBuffer(LPVOID param);
DWORD WINAPI SendDataFromBuffer(LPVOID param);

DWORD WINAPI RecieveMessage(LPVOID param);

int Algoritam(rHelper* h);
int SendOneMessage(rMessageHeader* header, int* idPoslednjePoslato, int* brojPaketa, int i, int velicinaPoruke, rHelper* h, int* trenutnoProcitano, int* procitano, char* tempbuffer);
int RecvOneMessage(rHelper* h, char* tempbuffer, int i);
int SlideOneMessage(rMessageHeader* header, char* tempbuffer, int i, rHelper* h);