#pragma once

enum rConnectionState {
	CONNECTED,
	DISCONNECTED
};

enum rMessageState {
	REQUEST,
	ACCEPTED,
	REJECTED,
	SENT,
	RECIEVED,
	DROPPED
};