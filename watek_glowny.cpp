#include "main.h"
#include "watek_glowny.h"
#include "skanseny.h"

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;
	int zostanSkansenemTemp;
	int older_counter = 0;
	int gathered = -1;
	packet_t *pkt;
    while (stan != InFinish) {
	switch (stan) {
	    case InRun:
		perc = random()%100;
		zostanSkansenemTemp = random()%100;
	//	if(zostanSkansenemTemp < 20){
		//	int skansen_id = random()%S;
			//generuj_zlecenia(skansen_id);
			//break;
		//}
		if ( perc < 25 ) {
		    debug("Perc: %d", perc);
		    println("Ubiegam się o sekcję krytyczną");
		    debug("Zmieniam stan na wysyłanie");
		    pkt = new packet_t;
		    pkt->data = perc;
		    ackCount = 0;
		    pthread_mutex_lock(&stateMut);
			lamport += 1;
			pthread_mutex_unlock(&stateMut);
			timestamps[rank] = lamport;
		    for (int i=0;i<=size-1;i++)
			if (i!=rank)
			    sendPacket( pkt, i, REQUESTK, 0);
		    changeState( InWant );
			kolejka_krasnali.push_back(std::make_pair(rank, lamport));
			sort_kolejka(&kolejka_krasnali);
			 // w VI naciśnij ctrl-] na nazwie funkcji, ctrl+^ żeby wrócić
					   // :w żeby zapisać, jeżeli narzeka że w pliku są zmiany
					   // ewentualnie wciśnij ctrl+w ] (trzymasz ctrl i potem najpierw w, potem ]
					   // między okienkami skaczesz ctrl+w i strzałki, albo ctrl+ww
					   // okienko zamyka się :q
					   // ZOB. regułę tags: w Makefile (naciśnij gf gdy kursor jest na nazwie pliku)
		    delete pkt;
		} // a skoro już jesteśmy przy komendach vi, najedź kursorem na } i wciśnij %  (niestety głupieje przy komentarzach :( )
		debug("Skończyłem myśleć");
		break;
		
	    case InWant:
		println("Czekam na wziecie zlecenia i wejscie do kolejki do portali");
		// tutaj zapewne jakiś muteks albo zmienna warunkowa
		// bo aktywne czekanie jest BUE
        while(TRUE)
        {


			/*if(rank == 5)
			{
				print_kolejka(&kolejka_krasnali);
				printf("ja to %d moj checkOlder to %d kolejka kranslia pierwszy to %d musze miec K - 2 = %d , kolejka zlecen size to %d\n",rank,checkOlder(),kolejka_krasnali[0].first,K - 2,kolejka_zlecen.size());
				for(int i = 1; i < 6; i++)
				printf("%d,",timestamps[i]);
				printf("\n");
			}*/

			if ((checkOlder() >= K - 2 && kolejka_krasnali[0].first == rank))
			{
				if(kolejka_zlecen.size() > 0)
				{
					id_zlecenia = kolejka_zlecen[0].first;
					changeState(InPortalQueue);
					break;
				}
			}
			else if(id_zlecenia != -1)
			{

				gathered = 1;
				changeState(InPortalQueue);
				break;
			}
        }
		break;
		case InPortalQueue:
			println("Wchodzę do kolejki portali");

			pkt = new packet_t;
			pkt->data = id_zlecenia;
			usun_z_kolejki(&kolejka_krasnali, rank);

			pthread_mutex_lock(&stateMut);
			lamport += 1;
			pthread_mutex_unlock(&stateMut);
			timestamps[rank] = lamport;
			kolejka_do_portali.push_back(std::make_pair(rank, lamport));
		    for (int i=1;i<=size-1;i++)
			{	
				if (i!=rank)
				{
					int place = which_in_queue(&kolejka_krasnali,i);
					if(place > -1 && place < kolejka_zlecen.size() && gathered != 1)
					{
						//printf("Ja %d daje zlecenie nr %d temu %d jego miejsce to %d gathered to %d moje zlecenie to %d",rank,kolejka_zlecen[place].first,i,place,gathered,id_zlecenia	);
						//print_kolejka(&kolejka_zlecen);
						
						pkt->task = kolejka_zlecen[place].first;
					}
					else
					{
						pkt->task = -1;
					}
					sendPacket( pkt, i, REQUEST,0);
				}
			}
			delete pkt;
			
			//printf("jestem %d w kolejce do portali\n", which_in_queue(&kolejka_do_portali,rank));
			while(TRUE)
			{
					if ( checkOlder() >= K - 2 - P + 1 && which_in_queue(&kolejka_do_portali,rank) < P)
					{
						changeState(InSection);
						break;
					}
			}
		break;
	    case InSection:
		// tutaj zapewne jakiś muteks albo zmienna warunkowa
		println("Jestem w sekcji krytycznej ze zleceniem %d", id_zlecenia);
		    sleep(10);
		//if ( perc < 25 ) {
		    debug("Perc: %d", perc);
		    println("Wychodzę z sekcji krytyczneh");
		    debug("Zmieniam stan na wysyłanie");
		    pkt = new packet_t;
		    pkt->data = id_zlecenia;
			pthread_mutex_lock(&stateMut);
			lamport += 1;
			pthread_mutex_unlock(&stateMut);
		    for (int i=0;i<=size-1;i++)
			if (i!=rank)
			    sendPacket( pkt, i, RELEASE,0);
			    //sendPacket( pkt, (rank+1)%size, RELEASE);
		    changeState( InRun );
		    delete pkt;
		//}
		gathered = -1;
		id_zlecenia = -1;
		usun_z_kolejki(&kolejka_do_portali, rank);
		
		break;
            }
        sleep(SEC_IN_STATE);
    }
}
