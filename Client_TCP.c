/* Client per richiedere l'ordinamento remoto di un file */

// Il Client chiede all’utente il nome del file e il numero della linea da eliminare,

// Dopodichè invia i dati al server e riceve il nuovo contenuto del file, inserendolo
// nel file system e stampandolo a video.

// Parametri da ricevere nella argv: eseg serverAddress serverPort

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define DIM_BUFF 256

int main(int argc, char *argv[]) {
	int sd, port, fd_sorg, fd_dest, nread, line;
	FILE *fp;
	char buff[DIM_BUFF];
	int okToContinue, countLine = 0;
	// FILENAME_MAX: lunghezza massima nome file. Costante di sistema.
	char nome_sorg[FILENAME_MAX+1], nome_dest[FILENAME_MAX+1];
	char c;
	struct hostent *host;
	struct sockaddr_in servaddr;

	// Controllo argomenti
	if (argc != 3) {
		printf("Client TCP: Error:%s serverAddress serverPort\n", argv[0]);
		exit(1);
	}

	// Inizializzazione indirizzo server
	memset((char *)&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	host = gethostbyname(argv[1]);

	nread = 0;

	while (argv[2][nread] != '\0') {
		if ((argv[2][nread] < '0') || (argv[2][nread] > '9')) {
			printf("Client TCP: Secondo argomento non intero\n");
			exit(2);
		}
		nread++;
	}

	port = atoi(argv[2]);

	// Verifica port e host
	if (port < 1024 || port > 65535) {
		printf("Client TCP: %s = porta scorretta...\n", argv[2]);
		exit(3);
	}
	if (host == NULL) {
		printf("Client TCP: %s not found in /etc/hosts\n", argv[1]);
		exit(4);
	} else {
		servaddr.sin_addr.s_addr = ((struct in_addr *)(host->h_addr)) -> s_addr;
		servaddr.sin_port = htons(port);
	}

	// Corpo del Client: ciclo di accettazione di richieste da utente
	printf("Client TCP: Ciclo di richieste di eliminazione riga fino a EOF\n\n");
	printf("Client TCP: Nome del file, EOF per terminare: ");

	while (gets(nome_sorg)) {
		/* Verifico l'esistenza del file */
		if ((fd_sorg = open(nome_sorg, O_RDONLY)) < 0) {
			perror("Client TCP: File sorgente"); 
			printf("\nClient TCP: Nome del file, EOF per terminare: ");
			continue;
		}

		okToContinue = 0;
		while (!okToContinue) {
			printf("Client TCP: Inserire numero di riga da eliminare: ");
			
			if (scanf("%d", &line) != 1) {
				printf("Client TCP: Numero linea mal formattato\n");
				while (getchar() != '\n');
				continue;
			}
			getchar();
	 		
			fp = fopen(nome_sorg, "r");
		    	for (c = getc(fp); c != EOF; c = getc(fp)) {
					if (c == '\n') {
			    		countLine++;
					}
				} 
		   
		    	fclose(fp); 

		    	if (countLine < line) {
		    		printf("Client TCP: Numero righe del file minore della riga richiesta\n");
		    		continue;
		    	}
				else if (line < 1) {
				printf("Client TCP: Il numero di riga deve essere positivo\n");
		    	continue;
			}

		    okToContinue = 1;
		}
		
		okToContinue = 0;

		while (!okToContinue) {
			printf("Client TCP: Inserisci nome del file destinazione: ");
			if (gets(nome_dest) == 0) {
				printf("Client TCP: Errore lettura nome del file di destinazione\n");
				continue;
			}
			okToContinue = 1;
		}
		
		// Verifico creazione file
		if ((fd_dest = open(nome_dest, O_WRONLY | O_CREAT, 0644)) < 0) {
			perror("Client TCP: File destinazione");
			exit(5);
		}

		// Creazione socket
		sd = socket(AF_INET, SOCK_STREAM, 0);
		if (sd < 0) {
			perror("Client TCP: Apertura socket");
			exit(6);
		}
		
		printf("\nClient TCP: Creata la socket sd=%d\n", sd);

		// Operazione di BIND implicita nella connect
		if (connect(sd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
			perror("Connect"); 
			exit(7);
		}

		printf("Client TCP: Connessione effettuata\n");

		// Invio del numero della linea da eliminare
		write(sd, &line, sizeof(int));

		// Invio file
		while ((nread = read(fd_sorg, buff, DIM_BUFF)) > 0) {
			write(sd,buff,nread);
		}
		shutdown(sd,1);

		// Ricezione file
		printf("Client TCP: Ricevo e stampo file senza riga\n---------------\n");
		while ((nread = read(sd, buff, DIM_BUFF)) > 0) {
			write(fd_dest,buff,nread);
			write(1,buff,nread);
		}
		
		printf("---------------\nClient TCP: Trasferimento terminato\n\n");
		// Chiusura socket in ricezione
		shutdown(sd, 0);
		// Chiusura file
		close(fd_sorg);
		close(fd_dest);
		close(sd);

		printf("Client TCP: Nome del file, EOF per terminare: ");
	}

	printf("\nClient TCP: termino...\n");
	exit(8);
}