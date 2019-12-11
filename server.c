/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> /* pour les sockets */
#include <sys/socket.h>

#include <netdb.h>  /* pour hostent, servent */
#include <string.h> /* pour bcopy, ... */
#include <pthread.h> /* librairie threads */

#define TAILLE_MAX_NOM 256
#define MAX_LOGGED 150




typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

/* Structure client connecté */

typedef struct chatClient
{
  char pseudo[15];
  int  socket;
};

/* Structure messages */

typedef struct msg
{
  int type;
  char pseudo[15];
  char message[255];
};

//read(descripteurSocket,(char* )&grille,sizeof(grille));


/**** Declaration procédures *****/
void ecoute();
void *traitementConnexion();
char *lecturePseudoBuffer(char buffer[]);
void ajouterClient(char *pseudo, int sock);
void supprimerClient(int sock);

/* Tableau de clients connectés */

struct chatClient clients[MAX_LOGGED];
int nbClientCo;

void ecoute (int listeningSocket){

  int longueur_adresse_courante, 
      new_socket;
  sockaddr_in adresse_client_courant;  

  /* attente des connexions et traitement des donnees recues */
  for (;;)
    {

      longueur_adresse_courante = sizeof(adresse_client_courant);

      /* adresse_client_courant sera renseignée par accept via les infos du connect */
      if ((new_socket =
              accept(listeningSocket,
                      (sockaddr *)(&adresse_client_courant),
                      &longueur_adresse_courante)) < 0)
      {
        perror("erreur : impossible d'accepter la connexion avec le client.");
        exit(1);
      }

      pthread_t thread1_id;
      // passer le socket en pointeur nul
      int retour = pthread_create (& thread1_id, NULL, traitementConnexion, (void*) &new_socket);
      if(retour != 0){
        perror ("Erreur lors de la création du thread");
        exit (1);
      }
    
    }

}

void *traitementConnexion (void *socket){

   int sock = *(int*)socket;
    struct msg buffer;
    int longueur;
   
    if ((longueur = read(sock, (char*) &buffer, sizeof(buffer))) <= 0) 
    	return (NULL);
    
    
    //char* pseudo;
    switch (buffer.type) {

      case 0 : // Type = 0 : Connexion au serveur
        // if(nbClientCo =150) envoyer erreur
        if(nbClientCo < 150) {
            //pseudo = lecturePseudoBuffer(buffer);
            ajouterClient(buffer.pseudo,sock);
            //free(pseudo);
        }
        break;

      case 1 : // Type = 1 : Envoi de message
        //pseudo = lecturePseudoBuffer(buffer);
        int sockDest = getClientSocket(buffer.pseudo);
        

      break;

      case 2 : // Type = 2 : Déconnexion
        supprimerClient (sock);
        break;

      default : // envoyer erreur
      break;
      

    }
    
}

/*char *lecturePseudoBuffer(char buffer[]){
        char *result;
        result = malloc((15+1)*sizeof(char)); // allocation taille 15 caractère
        int i = 1;
        while (buffer[i] != '\0' && buffer[i] != ' ' && i <= 14){

                    result[i-1] = buffer[i];
                    ++i;
                    
            }
        result[i]="\0";
        return result;
}*/

void ajouterClient(char pseudo[], int sock){
    struct chatClient newClient;
    newClient.pseudo = pseudo;
    newClient.socket = sock;
    clients[nbClientCo]=newClient;
    ++ nbClientCo;
}

/** Tableau sans cade vide ==> on trouve le client à supprimer, et on décale tous les suivants d'un cran vers la gauche */
void supprimerClient(int sock){
    int indexClient=0;
    int i=0;
    int trouve=0;
    while(!trouve && i<MAX_LOGGED){
      if(clients[i].socket = sock){
        trouve=1;
        indexClient=i;
      }
      i++;
    }
    while(i<nbClientCo){
      clients[i-1]=clients[i];
    }
    --nbClientCo;
}

int getClientSocket(char pseudo[]){
  int i=0;
  int sockRes;
  while(clients[i].pseudo != pseudo && i<nbClientCo){
    ++i;
  }
  if(i<nbClientCo){
    sockRes=clients[i].socket;
  }else{
    sockRes=-1;
  }
  return sockRes;
}

/*------------------------------------------------------*/
  main(int argc, char **argv){
  
  nbClientCo = 0;

  int socket_descriptor;          /* descripteur de socket */

  sockaddr_in adresse_locale;      /* structure d'adresse locale*/
 
  hostent *ptr_hote;                /* les infos recuperees sur la machine hote */
  servent *ptr_service;             /* les infos recuperees sur le service de la machine */
  char machine[TAILLE_MAX_NOM + 1]; /* nom de la machine locale */

  gethostname(machine, TAILLE_MAX_NOM); /* recuperation du nom de la machine */

  /* recuperation de la structure d'adresse en utilisant le nom */
  if ((ptr_hote = gethostbyname(machine)) == NULL)
  {
    perror("erreur : impossible de trouver le serveur a partir de son nom.");
    exit(1);
  }

  /* initialisation de la structure adresse_locale avec les infos recuperees */

  /* copie de ptr_hote vers adresse_locale */
  bcopy((char *)ptr_hote->h_addr, (char *)&adresse_locale.sin_addr, ptr_hote->h_length);
  adresse_locale.sin_family = ptr_hote->h_addrtype; /* ou AF_INET */
  adresse_locale.sin_addr.s_addr = INADDR_ANY;      /* ou AF_INET */

 
  /* Nouveau numero de port */
  adresse_locale.sin_port = htons(5000);
  

  printf("numero de port pour la connexion au serveur : %d \n",
         ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);

  /* creation de la socket */
  if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("erreur : impossible de creer la socket de connexion avec le client.");
    exit(1);
  }

  /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
  if ((bind(socket_descriptor, (sockaddr *)(&adresse_locale), sizeof(adresse_locale))) < 0)
  {
    perror("erreur : impossible de lier la socket a l'adresse de connexion.");
    exit(1);
  }

  /* initialisation de la file d'ecoute */
  listen(socket_descriptor, 5);
  

/*-----------------------------------------------------------*/
  ecoute(socket_descriptor);

  
  
}
