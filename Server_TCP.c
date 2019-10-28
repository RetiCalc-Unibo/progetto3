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

typedef struct {
	char file[MAX_LENGTH];
} Request;

int main(int argc, char * argv[]){

	int portNumber, i, backlog;
	char c;
	FILE * fp;

	int streamSocket, newSocket, result, addrLen = sizeof(struct sockaddr_in);
	struct sockaddr_in * servAddr;
	
	struct hostent * hp;
	int peeraddrLen;
	struct sockaddr_in peerAddr;

	Request request;

	// Controllo Argomenti
	if(argc == 1){
		// Se la porta non è passata in argomento, selezioniamo la porta 1050
		portNumber = 1050;
		printf("Server TCP: Inizializzato sulla porta [1050].\n");
	} else if(argc == 2){
		// Controllo che se la porta è passata in input, sia numerica
		for(i = 0; i < strlen(argv[1]); i++){
			if(!isnumeric(argv[1][i])){
				printf("Server TCP: La porta passata in input non è numerica.\n");
				exit(1);
			}
		}
		portNumber = atoi(argv[1]);
	} else {
		// Gestisco un numero di argomenti incongruo
		printf("Server TCP - Utilizzo: %s [porta]", argv]0]);
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

	result = bind(streamSocket, servAddr, addrLen);
	if(result < 0){
		printf("Server TCP: Errore durante il binding\n");
		exit(5);
	}

	result = listen(streamSocket, backlog);
	if(result < 0){
		printf("Server TCP: Errore durante la listen\n");
		exit(6);
	}

	peerAddr.sin_familiy = AF_INET;
	peerAddr.sin_port = portNumber;
	if(hp = gethostbyname("localhost"))
		peerAddr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	else {
		printf("Server TCP: Errore nella ricezione dell'Hostname del Client\n");
		exit(7);
	}

	newSocket = accept(streamSocket, peerAddr, &peeraddrLen);
	if(newSocket < 0){
		printf("Server TCP: Errore nella accept\n");
		exit(8);
	}
}