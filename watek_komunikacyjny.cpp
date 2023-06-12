#include "main.h"
#include "watek_komunikacyjny.h"

void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    // pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
    // pthread_mutex_t lampMut = PTHREAD_MUTEX_INITIALIZER;
    packet_t pakiet;
    int lamport_earlier;
    int zlecenia_count = 0;

    // / Obrazuje pętlę odbierającą pakiety o różnych typach /
    while (stan != InFinish )
    {
        debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (stan == InRun)
        {
            pthread_mutex_lock(&lampMut);
            lamport_earlier = lamport;
            pthread_mutex_unlock(&lampMut);
        }
        // lamport_earlier = lamport;
        if (status.MPI_SOURCE != 0)
        {
            pthread_mutex_lock(&lampMut);

            if (pakiet.ts > lamport)
            {
                lamport = pakiet.ts;
            }
            lamport++;
            pthread_mutex_unlock(&lampMut);
        }

        timestamps[status.MPI_SOURCE] = pakiet.ts;
        // printf("%d - %d - ile \n",rank, checkOlder());
        // printf("%d - %d , %d , %d \n",rank,timestamps[1],timestamps[2],timestamps[3]);

        switch (status.MPI_TAG)
        {
        case REQUEST:
            debug("Dostałem REQ od %d", status.MPI_SOURCE);
            if (pakiet.task != -1)
                id_zlecenia = pakiet.task;

            // kolejka_do_portali.push_back(std::make_pair(status.MPI_SOURCE, pakiet.ts));
            dodaj_do_kolejki(&kolejka_do_portali, status.MPI_SOURCE, pakiet.ts);
            sort_kolejka(&kolejka_do_portali);
            // printf("Ja %d usuwam %d z kolejki zlecen\n", rank, pakiet.data);
            // while (!usun_z_kolejki(&kolejka_zlecen, pakiet.data))
            //{
            usun_z_kolejki(&kolejka_zlecen, pakiet.data);
            //}
            // printf("Ja %d usuwam %d z kolejki krasnali\n", rank, status.MPI_SOURCE);
            // while (!usun_z_kolejki(&kolejka_krasnali, status.MPI_SOURCE))
            //{
            usun_z_kolejki(&kolejka_krasnali, status.MPI_SOURCE);
            //}
            if (stan != InPortalQueue)
                sendPacket(0, status.MPI_SOURCE, ACK, 0);
            break;
        case ACK:
            // dostanie ACK od krasnala dla zlecenia
            debug("Dostałem ACK od %d", status.MPI_SOURCE);
            break;

        case RELEASE:
            // zwolnienie kolejki do portali
            debug("Dostałem RELEASE od %d", status.MPI_SOURCE);
            // printf("Ja %d usuwam %d z kolejki do portali\n", rank, status.MPI_SOURCE);
            // while (!usun_z_kolejki(&kolejka_do_portali, status.MPI_SOURCE))
            //{
            usun_z_kolejki(&kolejka_do_portali, status.MPI_SOURCE);
            //}
            break;

        case ZLECENIE:
            debug("Dostałem ZLECENIE od %d", pakiet.data);
            // printf("Ja %d dodaje %d do kolejki zlecen\n", rank, pakiet.data);
            // kolejka_zlecen.push_back(std::make_pair(pakiet.data, pakiet.task));
            dodaj_do_kolejki(&kolejka_zlecen, pakiet.data, pakiet.task);
            // dostanie nowego zlecenia
            break;

        case REQUESTK:
            debug("Dostałem REQUESTK od %d", status.MPI_SOURCE);

            // kolejka_krasnali.push_back(std::make_pair(status.MPI_SOURCE, pakiet.ts));
            dodaj_do_kolejki(&kolejka_krasnali, status.MPI_SOURCE, pakiet.ts);
            sort_kolejka(&kolejka_krasnali);
            if (stan != InWant)
                sendPacket(0, status.MPI_SOURCE, ACK, 0);
            // prośba o wejscie do kolejki do zlecen
            break;

        default:
            break;
        }
    }
    return 0;
}