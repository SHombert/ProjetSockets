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

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

/* Structure client connecté */
struct chatClient
{
  char pseudo[15];
  int  scoket;
};

/* Tableau de clients connectés */
const int maxLogged = 150;
int clients[maxLogged];


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
      int retour = pthread_create (& thread1_id, NULL, traitementConnexion, (void*)NULL))
      if(retour != 0){
        perror ("Erreur lors de la création du thread");
        exit (1);
      }

      pthread_join(thread1_id,(void**)NULL);

      close(new_socket);
    }


}

void traitementConnexion (){


}
/*------------------------------------------------------*/

/*------------------------------------------------------*/
main(int argc, char **argv)
{

  int socket_descriptor;          /* descripteur de socket */

  sockaddr_in adresse_locale;      /* structure d'adresse locale*/
 
  hostent *ptr_hote;                /* les infos recuperees sur la machine hote */
  servent *ptr_service;             /* les infos recuperees sur le service de la machine */
  char machine[TAILLE_MAX_NOM + 1]; /* nom de la machine locale */


/*-----------------------------------------------------------*/

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
