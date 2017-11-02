#pragma once
#include "header.h"

DWORD WINAPI SendThread(LPVOID param);
DWORD WINAPI RecvThread(LPVOID param);

void CountACKs(rSocket* s, char* ackBuffer, int ackCount);

int Algoritam(rSocket* h);