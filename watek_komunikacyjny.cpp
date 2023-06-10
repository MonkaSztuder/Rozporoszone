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

    // id_skansenu; zlecienie
    std::vector<std::pair<int, int>> kolejka_zlecen;
    // id_krasnala; lamport_krasnala
    std::vector<std::pair<int, int>> kolejka_krasnali;
    // id_krasnala; lamport_krasnala
    std::vector<std::pair<int, int>> kolejka_do_portali;
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
        if (pakiet.ts > lamport)
        {
            lamport = pakiet.ts;
        }
        lamport++;
        if (stan == InRun)
        {
            ackCount = 0;
        }
        switch (status.MPI_TAG)
        {
        case REQUEST:
            if (stan == InRun || (stan == InWant && pakiet.ts <= lamport_earlier))
            {
                if (pakiet.ts == lamport_earlier && status.MPI_SOURCE > rank)
                    break;
                printf("%d Dostałem REQ od %d, odesłałem\n", rank, status.MPI_SOURCE);
                kolejka_krasnali.push_back(std::make_pair(status.MPI_SOURCE, pakiet.ts));
                printf("Kolejka krasnali: \n");
                print_kolejka(&kolejka_krasnali);
                sort_kolejka(&kolejka_krasnali);
                printf("Kolejka krasnali po sort: \n");
                print_kolejka(&kolejka_krasnali);
                sendPacket(0, status.MPI_SOURCE, ACK);
            }
            break;
        case ACK:
            //dostanie ACK od krasnala dla zlecenia
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
            pthread_mutex_lock(&stateMut);
            ackCount++; // czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. /
            pthread_mutex_unlock(&stateMut);
            break;

        case RELEASE:
            //zwolnienie kolejki do portali
            debug("Dostałem RELEASE od %d, mam już %d", status.MPI_SOURCE, ackCount);
            ackCount++; // czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
            kolejka_krasnali.erase(std::remove(kolejka_krasnali.begin(), kolejka_krasnali.end(), std::make_pair(status.MPI_SOURCE, pakiet.ts)), kolejka_krasnali.end());

            break;

        case ZLECENIE:
            debug("Dostałem ZLECENIE od %d", pakiet.data);
            //dostanie nowego zlecenia
            kolejka_zlecen.push_back(std::make_pair(pakiet.data,zlecenia_count));
            zlecenia_count++;
            break;

        case ZLECENIEZABRANE:
            debug("Dostałem ZLECENIEZABRANE od %d", status.MPI_SOURCE);
            //dostanie informacji o zabraniu zlecenia
            break;

        case RELEASEK:
            debug("Dostałem RELEASEK od %d", status.MPI_SOURCE);
            //zwolnienie kolejki do zlecen
            break;
        case REQUESTK:
            debug("Dostałem REQUESTK od %d", status.MPI_SOURCE);
            //prośba o wejscie do kolejki do zlecen
            break;

        default:
            break;
        }
    }
    return 0;
}