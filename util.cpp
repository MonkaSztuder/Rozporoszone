#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

/*
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
state_t stan = InRun;
int lamport = 0;

/* zamek wokół zmiennej współdzielonej między wątkami.
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami
 */
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lampMut = PTHREAD_MUTEX_INITIALIZER;

struct tagNames_t
{
    const char *name;
    int tag;
} tagNames[] = {{"pakiet aplikacyjny", APP_PKT}, {"finish", FINISH}, {"potwierdzenie", ACK}, {"prośbę o sekcję krytyczną", REQUEST}, {"zwolnienie sekcji krytycznej", RELEASE}, {"prośba o wpisanie do kolejki kranslai", REQUESTK}};

const char *const tag2string(int tag)
{
    for (int i = 0; i < sizeof(tagNames) / sizeof(struct tagNames_t); i++)
    {
        if (tagNames[i].tag == tag)
            return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
 */
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int blocklengths[NITEMS] = {1, 1, 1, 1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint offsets[NITEMS];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);
    offsets[3] = offsetof(packet_t, task);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag, int inc)
{
    pthread_mutex_lock(&lampMut);

    int freepkt = 0;
    if (pkt == 0)
    {
        pkt = new packet_t;
        freepkt = 1;
    }
    if (inc == 0)
    {
        lamport += 1;
        pkt->ts = lamport;
    }
    else
    {
        pkt->ts = inc;
    }

    pkt->src = rank;
    MPI_Send(pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
    if (freepkt)
        delete pkt;
    pthread_mutex_unlock(&lampMut);
}

void changeState(state_t newState)
{
    pthread_mutex_lock(&stateMut);
    if (stan == InFinish)
    {
        pthread_mutex_unlock(&stateMut);
        return;
    }
    stan = newState;
    pthread_mutex_unlock(&stateMut);
}

bool compare(const std::pair<int, int> &pair1, const std::pair<int, int> &pair2)
{
    if (pair1.second != pair2.second)
    {
        return pair1.second < pair2.second;
    }
    return pair1.first < pair2.first;
}

void sort_kolejka(std::vector<std::pair<int, int>> *v)
{
    std::sort(v->begin(), v->end(), compare);
}

void print_kolejka(std::vector<std::pair<int, int>> *v)
{
    printf("ja %d \n", rank);
    for (int i = 0; i < v->size(); i++)
    {

        printf("[%d] %d %d\n", rank, v->at(i).first, v->at(i).second);
    }
    printf("\n");
}

void usun_z_kolejki(std::vector<std::pair<int, int>> *v, int id)
{
    for (int i = 0; i < v->size(); i++)
    {
        if (v->at(i).first == id)
        {
            v->erase(v->begin() + i);
            break;
        }
    }
}

int checkOlder()
{
    int counter = 0;
    for (int i = 1; i < K + 1; i++)
    {
        if (timestamps[i] > timestamps[rank])
        {
            counter++;
        }
        else if (timestamps[i] == timestamps[rank] && i > rank)
        {
            counter++;
        }
    }
    return counter;
}

int which_in_queue(std::vector<std::pair<int, int>> *v, int id)
{
    for (int i = 0; i < v->size(); i++)
    {
        if (v->at(i).first == id)
        {
            return i;
        }
    }
    return -1;
}
