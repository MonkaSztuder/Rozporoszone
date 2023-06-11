#include "main.h"
#include "watek_komunikacyjny.h"

void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
    packet_t pakiet;
    int lamport_earlier;
    int zlecenia_count = 0;


    // / Obrazuje pętlę odbierającą pakiety o różnych typach /
    while (stan != InFinish)
    {
        debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (stan == InRun)
        {
            lamport_earlier = lamport;
        }
        // lamport_earlier = lamport;
        if(status.MPI_SOURCE != 0)
        {
            if (pakiet.ts > lamport)
            {
                lamport = pakiet.ts;
            }
            lamport++;
        }


        timestamps[status.MPI_SOURCE] = pakiet.ts;
        //printf("%d - %d - ile \n",rank, checkOlder());
        //printf("%d - %d , %d , %d \n",rank,timestamps[1],timestamps[2],timestamps[3]);
        
        switch (status.MPI_TAG)
        {
        case REQUEST:
            debug("Dostałem REQ od %d, mam już %d", status.MPI_SOURCE, ackCount);
            if(pakiet.task != -1)
                id_zlecenia = pakiet.task;
            kolejka_do_portali.push_back(std::make_pair(status.MPI_SOURCE, pakiet.ts));
            sort_kolejka(&kolejka_do_portali);
            usun_z_kolejki(&kolejka_zlecen, pakiet.data);
            usun_z_kolejki(&kolejka_krasnali, status.MPI_SOURCE);
            sendPacket(0, status.MPI_SOURCE, ACK, 1);
            break;
        case ACK:
            //dostanie ACK od krasnala dla zlecenia
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount + 1);
            pthread_mutex_lock(&stateMut);
            ackCount++; // czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. /
            pthread_mutex_unlock(&stateMut);
            break;

        case RELEASE:
            //zwolnienie kolejki do portali
            debug("Dostałem RELEASE od %d, mam już %d", status.MPI_SOURCE, ackCount);
            ackCount++; // czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
            usun_z_kolejki(&kolejka_do_portali, status.MPI_SOURCE);
            break;

        case ZLECENIE:
            debug("Dostałem ZLECENIE od %d", pakiet.data);
            kolejka_zlecen.push_back(std::make_pair(pakiet.data, pakiet.ts));
            //dostanie nowego zlecenia
            //kolejka_zlecen.push_back(std::make_pair(pakiet.data,zlecenia_count));
            //zlecenia_count++;
            break;

        case ZLECENIEZABRANE:
            debug("Dostałem ZLECENIEZABRANE od %d", status.MPI_SOURCE);
            //dostanie informacji o zabraniu zlecenia
            break;

        case RELEASEK:
            debug("Dostałem RELEASEK od %d", status.MPI_SOURCE);
            ackCount++;


            //zwolnienie kolejki do zlecen
            break;
        case REQUESTK:
            debug("Dostałem REQUESTK od %d", status.MPI_SOURCE);

                kolejka_krasnali.push_back(std::make_pair(status.MPI_SOURCE, pakiet.ts));
                sort_kolejka(&kolejka_krasnali);
                sendPacket(0, status.MPI_SOURCE, ACK, 1);
            //prośba o wejscie do kolejki do zlecen
            break;

        default:
            break;
        }
    }
    return 0;
}