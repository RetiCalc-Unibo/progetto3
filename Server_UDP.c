#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define MAX_LENGTH 256

// Struttura di una richiesta
typedef struct {
	char file[MAX_LENGTH];
} Request;

int main(int argc, char *argv[]) {
	const int on = 1;
	int datagramSocket, portNumber, length, number, longestWord, currentWordCounter;
	char c;
	FILE *fp;
	struct sockaddr_in cliaddr, servaddr;
	struct hostent *clienthost;
	Request request;
	
	// Controllo argomenti
	if (argc != 2) {
		printf("Errore. Utilizzo del programma: %s serverPort\n", argv[0]);
		exit(1);
	}

	number = 0;
	while (argv[1][number] != '\0') {
		if ((argv[1][number] < '0') || (argv[1][number] > '9')) {
			printf("Secondo argomento non intero.\n");
			printf("Errore. Utilizzo del programma: %s serverPort\n", argv[0]);
			exit(2);
		}
		number++;
	}

	portNumber = atoi(argv[1]);

	// Verifica porta
	if (portNumber < 1024 || portNumber > 65535) {
		printf("La porta passata in input deve essere compresa tra 1024 e 65535.\n");
		exit(3);
	}

	// Inizializzazione indirizzo Server
	memset((char *)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(portNumber);

	// Creazione, settaggio opzioni e connessione socket
	datagramSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (datagramSocket < 0) {
		perror("Creazione socket.");
		exit(4);
	}
	printf("Creata la socket %d.\n", datagramSocket);

	if (setsockopt(datagramSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		perror("Set opzioni socket.");
		exit(5);
	}
	printf("Set opzioni socket ok.\n");

	if (bind(datagramSocket, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("Bind socket.");
		exit(6);
	}
	printf("Bind socket ok.\n");

	// Ciclo di ricezione richieste
	for(;;) {
		length = sizeof(struct sockaddr_in);
		if (recvfrom(datagramSocket, &request, sizeof(Request), 0, (struct sockaddr*)&cliaddr, &length) < 0) {
			perror("Errore nella recvfrom.");
			continue;
		}

		clienthost = gethostbyaddr((char *)&cliaddr.sin_addr, sizeof(cliaddr.sin_addr), AF_INET);
		if (clienthost == NULL) {
			printf("Non sono state trovate informazioni sull'host.\n");
		}
		else {
			printf("Operazione richiesta da: %s %i\n", clienthost->h_name, (unsigned)ntohs(cliaddr.sin_port));
		}

		printf("Ricevuta la richiesta di aprire il file %s\n", request.file);

		longestWord = 0;
		fp = fopen(request.file, "rt"); // rt = read text

		if (fp == NULL) { // L'apertura del file non è riuscita
			longestWord = -1;
		} else {
			do {
				c = fgetc(fp);
				currentWordCounter = 0;
				while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
					currentWordCounter++;
					c = fgetc(fp);
				}

				if (currentWordCounter > longestWord) {
					longestWord = currentWordCounter;
				}
			} while (c != EOF);

			fclose(fp);
		}

		longestWord = htonl(longestWord);
		if (sendto(datagramSocket, &longestWord, sizeof(longestWord), 0, (struct sockaddr*)&cliaddr, length) < 0) {
			perror("Errore nella sendto."); 
			continue;
		}
	}
}
