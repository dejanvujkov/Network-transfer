#pragma once

enum rMessageType
{
	REQUEST = 10,
	ACCEPT = 11,
	REJECT = 12,
	DATA = 30,
	ACK = 31
};

enum rConnectionState {
	CONNECTED,
	DISCONNECTED
};

/////////////

//
//enum rMessageState {
//	REQUEST,
//	ACCEPTED,
//	REJECTED,
//	SENT,
//	RECIEVED,
//	DROPPED
//};