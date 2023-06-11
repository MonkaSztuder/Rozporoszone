#include "main.h"
#include "watek_zlecen.h"

void skansen()
{
    int id_skan;
    int id_zlec = 1;
    while (TRUE)
    {
        id_skan = random()%S;
        usleep(5000);
        packet_t *pkt = new packet_t;
        pkt->data = id_zlec;
        pkt->task = id_skan;
        for (int i = 1; i <= size-1; i++)
            sendPacket(pkt, i, ZLECENIE,0);
        delete pkt;
        id_zlec++;
    }
}
