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

/* Structure message standard*/

typedef struct msg
{
  int type;
  char pseudo[15];
  char message[255];
};

/* Structure liste clients */

typedef struct listCos 
{
  char listePseudos[MAX_LOGGED][15];

};// envoi de la liste des connectés lors de la connexion d'un client + si demande explicite


/**** Declaration procédures *****/
void ecoute(int listeningSocket);
void *traitementConnexion();
char *lecturePseudoBuffer(char buffer[]);
int ajouterClient(char *pseudo, int sock);
void supprimerClient(int sock);
int getClientSocket(char pseudo[]);
char *getClientPseudo(int sock);

/* Tableau de clients connectés */

struct chatClient clients[MAX_LOGGED]; // tableau des structs socket/pseudo
struct listCos pseudosClients;
int nbClientCo;

/** 
 * Methode en attente de connexion au serveur
 * Creation d'un socket et d'un thread correspondant pour chaque connexion de client et appel a la methode thread de traitement des donnees
 */

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

/** 
 * Traite les donnees selon le type du message envoye (premier champ de la structure du buffer)
 */
void *traitementConnexion (void *socket){

    int sock = *(int*)socket;
    struct msg buffer, newBuffer;
    int longueur, sockDest, success;
   
    if ((longueur = read(sock, (char*) &buffer, sizeof(buffer))) <= 0)
    	return (NULL);
    
    
    //char* pseudo;
    switch (buffer.type) {

      /**
       * Type = 0 : Connexion au serveur
       * Pseudo -> Pseudo du client a ajouter dans la liste des clients connectes
       * Message -> vide
       */ 
      case 0 : 
        if(nbClientCo < 150) {
          success = ajouterClient(buffer.pseudo,sock);
          write(sock,(char*) &pseudosClients,sizeof(newBuffer)); // Renvoi de la liste des clients connectés
          if (success == -1){ // Si pseudo deja utilise
            newBuffer.type = 6;
            write(sock,(char*) &newBuffer,sizeof(newBuffer));
          }
        }else{
          // si taille max atteinte, envoi message erreur
          newBuffer.type = 5;
          write(sock,(char*) &newBuffer,sizeof(newBuffer));
        }
      
        break;

      /**
       * Type = 1 : Envoi de message
       * Pseudo -> Pseudo du destinataire
       * Message -> message a envoyer
       */
      case 1 :

        sockDest = getClientSocket(buffer.pseudo); // socket du destinataire
        if(sockDest != -1) { // si pseudo trouve
          newBuffer.type = 1;
          strcpy(newBuffer.pseudo, getClientPseudo(sock));
          strcpy(newBuffer.message, buffer.message);
        }else{
          newBuffer.type = 7; // sinon message erreur
        }
        write(sockDest,(char*) &newBuffer,sizeof(newBuffer));
        break;

      /**
       * Type = 2 : Déconnexion
       * PSeudo -> vide
       * Message -> vide
       */ 
      case 2 : 
        supprimerClient (sock);
        break;

      /**
       * Type = 3
       * Pseudo -> vide
       * Message -> vide
       */
      case 3 : 
        write(sock,(char*) &pseudosClients,sizeof(newBuffer)); // Renvoi de la liste des clients connectés
        break;

      /**
       * Type inconnu
       * Envoie de message erreur
       */
      default : 
        newBuffer.type = 8;
        write(sock,(char*) &newBuffer,sizeof(newBuffer));
    }
    
}

/** 
 * Methode pour ajouter le client avec son socket et son pseudo dans la liste des clients connectes 
 * renvoie 0 si le client a bien ete ajoute, -1 sinon (ie. Le pseudo est deja utilise)
*/
int ajouterClient(char pseudo[], int sock){
    int i=0;
    while(clients[i].pseudo != pseudo && i<nbClientCo){
      ++i;
    }
    if(i==nbClientCo){ // On n'a pas trouvé le pseudo, on peut ajouter le client
      struct chatClient newClient;
      strcpy(newClient.pseudo,pseudo);
      newClient.socket = sock;
      clients[nbClientCo]=newClient;
      strycpy(clients[nbClientCo].pseudo,pseudosClients.listePseudos[nbClientCo]);
      ++ nbClientCo;
      return 0;
    }else{
      return -1;
    }
}

/** 
 * Méthode pour supprimer le client de notre tableau de clients connectés lorsqu'il se déconnecte
 * Tableau sans cade vide ==> on trouve le client à supprimer, et on décale tous les suivants d'un cran vers la gauche */
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
      strycpy(clients[i].pseudo,pseudosClients.listePseudos[i-1]);   
    }
    --nbClientCo;
}

/** Methode pour recuperer le numero de socket a partir du pseudo dans la liste des clients connectes */
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

/** Méthode pour récupérer le pseudo du client à partir du numéro de socket */
char *getClientPseudo(int sock){
  char *pseudo;
  int i=0;
  while(clients[i].socket != sock && i<nbClientCo){
     ++i;
  }
  if(i<nbClientCo){
   strcpy(pseudo,clients[i].pseudo);
  }
  return pseudo;
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
