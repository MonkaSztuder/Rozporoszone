#pragma once
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <unordered_map>


#include "util.h"
/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1
#define STATE_CHANGE_PROB 10
#define ROOT 0

/* tutaj TYLKO zapowiedzi - definicje w main.c */
extern int rank;
extern int id_skansenu;
extern int size;
extern int lamport;
extern int id_zlecenia;
extern pthread_t threadKom;
// id_skansenu; zlecienie
extern std::vector<std::pair<int, int>> kolejka_zlecen;
// id_krasnala; lamport_krasnala
extern std::vector<std::pair<int, int>> kolejka_krasnali;
// id_krasnala; lamport_krasnala
extern std::vector<std::pair<int, int>> kolejka_do_portali;

extern int timestamps[];
/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta

   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
                       "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape.
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów

*/
#ifdef DEBUG
#define debug(FORMAT, ...) printf("%c[%d;%dm [%d, %d]: " FORMAT " %c[%d;%dm\n", 27, (1 + (rank / 7)) % 2, 31 + (6 + rank) % 7, rank, lamport, ##__VA_ARGS__, 27, 0, 37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT, ...) printf("%c[%d;%dm [%d, %d]: " FORMAT "%c[%d;%dm\n", 27, (1 + (rank / 7)) % 2, 31 + (6 + rank) % 7, rank, lamport, ##__VA_ARGS__, 27, 0, 37);
