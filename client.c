/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Bibliothèques ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>

/* Constantes */
#define TAILLE_MAX_MESS 256
#define TAILLE_MAX_PSEUDO 15
#define MAX_LOGGED 150
#define h_addr h_addr_list[0]

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

//message a envoyer au server
typedef struct _msg
{
        int type;
        char pseudo[15]; //destinataire msg.
        char message[255];
};

//liste des connectés
typedef struct _list
{
        char listePseudos[MAX_LOGGED][TAILLE_MAX_PSEUDO];
        int nbCo;
};
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Type de Procédure ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int connection(char pseudo[15]);
void sendAMessage();
void disconnection();
int verificationId(char pseudo[15]);
void readConnect();

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Programme Principal ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int socket_descriptor;  /* descripteur de socket */
struct _list pseudoCos; /* Liste des utilisateurs connectés */

char pseudo[TAILLE_MAX_PSEUDO];
struct _msg bufferServ, bufferCli; // Buffer des sockets

sockaddr_in adresse_locale; /* structure d'adresse locale*/
hostent *ptr_host;          /* infos récupérées sur la machine hote */
servent *ptr_service;       /* infos récupérées sur le service de la machine */
char *host;

int main(int argc, char **argv)
{

        int longueur, longueurStdin; /* longueur d'un buffer utilisé */
        char *buffer;                // Buffer de l'entrée std
        char *mess, *pseudoDest;     // message entré par l'utilisateur
        char pseudoTemp[TAILLE_MAX_PSEUDO];
        int connected, typeRet;
        fd_set rdfs;

        /*Corps de l'algo*/
        connected = 1; // Permet de quitter l'écoute lorsque passé à zéro

        printf("Bonjour ! Bienvenue sur le chat 100\% féminin ! /n");

        // Demande du pseudo
        do
        {
                printf("Quel est votre pseudo de connexion ? 15 caractères maximum \n");
                scanf("%s", pseudoTemp);
                longueur = strlen(pseudoTemp);
                if (longueur > 16)
                {
                        printf("pseudo trop long");
                }

        } while (longueur > 16);
        printf("Vous avez rentré : %s", pseudoTemp);

        // Demande de connexion au serveur
        typeRet = connection(pseudoTemp);
        switch (typeRet)
        {
        case 5:
                printf("Le serveur de chat est complet. Veuillez réessayer plus tard.");
                close(socket_descriptor);
                printf("connexion avec le serveur fermee, fin du programme.\n");
                connected = 0;
                break;
        case 6:
                printf("Le pseudo est déjà utilisé.");
                // askPseudo
                break;
        case 0:
                printf("Vous êtes connecté au chat ! \n");
                printf("--> Pour quitter, tapez q. \n");
                printf("--> Pour envoyer un message, tapez 'send'.\n");
                strcpy(pseudo, pseudoTemp);
                break;

        default:
                break;
        }

        // Récupération de la liste des personnes connectées

        while ((longueur = read(socket_descriptor, (char *)&pseudoCos, sizeof(pseudoCos))) <= 0)
        {
        }
        readConnect(); //lecture et affichage de la liste des clients connectés envoyée par le serveur
        while (connected)
        {
                FD_ZERO(&rdfs);
                FD_SET(STDIN_FILENO, &rdfs);
                FD_SET(socket_descriptor, &rdfs);
                if (select(socket_descriptor + 1, &rdfs, NULL, NULL, NULL) == -1)
                {
                        perror("Erreur avec la fonction select");
                        exit(-1);
                }

                // Si entrée clavier
                if (FD_ISSET(STDIN_FILENO, &rdfs))
                {

                        longueurStdin = lire(buffer, TAILLE_MAX_MESS);
                        if (strcmp(buffer, "q") == 0)
                        {
                                disconnection();
                        }
                        else if (strcmp(buffer, "send") == 0)
                        {

                                bufferCli.type = 1;
                                printf("Pseudo destinataire : ");
                                longueurStdin = lire(buffer, TAILLE_MAX_MESS);
                                strcpy(bufferCli.pseudo, buffer);
                                //fgets(pseudoDest,TAILLE_MAX_PSEUDO,stdin);
                                //longueurStdin = lire(pseudoDest,TAILLE_MAX_PSEUDO);
                                //scanf("%s", bufferCli.pseudo);

                                printf("\n");
                                printf("Message : ");
                                longueurStdin = lire(buffer, TAILLE_MAX_MESS);
                                strcpy(bufferCli.message, buffer);
                                printf("\n");

                                //longueurStdin = lire(mess,TAILLE_MAX_MESS);
                                write(socket_descriptor, (char *)&bufferCli, sizeof(bufferCli));
                        }
                }
                // Cas d'un message du serveur entrant
                else if (FD_ISSET(socket_descriptor, &rdfs))
                {
                        longueur = read(socket_descriptor, (char *)&bufferServ, sizeof(bufferServ));
                        switch (bufferServ.type)
                        {
                        case 1: // reception d'un message
                                printf("%s a écrit : ", bufferServ.pseudo);
                                printf("%s \n", bufferServ.message);
                                break;

                        case 7:
                                printf("Il semblerait que le destinataire ne soit pas connecté au chat. Veuillez réessayer plus tard ou vérifier le pseudo. \n");
                                break;

                        default:
                                break;
                        }
                        //printf("Server: ");
                        //puts(buffer);
                }
        }
}

/*
Méthodes de gestion d'entrée utilisateur via fgets
https://yard.onl/sitelycee/cours/c/_index.html?Rcuprerunechanedecaractres.html */
void viderBuffer()
{
        int c = 0;
        while (c != '\n' && c != EOF)
        {
                c = getchar();
        }
}

int lire(char *chaine, int longueur)
{
        char *positionEntree = NULL;
        if (fgets(chaine, longueur, stdin) != NULL)
        {
                positionEntree = strchr(chaine, '\n');
                if (positionEntree != NULL)
                {
                        *positionEntree = '\0';
                }
                else
                {
                        viderBuffer();
                }
                return 1;
        }
        else
        {
                viderBuffer();
                return 0;
        }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Définition des fonctions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**  int verificationId(char pseudoTemp[15], struc msg)
        {
                //lexique local
                int valid = 0; //pseudo non utilisé si valid=0, utilisé si valid=1

                //envoie pseudo server et attente reponse du server pour savoir si pseudo existe déjà.
                msg->type = 0;
                msg->pseudo = pseudoTemp;
                write(sockPipe, (char *)&msg, sizeof(msg));

                //lecture retour serveur
                read(sockPipe, (char *)&msg, sizeof(msg));

                //traitement
                if (msg.type == 6)
                {
                        return valid;
                }
                else
                {
                        return valid = 1;
                }
        };*/

/**
 *      Renvoie le type de message retourné par le serveur après la connexion 
 * 
 */
int connection(char pseudo[15])
{

        host = "localhost";
        int longueur;
        if ((ptr_host = gethostbyname(host)) == NULL)
        {
                perror("erreur : impossible de trouver le serveur a partir de son adresse.");
                exit(1);
        }

        /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
        bcopy((char *)ptr_host->h_addr, (char *)&adresse_locale.sin_addr, ptr_host->h_length);
        adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */

        /* Nouveau numéro de port : 5000 */
        adresse_locale.sin_port = htons(5000);

        /* creation de la socket */
        if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
                perror("erreur : impossible de creer la socket de connexion avec le serveur.");
                exit(1);
        }

        /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
        if ((connect(socket_descriptor, (sockaddr *)(&adresse_locale), sizeof(adresse_locale))) < 0)
        {
                perror("erreur : impossible de se connecter au serveur.");
                exit(1);
        }

        //type=0 signifie une connexion au serveur
        bufferCli.type = 0;
        strcpy(bufferCli.pseudo, pseudo);
        write(socket_descriptor, (char *)&bufferCli, sizeof(bufferCli));
        while ((longueur = read(socket_descriptor, (char *)&bufferServ, sizeof(bufferServ))) <= 0)
        {
        }
        return (bufferServ.type);
};

void disconnection()
{
        struct _msg bufferCli;
        //type = 2 signifie au serveur une volonté de se déconnecter
        bufferCli.type = 2;
        write(socket_descriptor, (char *)&bufferCli, sizeof(bufferCli));
        printf("Vous quittez le chat féminin, au revoir et à bientôt !\n");
        close(socket_descriptor);
        printf("connexion avec le serveur fermee, fin du programme.\n");
        exit(0);
};

//Fonction qui affiche la liste des connectés au chat
void readConnect()
{
        // TODO
        int i = 0;
        printf("===========================\n");
        printf("Utilisateurs connectés :\n");
        while (i < pseudoCos.nbCo)
        {
                printf("%s\n", pseudoCos.listePseudos[i]);
                i++;
        }
        printf("===========================\n");
}
