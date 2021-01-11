#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sqlite3.h>
#define PORT 7918

extern int errno;


//functie care cauta username-ul si parola. daca una dintre ele nu corespunde, intoarce 0, altfel 1
int cautare_parola (char *username, char *parola)
{
	FILE *fp;
	int exista = 0;
	char temp[512];
	char deCautat[20000];
	strcat(deCautat,username);
	strcat(deCautat, parola);
	
	if((fp = fopen("/home/prebecca/Desktop/paroleMaster", "r")) == NULL) {
		return(-1);
	}
	while(fgets(temp, 512, fp) != NULL) {
		if((strstr(temp, deCautat)) != NULL) {
			exista=1;
		}
	}
	
	if(fp) {
		fclose(fp);
	}
	
	if(exista==1) {
		return 1;
	}
	else
   		return 0;
}



//functie callback ce ajuta la exec-uri

static int callback(void* data, int argc, char** argv, char** azColName) 
{ 
    int i; 
    fprintf(stderr, "%s: ", (const char*)data); 
  
    for (i = 0; i < argc; i++) { 
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); 
    } 
  
    printf("\n"); 
    return 0; 
} 


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



		int main ()
{

  struct sockaddr_in server;	
  struct sockaddr_in from;	
  int sd;		
  int client;		 	
  int optval=1;
   
  char sql[100000];
  char *err_msg = 0;
  char *zErrMsg = 0;
  int rc;
  sqlite3 *db;
   
  char intrare[9000];
  char raspuns[9000];	
  char comanda[10000];
	
//conectare la baza de date	
	
   rc = sqlite3_open("passManager.db", &db);
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }
  
//crearea socket-ului si asteptarea conexiunii

  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket\n");
      return errno;
    }		

  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  server.sin_port = htons (PORT);
   

  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("Eroare la bind().\n");
      return errno;
    }

  if (listen (sd, 5) == -1)
    {
      perror ("Eroare la listen().\n");
      return errno;
    }

  fflush (stdout);
 

  while(1)
   {
   
//acceptarea conexiunii 
       int client;
	int len;	
	int continua =1; 
	char username[9000]={0};
	char parolaMaster[9000]={0};
	 len=sizeof(from);
 	 client = accept (sd, (struct sockaddr *) &from, &len);
	 if(client<0){
		perror("Nu se poate face legatura cu clientul la accept\n");
		continue;
	}


	if(fork() == 0)  {  
		 
            while(continua==1){					          
            
		memset(intrare, 0, sizeof(intrare));
		memset(raspuns, 0, sizeof(raspuns));

		fflush(stdout);
//citirea primei comenzi

		if(read(client,intrare,90000)<=0)
		{
		  perror("Eroare la citirea comenzii\n");
		  close (client);
		  continue;
		}
		intrare[strlen(intrare)-1]=0;
		printf("%s",intrare);		
		
//prelucrare comenzi
		strcpy(comanda,intrare);
		fflush(stdout);
		if(strcmp(comanda,"login")==0){
		
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti username-ul cu care vreti sa va logati");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda login\n");		  
		  continue;
		}
		
//primire utilizator
		printf("Am receptionat utilizatorul \n");
		fprintf(stderr,"username= %s",intrare);
		fflush(stdout);
		intrare[strlen(intrare)-1]=0;
		strcpy(username,intrare);
//primire parola master
		memset(raspuns, 0, sizeof(raspuns));
      		strcpy(raspuns, "Introduceti parola master");
      		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite masterul");
			continue;
			}
			
		memset(intrare, 0, sizeof(intrare));
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la citirea parolei\n");
		  close (client);
		  continue;
		}
		intrare[strlen(intrare)-1]=0;
		strcpy(parolaMaster, intrare);
		memset(raspuns, 0, sizeof(raspuns));
		if(cautare_parola(username,parolaMaster)==1)
		{
		strcpy(raspuns, "Aveti permisiunea de a vizualiza parolele. Va rugam introduceti una dintre comenzile: introducere, username, titlu, url, notite.");}
		else
		{
		strcpy(raspuns, "Nu aveti permisiunea de a vizualiza parolele. Fie ati introdus parola gresit, fie utilizatorul introdus nu exista. Va rugam sa va deconectati si sa incercati din nou logarea");}
		
		memset(intrare, 0, sizeof(intrare));
		
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		fflush(stdout);
		}
		
		
//introducere parola si categorie
		else if(strcmp(comanda,"introducere")==0)
		{
//cerem parola de adaugat
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti parola pe care vreti sa o salvati");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda introducere\n");		  
		  continue;
		}
		char parola[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(parola,intrare);

//cerem categoria din care face parte
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti categoria din care face parte parola");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda introducere\n");		  
		  continue;
		}
		char categorie[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(categorie,intrare);
		
//introducem parola
		memset (sql, 0, sizeof (sql));
		sprintf(sql,"INSERT INTO parole VALUES('%s','%s',NULL,NULL,NULL,NULL,'%s');",parola,categorie,username); 
		printf("%s",sql);   		
    		sqlite3_exec(db, sql, 0, 0, &err_msg); 
    		if (rc != SQLITE_OK) { 
       	printf("eroare la executarea sql!");  
       	sqlite3_free(zErrMsg);
		
		}
		else{	printf("parola introdusa cu succes");}
			memset(raspuns, 0, sizeof(raspuns));
			strcpy(raspuns,"parola introdusa cu succes");
			if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		}
		
		
		
		
//modificare username
		else if(strcmp(comanda,"username")==0){
		
//cerem parola
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti parola al carei username vreti sa il modificati");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda username\n");		  
		  continue;
		}
		char parola[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(parola,intrare);
		
		
//cerem username-ul
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti username-ul corespunzator parolei");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda introducere\n");		  
		  continue;
		}
		char usernameNou[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(usernameNou,intrare);
		
//modificam username-ul
		memset (sql, 0, sizeof (sql));
		sprintf(sql,"UPDATE parole SET username='%s' WHERE parola='%s' AND utilizator='%s';",usernameNou,parola,username); 
		printf("%s",sql);
   		
    		sqlite3_exec(db, sql, 0, 0, &err_msg); 
    		if (rc != SQLITE_OK) { 
       	printf("eroare la executarea sql!");  
       	sqlite3_free(zErrMsg);
		
		}
		else{	printf("username actualizat cu succes");}
			memset(raspuns, 0, sizeof(raspuns));
			strcpy(raspuns,"username actualizat cu succes");
			if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		} 
		
		
		
//modificare titlu
			else if(strcmp(comanda,"titlu")==0){
//cerem parola
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti parola al carei titlu vreti sa il modificati");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda titlu\n");		  
		  continue;
		}
		char parola[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(parola,intrare);
		
		
//cerem username-ul
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti titlul corespunzator parolei");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda introducere\n");		  
		  continue;
		}
		char titluNou[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(titluNou,intrare);
		
//modificam titlul
		memset (sql, 0, sizeof (sql));
		sprintf(sql,"UPDATE parole SET titlu='%s' WHERE parola='%s' AND utilizator='%s';",titluNou,parola,username); 
		printf("%s",sql);
   		
    		sqlite3_exec(db, sql, 0, 0, &err_msg); 
    		if (rc != SQLITE_OK) { 
       	printf("eroare la executarea sql!");  
       	sqlite3_free(zErrMsg);
		
		}
		else{	printf("titlu actualizat cu succes");
		    	memset(raspuns, 0, sizeof(raspuns));
			strcpy(raspuns,"titlu actualizat cu succes");
			if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		}
		}
			
			
			
//modificare url			
			else if(strcmp(comanda,"url")==0){
//cerem parola
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti parola al carei url vreti sa il modificati");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda url\n");		  
		  continue;
		}
		char parola[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(parola,intrare);
		
		
//cerem url-ul
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti url-ul corespunzator parolei");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda url\n");		  
		  continue;
		}
		char urlNou[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(urlNou,intrare);
		
//modificam url-ul
		memset (sql, 0, sizeof (sql));
		sprintf(sql,"UPDATE parole SET url='%s' WHERE parola='%s' AND utilizator='%s';",urlNou,parola,username); 
		printf("%s",sql);
   		
    		sqlite3_exec(db, sql, 0, 0, &err_msg); 
    		if (rc != SQLITE_OK) { 
       	printf("eroare la executarea sql!");  
       	sqlite3_free(zErrMsg);
		
		}
		else{printf("url actualizat cu succes");
		memset(raspuns, 0, sizeof(raspuns));
		strcpy(raspuns,"url actualizat cu succes");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		}
			
		}
		
		
		
//modificare notite
			else if(strcmp(comanda,"notite")==0){
//cerem parola
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti parola ale carei notite vreti sa le modificati");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda notite\n");		  
		  continue;
		}
		char parola[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(parola,intrare);
		
		
//cerem username-ul
		memset(raspuns, 0, sizeof(raspuns));
		memset(intrare, 0, sizeof(intrare));
		strcpy(raspuns,"introduceti notitele corespunzatoare parolei");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		if(read(client,intrare,9000)<=0)
		{
		  perror("Eroare la comanda notite\n");		  
		  continue;
		}
		char notiteNou[1000];
		intrare[strlen(intrare)-1]=0;
		strcpy(notiteNou,intrare);
		
//modificam notitele
		memset (sql, 0, sizeof (sql));
		sprintf(sql,"UPDATE parole SET notite='%s' WHERE parola='%s' AND utilizator='%s';",notiteNou,parola,username); 
		printf("%s",sql);
   		
    		sqlite3_exec(db, sql, 0, 0, &err_msg); 
    		if (rc != SQLITE_OK) { 
       	printf("eroare la executarea sql!");  
       	sqlite3_free(zErrMsg);
		
		}
		else{printf("notite actualizate cu succes");
		memset(raspuns, 0, sizeof(raspuns));
		strcpy(raspuns,"notite actualizate cu succes");
		if(write(client, raspuns, 9000)<=0){
			perror("Nu s-a putut trimite raspunsul");
			continue;
			}
		}
		}
//comanda de iesire 
	
	else if(strcmp(comanda,"iesire")==0)
	{
	memset(raspuns, 0, sizeof(raspuns));
	strcpy(raspuns,"te-ai deconectat cu succes");
	if(write(client, raspuns, 9000)<=0){
		perror("Nu s-a putut trimite raspunsul");
		continue;
		}
		
	close(client);
	continua=0; //acest continua ne asigura ca server-ul va astepta si accepta si alti clienti
	}			
	}
	} 
		}
		
sqlite3_close(db);
close (client);

}


 

