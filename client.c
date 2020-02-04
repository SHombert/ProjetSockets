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
#define MAX_LOGGED 3
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
        char pseudo[TAILLE_MAX_PSEUDO]; //destinataire msg.
        char message[TAILLE_MAX_MESS];
};

//liste des connectés
typedef struct _list
{
        char listePseudos[MAX_LOGGED][TAILLE_MAX_PSEUDO];
        int nbCo;
};
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Type de Procédure ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int connection(char pseudo[TAILLE_MAX_PSEUDO]);
void sendAMessage();
void disconnection(int notif);
int verificationId(char pseudo[TAILLE_MAX_PSEUDO]);
void readConnect();
int lire(char *chaine, int longueur);
void viderBuffer();
void askPseudo();

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Programme Principal ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct _list pseudoCos;                /* Liste des utilisateurs connectés */
char pseudo[TAILLE_MAX_PSEUDO];        /* Pseudo de l'utilisateur courant */
char destRegistred[TAILLE_MAX_PSEUDO]; /* Pseudo du destinataire enregistré */
int isDest;                            /* booleen pour verifier si un destinataire est enregistré ou non */

struct _msg bufferServ, bufferCli; /* Buffer des sockets */
int socket_descriptor;             /* descripteur de socket */
sockaddr_in adresse_locale;        /* structure d'adresse locale*/
hostent *ptr_host;                 /* infos récupérées sur la machine hote */
servent *ptr_service;              /* infos récupérées sur le service de la machine */
char *host;

int main(int argc, char **argv)
{

        int longueur, longueurStdin; /* longueur d'un buffer utilisé */
        char *buffer;                // Buffer de l'entrée std
        char *mess, *pseudoDest;     // message entré par l'utilisateur
        char pseudoTemp[TAILLE_MAX_PSEUDO];

        int typeRet; // Type retourné lors de la connexion au serveur
        fd_set rdfs; // Gestion du select

        /*Corps de l'algo*/
        isDest = 0;
        printf("Bonjour ! Bienvenue sur le chat féminin ! \n");


        askPseudo();

        // Récupération de la liste des personnes connectées

        while ((longueur = read(socket_descriptor, (char *)&pseudoCos, sizeof(pseudoCos))) <= 0)
        {
        }
        readConnect(); //lecture et affichage de la liste des clients connectés envoyée par le serveur
        while (1)
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
                                disconnection(1);
                        }
                        else if (strcmp(buffer, "+send") == 0)
                        {

                                bufferCli.type = 1;
                                printf("Pseudo destinataire : ");
                                longueurStdin = lire(buffer, TAILLE_MAX_MESS);
                                strcpy(bufferCli.pseudo, buffer);
                                strcpy(destRegistred, buffer);
                                isDest = 1;
                                printf("* Vous commencez la conversation avec %s ... \n", destRegistred);
                                
                        }
                        else if (strcmp(buffer, "+global") == 0)
                        {
                                printf("Message : ");
                                longueurStdin = lire(buffer, TAILLE_MAX_MESS);
                                bufferCli.type = 4;
                                strcpy(bufferCli.message, buffer);
                                write(socket_descriptor, (char *)&bufferCli, sizeof(bufferCli));
                        }
                        else if (strcmp(buffer, "+liste") == 0)
                        {
                                bufferCli.type = 3;
                                write(socket_descriptor, (char *)&bufferCli, sizeof(bufferCli));
                                if (longueur = read(socket_descriptor, (char *)&pseudoCos, sizeof(pseudoCos)) > 0)
                                {
                                        readConnect();
                                }
                        }
                        // Si destinataire enregistré, lecture directe du message
                        else if (isDest == 1)
                        {

                                if (strcmp(buffer, "\0") != 0) // On évite d'envoyer des sauts de lignes
                                {
                                        bufferCli.type = 1;
                                        strcpy(bufferCli.message, buffer);
                                        strcpy(bufferCli.pseudo, destRegistred);
                                        write(socket_descriptor, (char *)&bufferCli, sizeof(bufferCli));
                                }
                        }
                }
                // Cas d'un message du serveur entrant
                else if (FD_ISSET(socket_descriptor, &rdfs))
                {

                        longueur = read(socket_descriptor, (char *)&bufferServ, sizeof(bufferServ));
                        switch (bufferServ.type)
                        {
                        case 1: // reception d'un message
                                if (strcmp(bufferServ.pseudo, destRegistred) != 0)
                                {
                                        strcpy(destRegistred, bufferServ.pseudo);
                                        printf("* Nouvelle conversation entrante avec %s : \n", destRegistred);
                                        isDest = 1;
                                }
                                printf("%s a écrit : ", bufferServ.pseudo);
                                printf("%s \n", bufferServ.message);
                                break;
                        case 4 :
                                printf("%s a écrit (global): ", bufferServ.pseudo);
                                printf("%s \n", bufferServ.message);
                                break;
                        default:
                                break;
                        }
                }
        }
        return (0);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Définition des fonctions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * Méthode qui demande le pseudo de connexion et traite le retour serveur
 **/
void askPseudo()
{
        char pseudoTemp[TAILLE_MAX_PSEUDO];
        int typeRet; // Type retourné lors de la connexion au serveur
        int longueur;
        do
        {
                printf("Quel est votre pseudo de connexion ? 15 caractères maximum \n");
                scanf("%s", pseudoTemp);
                longueur = strlen(pseudoTemp);
                if (longueur >= TAILLE_MAX_PSEUDO)
                {
                        printf("Pseudo trop long \n");
                }

        } while (longueur >= TAILLE_MAX_PSEUDO);
        printf("Vous avez entré : %s. \n", pseudoTemp);

        // Demande de connexion au serveur
        typeRet = connection(pseudoTemp);
        switch (typeRet)
        {
        case 5:
                disconnection(0);
                break;
        case 6:
                printf("Le pseudo est déjà utilisé. ");
                askPseudo();
                break;
        case 0:
                printf("Vous êtes connecté au chat ! \n");
                printf("--> Pour quitter, tapez q. \n");
                printf("--> Pour commencer une conversation avec un utilisateur, tapez '+send'. Recommencez si vous souhaitez changer d'interlocuteur. \n");
                printf("--> Pour envoyer un message global, tapez '+global'.\n");
                printf("--> Pour mettre à jour les utilisateurs connectés, tapez '+liste'.\n");
                strcpy(pseudo, pseudoTemp); // pseudo validé
                break;

        default:
                break;
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

/**
 *      Renvoie le type de message retourné par le serveur après la connexion 
 * 
 */
int connection(char pseudo[TAILLE_MAX_PSEUDO])
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

/**
 * Deconnecte le client du serveur et ferme le socket
 * notif :  si = 1, envoie un message de notification au serveur
 * */
void disconnection(int notif)
{
        if (notif == 1)
        {
                struct _msg bufferCli;
                //type = 2 signifie au serveur une volonté de se déconnecter
                bufferCli.type = 2;
                write(socket_descriptor, (char *)&bufferCli, sizeof(bufferCli));
        }
        printf("Vous quittez le chat féminin, au revoir et à bientôt !\n");
        close(socket_descriptor);
        printf("Connexion avec le serveur fermée, fin du programme.\n");
        exit(0);
};

/**
 * Fonction qui affiche la liste des connectés au chat
 **/
void readConnect()
{

        int i = 0;
        printf("===========================\n");
        printf("Utilisateurs connectés :\n");
        while (i < pseudoCos.nbCo)
        {
                if (strcmp(pseudo, pseudoCos.listePseudos[i]) == 0)
                {
                        printf(" * %s (vous) \n", pseudoCos.listePseudos[i]);
                }
                else
                {
                        printf(" * %s\n", pseudoCos.listePseudos[i]);
                }
                i++;
        }
        printf("===========================\n");
}
