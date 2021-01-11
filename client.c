#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 7918
extern int errno;

//functii culori
void reset () {
  printf("\033[0m");
}

void rosu () {
  printf("\033[1;31m");
}

void galben () {
  printf("\033[1;33m");
}


int main (int argc, char *argv[])
{
	int sd;
	struct sockaddr_in server;
	sd= socket (AF_INET, SOCK_STREAM, 0);

	if(sd<0){
	 perror("Eroare la socket\n");
	 return errno; 
 	}
  	
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	int conectare= connect(sd, (struct sockaddr*) &server, sizeof(struct sockaddr));
	if(conectare==-1){
	 perror("Eroare la conectare\n");
	 return errno;
	}

	rosu();
	printf("Conectarea la server a reusit!\n");
	reset();

	galben();
        printf("Bine ati venit la PasswordManager! Pentru a continua va rugam sa va introduceti fie comanda 'login', fie sa va deconectati prin comanda 'iesire'\n");
	reset();
	
    	while(1){

	char intrare[9000];
  	char raspuns_server[9000];
	fflush(stdout);
	memset(intrare,0,sizeof(intrare));
	read(0, intrare, 9000);

	if (write (sd, intrare, 9000) <= 0)
        {
            perror ("Nu s-a putut trimite mesajul catre server\n");
            return errno;
        }
	memset(raspuns_server,0,sizeof(raspuns_server));
	if(read(sd, raspuns_server, 9000) < 0)
	{
	    perror("Nu s-a putut primi raspunsul de la server\n");
	    return errno;
	}
	
	else	
	rosu();
	printf("%s\n",raspuns_server);
	reset();
	if(strcmp(raspuns_server,"te-ai deconectat cu succes")==0)
	{
	
	close(sd);
	return 0;
	}

	}
   
 	
	close(sd);
 
}

