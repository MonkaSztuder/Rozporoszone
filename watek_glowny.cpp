#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
	srand(rank);
	int tag;
	int perc;

	int older_counter = 0;
	int gathered = -1;
	packet_t *pkt;
	while (stan != InFinish)
	{
		switch (stan)
		{
		case InRun:
			perc = random() % 100;

			if (perc < 25)
			{
				debug("Perc: %d", perc);
				println("Ubiegam się o wzięcie zlecenia");
				debug("Zmieniam stan na wysyłanie");
				pkt = new packet_t;
				pkt->data = perc;

				pthread_mutex_lock(&lampMut);
				lamport += 1;
				timestamps[rank] = lamport;
				pthread_mutex_unlock(&lampMut);

				for (int i = 1; i <= size - 1; i++)
					if (i != rank)
						sendPacket(pkt, i, REQUESTK, timestamps[rank]);
				changeState(InWant);
				// printf("DODAJe SIEBIE  %d\n", rank);
				// print_kolejka(&kolejka_krasnali);

				// kolejka_krasnali.push_back(std::make_pair(rank, timestamps[rank]));
				dodaj_do_kolejki(&kolejka_krasnali, rank, timestamps[rank]);
				// print_kolejka(&kolejka_krasnali);
				sort_kolejka(&kolejka_krasnali);
				// print_kolejka(&kolejka_krasnali);

				delete pkt;
			} // a skoro już jesteśmy przy komendach vi, najedź kursorem na } i wciśnij %  (niestety głupieje przy komentarzach :( )
			debug("Skończyłem myśleć");
			break;

		case InWant:
			println("Czekam na wziecie zlecenia i wejscie do kolejki do portali");
			while (TRUE)
			{
				/*
							//	if(rank == 3 || rank == 5)
							//	{
									print_kolejka(&kolejka_krasnali);
									printf("ja to %d moj checkOlder to %d kolejka kranslia pierwszy to %d musze miec K - 2 = %d , kolejka zlecen size to %ld\n",rank,checkOlder(),kolejka_krasnali[0].first,K - 2,kolejka_zlecen.size());
									for(int i = 1; i < K + 1; i++)
									printf("%d,",timestamps[i]);
									printf("\n");
							//	} */

				// print_kolejka(&kolejka_krasnali);

				if ((checkOlder() >= K - 1 && kolejka_krasnali[0].first == rank))
				{
					if (kolejka_zlecen.size() > 0)
					{
						id_zlecenia = kolejka_zlecen[0].first;

						changeState(InPortalQueue);
						break;
					}
				}
				else if (id_zlecenia != -1)
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
			// printf("Ja %d Usuwam SIEBIE z kolejkikrasnali %d\n", rank, rank);
			// while (!usun_z_kolejki(&kolejka_krasnali, rank))
			//{
			usun_z_kolejki(&kolejka_krasnali, rank);
			//};
			id_skansenu = kolejka_zlecen[which_in_queue(&kolejka_zlecen, id_zlecenia)].second;
			// printf("Ja %d Usuwam SWOJE ZLECENIE z kolejkizlecen %d\n", rank, id_zlecenia);
			// while (!usun_z_kolejki(&kolejka_zlecen, id_zlecenia))
			//{
			usun_z_kolejki(&kolejka_zlecen, id_zlecenia);
			//};
			pthread_mutex_lock(&lampMut);
			lamport += 1;
			timestamps[rank] = lamport;
			pthread_mutex_unlock(&lampMut);

			// kolejka_do_portali.push_back(std::make_pair(rank, timestamps[rank]));
			dodaj_do_kolejki(&kolejka_do_portali, rank, timestamps[rank]);
			sort_kolejka(&kolejka_do_portali);
			for (int i = 1; i <= size - 1; i++)
			{
				if (i != rank)
				{
					int place = which_in_queue(&kolejka_krasnali, i);
					if (place > -1 && place < kolejka_zlecen.size() && gathered != 1)
					{
						// printf("Ja %d daje zlecenie nr %d temu %d jego miejsce to %d gathered to %d moje zlecenie to %d",rank,kolejka_zlecen[place].first,i,place,gathered,id_zlecenia	);

						pkt->task = kolejka_zlecen[place].first;
						sendPacket(pkt, i, REQUEST, timestamps[rank]);
					}
					else
					{
						pkt->task = -1;
						sendPacket(pkt, i, REQUEST, timestamps[rank]);
					}
				}
			}
			delete pkt;
			//printf("RANK: %d", rank);
			// printf("jestem %d w kolejce do portali\n", which_in_queue(&kolejka_do_portali,rank));
			while (TRUE)
			{
				if (checkOlder() >= (K - 2 - P + 1) && which_in_queue(&kolejka_do_portali, rank) < P)
				{
					changeState(InSection);
					break;
				}
			}
			break;
		case InSection:
			// tutaj zapewne jakiś muteks albo zmienna warunkowa
			println("Jestem w portalu ze zleceniem %d od skansenu %d", id_zlecenia, id_skansenu);

			sleep(5);
			// if ( perc < 25 ) {
			debug("Perc: %d", perc);
			println("Wychodzę z PORTALU");
			debug("Zmieniam stan na wysyłanie");
			pkt = new packet_t;
			pkt->data = id_zlecenia;

			pthread_mutex_lock(&lampMut);
			lamport += 1;
			timestamps[rank] = lamport;
			pthread_mutex_unlock(&lampMut);

			for (int i = 1; i <= size - 1; i++)
				if (i != rank)
					sendPacket(pkt, i, RELEASE, 0);
			// sendPacket( pkt, (rank+1)%size, RELEASE);
			changeState(InRun);
			delete pkt;
			// sleep(2);
			// printf("JA %d wychodze z PORTALU usuwam SIEBIE z kolejki %d\n", rank, rank);

			// while (!usun_z_kolejki(&kolejka_do_portali, rank))
			//{
				//printf("JA %d wychodze z PORTALU usuwam SIEBIE z kolejki\n", rank);
			usun_z_kolejki(&kolejka_do_portali, rank);
			//};
			// sleep(2);
			// }
			gathered = -1;
			id_zlecenia = -1;

			break;
		}
		sleep(SEC_IN_STATE);
	}
}
