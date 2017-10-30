#pragma once

enum rConnectionState {
	REQUEST = 10,
	ACCEPTED,
	REJECTED,
	CONNECTED,
	DISCONNECTED
};

enum rMessageState {
	SENT,
	RECIEVED,
	DROPPED
};