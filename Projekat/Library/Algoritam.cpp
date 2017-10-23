#include "Algoritam.h"

void Algoritam(int * cwnd, int * ssthresh, int * recv, bool * slowstart)
{
	/*if (*slowstart)
	{
		// Ako je primljeno manje nego sto je poslato, preci u Tahoe mod i ponovo poslati iste pakete
		if (h->cwnd < h->recv)
		{
			h->ssthresh = h->cwnd / 2;
			h->cwnd = h->ssthresh;
			h->slowstart = false;
			continue;
		}
		// Ako nema problema, povecati cwnd 2x
		else
		{
			h->cwnd = h->cwnd * 2;
		}
	}
	// Ako je u Tahoe modu, povecavati cwnd za 1 u svakoj iteraciji
	else
	{
		// Ako je primljeno manje nego sto je poslato, postaviti cwnd na ssthresh i poslati ponovo
		if (h->cwnd < h->recv)
		{
			h->cwnd = h->ssthresh;
			continue;
		}
		// Ako nema problema, povecati cwnd za 1
		else
		{
			h->cwnd = h->cwnd + 1;
		}
	}*/
}
