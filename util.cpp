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
pthread_mutex_t modyfikacjaKolejkiMut = PTHREAD_MUTEX_INITIALIZER;

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
    if (v == nullptr)
    {
        // Obsługa przypadku, gdy wskaźnik na wektor jest nieprawidłowy
        return;
    }

    if (v->empty())
    {
        // Obsługa przypadku, gdy wektor jest pusty
        return;
    }

    // Sprawdzenie poprawności indeksów przed sortowaniem
    for (const auto &pair : *v)
    {
        if (pair.first < 0 || pair.first >= v->size())
        {
            // Obsługa przypadku, gdy indeks jest nieprawidłowy
            return;
        }
    }

    pthread_mutex_lock(&modyfikacjaKolejkiMut);
    std::sort(v->begin(), v->end() - 1, compare);
    pthread_mutex_unlock(&modyfikacjaKolejkiMut);
}

void print_kolejka(std::vector<std::pair<int, int>> *v)
{
    printf("ja %d \n", rank);
    for (int i = 0; i < v->size(); i++)
    {
        if (i < v->size())
            printf("[%d] %d %d\n", rank, v->at(i).first, v->at(i).second);
    }
    printf("\n");
}

bool usun_z_kolejki(std::vector<std::pair<int, int>> *v, int id)
{
    int znal = 0;
    pthread_mutex_lock(&modyfikacjaKolejkiMut);
    // printf("vektor %ld\n", v->size());

    for (int i = 0; i < v->size(); i++)
    {
        if (v->at(i).first == id)
        {
            if (i < v->size())
            {
                // printf("vektor w petli %ld begin %ld\n", v->size(), v->begin());
                v->erase(v->begin() + i);
                // printf("vektor po usunieciu %ld ZLICZ %d\n", v->size(),znal);
                pthread_mutex_unlock(&modyfikacjaKolejkiMut);
                return true;
            }
        }
    }
    //if (znal == 0)
        //printf("nie ma takiego id %d\n", id);
    pthread_mutex_unlock(&modyfikacjaKolejkiMut);
    return false;
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
    //printf("size %ld\n", v->size());
    pthread_mutex_lock(&modyfikacjaKolejkiMut);
    for (int i = 0; i < v->size(); i++) // iteracja po vectorze
    {
        //printf("znalazlem %d size %ld i %d\n", id, v->size(), i);

        if (v->at(i).first == id)
        {
            pthread_mutex_unlock(&modyfikacjaKolejkiMut);
            return i; // zwraca pozycje w kolejce
        }
    }
    pthread_mutex_unlock(&modyfikacjaKolejkiMut);
    return -1; // nie ma w kolejce
}

void dodaj_do_kolejki(std::vector<std::pair<int, int>> *v, int id, int ts)
{
    // printf("dodaje do kolejki %d %d SIZE %ld\n", id, ts, v->size());
    pthread_mutex_lock(&modyfikacjaKolejkiMut);
    v->push_back(std::make_pair(id, ts));
    pthread_mutex_unlock(&modyfikacjaKolejkiMut);
    // printf("dodalem do kolejki %d %d SIZE %ld\n", id, ts, v->size());
}