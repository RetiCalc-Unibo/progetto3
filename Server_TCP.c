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
#include <ctype.h>

#include <time.h>

#define DIM_BUFF 4096

int main(int argc, char * argv[]){

	int pid;

	int portNumber, i;
	char c;
	int readSocket, numLinea, contaLinea = 1;

	int streamSocket, newSocket, result, addrLen = sizeof(struct sockaddr_in);
	
	struct hostent * hp;
	int peeraddrLen;
	struct sockaddr_in servAddr, peerAddr;
	
	char buff[DIM_BUFF];

	clock_t start, diff_time;
	int msec;

	// Controllo Argomenti
	if(argc == 1){
		// Se la porta non è passata in argomento, selezioniamo la porta 1050
		portNumber = 1050;
		printf("Server TCP: Inizializzato sulla porta [1050].\n");
	} else if(argc == 2){
		// Controllo che se la porta è passata in input, sia numerica
		for(i = 0; i < strlen(argv[1]); i++){
			if(!isdigit(argv[1][i])){
				printf("Server TCP: La porta passata in input non è numerica.\n");
				exit(1);
			}
		}
		portNumber = atoi(argv[1]);
	} else {
		// Gestisco un numero di argomenti incongruo
		printf("Server TCP: Utilizzo -> %s [porta]", argv[0]);
		exit(2);
	}

	if(portNumber < 1024 || portNumber > 65535){
		printf("Server TCP: 1024 < porta < 65536.\n");
		exit(3);
	}

	// Inizializzazione indirizzo Server
	streamSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(streamSocket < 0){
		printf("Server TCP: Errore durante l'inizializzazione della Socket\n");
		exit(4);
	}
	printf("Server TCP: Inizializzata la Socket\n");

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(portNumber);
	printf("Server TCP: ServerAddress[localhost:%d, AF_INET]\n", portNumber);

	// Binding della Socket
	result = bind(streamSocket, (struct sockaddr *) &servAddr, addrLen);
	if(result < 0){
		printf("Server TCP: Errore durante il binding\n");
		exit(5);
	}
	printf("Server TCP: Binding della Socket effettuato\n\n");
	
	// La Socket si mette in attesa di un messaggio
	result = listen(streamSocket, 5);
	if(result < 0){
		printf("Server TCP: Errore durante la listen\n");
		exit(6);
	}

	while(1){

		// Accetto la richiesta in arrivo
		newSocket = accept(streamSocket, (struct sockaddr *) &peerAddr, &peeraddrLen);
		if(newSocket < 0){
			printf("Server TCP: Errore nella accept\n");
			exit(7);
		}

		pid = fork();
		if(pid < 0){
			perror("fork");
			exit(8);
		} else if(pid == 0){
			close(streamSocket);
			printf("Server TCP: Accettata connessione con il Peer\n");

			// Leggo il numero di riga da cancellare
			if(readSocket = read(newSocket, &numLinea, sizeof(int)) < 0){
				perror("Server TCP: Linea");
				printf("Server TCP: Errore nella lettura della linea\n");
				exit(9);
			}
			printf("Server TCP: Ricevuta linea n.%d da cancellare\n", numLinea);


			start = clock();
			// Leggo il file
			// Algoritmo con lettura carattere per carattere
			/*while((readSocket = read(newSocket, &c, sizeof(char))) > 0){
				//write(1, &c, sizeof(char));
				if(numLinea != contaLinea)
					write(newSocket, &c, sizeof(char));
				if(c == '\n')
					contaLinea++;
			}*/

			// Algoritmo con lettura buffer 256 caratteri	
			while((readSocket = read(newSocket, &buff, DIM_BUFF)) > 0){
				
				i = 0;
				while(i < readSocket){
					if(numLinea != contaLinea){
						write(newSocket, &(buff[i]), sizeof(char));
					} else while(buff[i] != '\n' && i < readSocket) i++;
					
					if(buff[i] == '\n') {
						contaLinea++;
					}
					i++;
				}
				
			}
			diff_time = clock() - start;
			msec = diff_time * 1000 / CLOCKS_PER_SEC;

			printf("Tempo impiegato: %d millisecondi\n", msec%1000);

			shutdown(newSocket, 0);
			shutdown(newSocket, 1);
			printf("Server TCP: Letto il file e cancellata la riga\n\n");
			exit(0);
		} else {
			close(newSocket);
		}
		
	}

}
