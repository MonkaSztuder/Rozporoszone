#pragma once
#include "main.h"

/* typ pakietu */
typedef struct
{
    int ts; /* timestamp (zegar lamporta */
    int src;

    int data; /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK 1
#define REQUEST 2
#define RELEASE 3
#define APP_PKT 4
#define FINISH 5

#define ZLECENIE 6
#define ZLECENIEZABRANE 66
#define REQUESTK 22
#define RELEASEK 33
#define ACKP 11

#define S 2
#define K 4
#define P 4

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum
{
    InRun,
    InMonitor,
    InWant,
    InSection,
    InFinish
} state_t;
extern state_t stan;
extern pthread_mutex_t stateMut;
/* zmiana stanu, obwarowana muteksem */
void changeState(state_t);

bool compare(const std::pair<int, int> &pair1, const std::pair<int, int> &pair2);

void sort_kolejka(std::vector<std::pair<int, int>> *v);

void print_kolejka(std::vector<std::pair<int, int>> *v);