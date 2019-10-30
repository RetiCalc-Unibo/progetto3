#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>


//non sarebbe meglio controllare qual è la grandezza massima di un nome file?
#define MAX_LENGTH 255

// Struttura di una richiesta
typedef struct {
	char file[MAX_LENGTH];
} Request;

int main(int argc, char *argv[]) {
	struct hostent *host;
	struct sockaddr_in clientaddr, servaddr;
	int portNumber, datagramSocket, number, length, result, nameLength,nreq;
	char fileName[MAX_LENGTH];
	Request request;
	clock_t start,diff_time;
	double sec;
	
	strcpy(fileName, argv[3]);

	// Controllo argomenti in input
	if (argc != 4) {
		printf("Errore. Utilizzo del programma: %s serverAddress serverPort fileName\n", argv[0]);
		exit(1);
	}

	// Inizializzazione indirizzo Client e Server
	memset((char *)&clientaddr, 0, sizeof(struct sockaddr_in));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = INADDR_ANY;

	// Passando 0 ci leghiamo ad una qualsiasi porta libera
	clientaddr.sin_port = 0;

	memset((char *)&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	host = gethostbyname(argv[1]);

	// Controllo che la porta passata come argomento sia formata da soli numeri
	number = 0;
	while (argv[2][number] != '\0') {
		if ((argv[2][number] < '0') || (argv[2][number] > '9')) {
			printf("Secondo argomento non intero.\n");
			printf("Errore. Utilizzo del programma: %s serverAddress serverPort\n", argv[0]);
			exit(2);
		}
		number++;
	}

	portNumber = atoi(argv[2]);

	// Verifica porta e host
	// NB: NON è un controllo da fare solo quando si crea la SOCKET nel SERVER?
	if (portNumber < 1024 || portNumber > 65535) {
		printf("La porta passata in input deve essere compresa tra 1024 e 65535.\n");
		exit(3);
	}

	if (host == NULL) {
		printf("L'host non è stato trovato.\n");
		exit(4);
	} else {
		servaddr.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
		servaddr.sin_port = htons(portNumber);
	}

	// Creazione socket
	datagramSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (datagramSocket < 0) {
		perror("Apertura socket.");
		exit(5);
	}
	printf("Creata la socket %d.\n", datagramSocket);

	// Bind socket a una porta scelta dal sistema
	if (bind(datagramSocket, (struct sockaddr*)&clientaddr, sizeof(clientaddr)) <0) {
		perror("Bind socket.");
		exit(6);
	}
	printf("Bind socket riuscito alla porta %i\n", clientaddr.sin_port);

	// Inizio del programma effettivo

	start = clock();

	nameLength = strlen(fileName);
	if(nameLength > 4
		&& fileName[nameLength-4] == '.'
		&& fileName[nameLength-3] == 't'
		&& fileName[nameLength-2] == 'x'
		&& fileName[nameLength-1] == 't') {

		// copio l'array
		memcpy(&(request.file), &fileName, sizeof(request.file));	
		length = sizeof(servaddr);

	

		for(nreq = 0; nreq < 4; nreq++){

			if (sendto(datagramSocket, &request, sizeof(Request), 0, (struct sockaddr*)&servaddr, length) < 0) {
				perror("Errore nella sendto.");
			}
			if (recvfrom(datagramSocket, &result, sizeof(result), 0, (struct sockaddr*)&servaddr, &length) < 0) {
				perror("Errore nella recvfrom.");
			}
		
			if((int)ntohl(result) > 0) {
				//printf("La parola piu' lunga nel file richiesto ha %i caratteri.\n", (int)ntohl(result));
			} else if((int)ntohl(result) == 0){
				printf("Nel file richiesto non ci sono parole\n");
			} else {
				printf("Il file %s non esiste sul server\n", fileName);
			}

		printf("N° richiesta: %d\n", nreq);

		}

	} else {
		printf("Il file inserito non è un file di testo (*.txt)\n");
	}

	diff_time = clock() - start;

	sec=((double)diff_time)/ CLOCKS_PER_SEC;

	printf("Tempo impiegato per 300 richieste: %f secondi\n", sec);

	close(datagramSocket);
	printf("Il Client sta terminando...\n");  
	exit(0);
}
