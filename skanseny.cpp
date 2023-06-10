#include "main.h"
#include "skanseny.h"

void generuj_zlecenia(int id_skansenu)
{
    packet_t *pkt = new packet_t;
    pkt->data = id_skansenu;
    for (int i = 0; i <= K - 1; i++)
    sendPacket(pkt, i, ZLECENIE);
    delete pkt;
}