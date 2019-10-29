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

#define MAX_LENGTH 256

int main(int argc, char * argv[]){

	while(1){
		int portNumber, i, backlog;
		char buff[MAX_LENGTH];
		int readSocket, numLinea, contaLinea = 1;


		int streamSocket, newSocket, result, addrLen = sizeof(struct sockaddr_in);
		struct sockaddr_in * servAddr;
		
		struct hostent * hp;
		int peeraddrLen;
		struct sockaddr_in * peerAddr;

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

		servAddr->sin_family = AF_INET;
		servAddr->sin_port = portNumber;
		if(hp = gethostbyname("localhost"))
			servAddr->sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
		else {
			printf("Server TCP: Errore nella ricezione dell'Hostname del Client\n");
			exit(7);
		}
		printf("Server TCP: ServerAddress[localhost, AF_INET, %d]\n", portNumber);

		// Binding della Socket
		result = bind(streamSocket, servAddr, &addrLen);
		if(result < 0){
			printf("Server TCP: Errore durante il binding\n");
			exit(5);
		}
		printf("Server TCP: Binding della Socket effettuato\n");

		// La Socket si mette in attesa di un messaggio
		result = listen(streamSocket, backlog);
		if(result < 0){
			printf("Server TCP: Errore durante la listen\n");
			exit(6);
		}
		printf("Server TCP: Ricevuta richiesta di Servizio\n");

		//Inizializzo l'address del peer
		peerAddr->sin_family = AF_INET;
		peerAddr->sin_port = portNumber;
		if(hp = gethostbyname("localhost"))
			peerAddr->sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
		else {
			printf("Server TCP: Errore nella ricezione dell'Hostname del Client\n");
			exit(7);
		}
		printf("Server TCP: PeerAddress[localhost, AF_INET, %d", portNumber);

		// Accetto la richiesta in arrivo
		newSocket = accept(streamSocket, peerAddr, &peeraddrLen);
		if(newSocket < 0){
			printf("Server TCP: Errore nella accept\n");
			exit(8);
		}
		printf("Server TCP: Accettata connessione con il Peer\n");

		// Leggo il numero di riga da cancellare
		readSocket = read(streamSocket, buff, MAX_LENGTH);
		if(readSocket < 0){
			printf("Server TCP: Errore nella lettura della linea\n");
			exit(9);
		}
		numLinea = atoi(buff);
		printf("Server TCP: Ricevuta linea n.%d da cancellare\n", numLinea);

		// Leggo il file
		while(readSocket = read(streamSocket, buff, MAX_LENGTH) > 0){
			if(numLinea != contaLinea)
				write(streamSocket, buff, MAX_LENGTH);
			contaLinea++;
		}
		printf("Server TCP: Letto il file e cancellata la riga\n");
	}

}