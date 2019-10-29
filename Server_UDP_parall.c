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
	int s, port, length, n, longestWord, counter, pid;
	char c;
	FILE *fp;
	struct sockaddr_in cliaddr, servaddr;
	struct hostent *clienthost;
	Request request;
	
	// Controllo argomenti
	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		exit(1);
	}

	n = 0;
	while (argv[1][n] != '\0') {
		if ((argv[1][n] < '0') || (argv[1][n] > '9')) {
			printf("Secondo argomento non intero.\n");
			printf("Usage: %s port\n", argv[0]);
			exit(2);
		}
		n++;
	}

	port = atoi(argv[1]);

	// Verifica porta
	if (port < 1024 || port > 65535) {
		printf("La porta passata in input deve essere compresa tra 1024 e 65535.\n");
		exit(3);
	}

	// Inizializzazione indirizzo Server
	memset((char *)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);

	// Creazione, settaggio opzioni e connessione socket
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("Creazione socket error ");
		exit(4);
	}
	printf("Server: creata la socket %d.\n", s);

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		perror("set opzioni socket ");
		exit(5);
	}
	printf("Server: set opzioni socket ok\n");

	if (bind(s, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("Bind socket error ");
		exit(6);
	}
	printf("Server: bind socket ok.\n");

	// Ciclo ricezione richieste
	for(;;) {
		
		length = sizeof(struct sockaddr_in);
		if (recvfrom(s, &request, sizeof(Request), 0, (struct sockaddr*)&cliaddr, &length) < 0) {
			perror("Recvfrom error ");
			continue;
		}
		clienthost = gethostbyaddr((char *)&cliaddr.sin_addr, sizeof(cliaddr.sin_addr), AF_INET);
		if (clienthost == NULL) 
			printf("client host information not found\n");
		else {
			printf("Operazione richiesta da: %s %i\n", clienthost->h_name, (unsigned)ntohs(cliaddr.sin_port));
		}

		longestWord = -1;
		counter = 0;

		pid = fork();

		if (pid == 0){

			printf("PID %d: ricevuta richiesta operazione su file: %s\n", getpid(), request.file);
			
			fp = fopen(request.file, "rt");

			if (fp != NULL) { // Apertura ok
				
				while((c = fgetc(fp))!= EOF) {
				
					if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))){
						if(counter > longestWord)
							longestWord = counter;
						counter = 0; 
						continue;
					}
					else counter++;	

					printf("%d\n", counter);
				}

				fclose(fp);
			}

			longestWord = htonl(longestWord);
			if (sendto(s, &longestWord, sizeof(longestWord), 0, (struct sockaddr*)&cliaddr, length) < 0) {
				perror("Sendto error "); 
				exit(1);
			}

			printf("PID %d: operazione conclusa con successo\n",getpid() );
			exit(0);
		}

		//controllo torna a padre
	}
}