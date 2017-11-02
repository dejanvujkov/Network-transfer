#pragma once
/**
 * Enumeracije koje se koriste u projektu
 */

/* Tip poruke koja se salje */
enum rMessageType
{
	REQUEST = 10,	/* Oznaka koju klijent salje za uspostavljanje veze. */
	ACCEPT = 11,	/* Oznaka koju server salje ako je prihvatio zahtev. */
	REJECT = 12,	/* Oznaka koju server salje ako je odbio zahtev. */
	DATA = 30, 		/* Oznaka da se salju podaci */
	ACK = 31		/* Oznaka da je poruka primljena */
};

/* Stanje konekcije Client-Server */
enum rConnectionState {
	CONNECTED,		/* Oznaka da je veza uspostavljena */
	DISCONNECTED	/* Oznaka da veza nije uspostavljena */
};