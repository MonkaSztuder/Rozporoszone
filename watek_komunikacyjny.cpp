#include "main.h"
#include "watek_komunikacyjny.h"

void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    int lamport_earlier;
   // / Obrazuje pętlę odbierającą pakiety o różnych typach /
    while ( stan!=InFinish ) {
    debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        lamport_earlier = lamport;
        if(pakiet.ts >lamport)
        {
            lamport=pakiet.ts;
        }
        lamport++;

        switch ( status.MPI_TAG ) {
        case REQUEST: 
                debug("Ktoś coś prosi. A niech ma!")
        if(stan == InRun || (stan == InWant && pakiet.ts <= lamport_earlier))
        {
            if(pakiet.ts == lamport_earlier && status.MPI_SOURCE > rank)
                break;
            printf("%d Dostałem REQ od %d, odesłałem\n", rank,status.MPI_SOURCE);
            sendPacket( 0, status.MPI_SOURCE, ACK );
        }
        break;
        case ACK: 
                debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
            ackCount++; // czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. /
        break;
        case RELEASE:
                debug("Dostałem RELEASE od %d, mam już %d", status.MPI_SOURCE, ackCount);
            ackCount++; // czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */

        break;
        default:
        break;
        }
    }
}