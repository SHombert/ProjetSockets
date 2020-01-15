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
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

//message a envoyer au server
typedef struct _msg
{
    int type; 
    char pseudo[15]; //destinataire msg.
    char message[255];
} msg;

//liste des connectés
typedef struct _list
{
    char listePseudo[Max_LOGGED][15];
};
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Type de Procédure ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void Connection (char pseudo [15]);
void SendAMessage ();
void Disconnection ();
int verificationId(char pseudo [15]);
void readConnect ();
static void Listening ();
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Programme Principal ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main(int argc, char **argv) {

    /*Lexique Général*/
    int 	socket_descriptor, 	/* descripteur de socket */
		longueur; 		/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    servent *	ptr_service; 		/* info sur service */
    char 	buffer[256];
    char *	prog; 			/* nom du programme */
    char *	host; 			/* nom de la machine distante */
    char *	mesg; 			/* message envoyé */
    int answer;
    char pseudoTemp [15]; 
    int valid, longueur;
    
    /*Corps de l'algo*/ 

    //=>>
    printf("Bonjour ! Bienvenu sur le chat 100% féminin ! /n");
    do
    {
       
    
        do
        {
            printf("Quel est votre pseudo de connection ? 15 caractères maximum");
            scanf("%d, &pseudoTemp");
            longueur = strlen(pseudoTemp);
            if (longueur>16)
            {
            printf("pseudo trop long");
            }
            
        }while (longueur>16);
        printf ("Vous avez rentré : &d",&pseudoTemp);

        // initMPD;  ?
            valid = verificationId(pseudoTemp); //envoie du pseudo au serveur pour savoir s'il est déja utilisé
            if (valid==1)
            {
                printf("pseudo enregistré !");
            } else if (valid ==0)
            {
                printf("pseudo déjà utilisé !");
            }
     } while (valid = 1);

    Connection(pseudoTemp);   //envoie de la connetion au serveur
    Listening();

    readConnect (); //lecture et affichage de la liste, envoyé par le serveur,  des clients connectés 


    do{
    printf("\n");
    printf("  ===========================================/n");
    printf ("  ||           Menu             ||/n");
    printf ("  || Que voulez vous faire ?    ||/n");
    printf ("  || 1 : Envoyer un message     ||/n");
    printf ("  || 2 : Déconnection           ||/n");
    printf("  ===========================================");
    printf("\n");
    scanf("%d",&answer);
    } while (answer <3 & answer >0 );

    switch (answer) {
        case 1: SendAMessage ;
        case 2: Disconnection;
        break;
    }
    //<<=    
    exit(0);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Définition des fonctions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//Fonction d'écoute par le client
static void Listening ()  {
    //en boucle pour lecture messages entrant
    /* lecture de la reponse en provenance du serveur */
    while((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
	printf("reponse du serveur : \n");
	write(1,buffer,longueur);
    }
};



int verificationId(char pseudoTemp[15], struc msg)
{
    //lexique local
    int valid=0; //pseudo non utilisé si valid=0, utilisé si valid=1

    //envoie pseudo server et attente reponse du server pour savoir si pseudo existe déjà.
    msg->type = 0; 
    msg->pseudo = pseudoTemp; 
    write(sockPipe, (char*)&msg, sizeof(msg));

    //lecture retour serveur
    read(sockPipe, (char*)&msg, sizeof(msg));

    //traitement
    if (msg.type ==6 )
    {
        return valid;
    } else
    {
        return valid=1;
    }
};

void Connection (char pseudo [15])
{

    if ((ptr_host = gethostbyname(host)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
    
   /* Nouveau numéro de port : 5000 */ 
    adresse_locale.sin_port = htons(5000);

    
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le serveur.");
        exit(1);
    }
    
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de se connecter au serveur.");
        exit(1);
    }
    //type=0 signifie une connection au serveur
     msg->type=0;
     msg->pseudo=pseudo;
     msg->message=NULL;

     write(socket_descriptor, msg, strlen(msg));

    printf("connexion etablie avec le serveur. \n");

    return(0);
};

void SendAMessage ()
{ 
    //type=1 signifie un envoie de message au serveur
    msg->type=1;
    
    printf("A qui voulez vous envoyer votre message ?");
    scanf("%d, &msg.pseudo");
    printf("Quel est votre message ?");
    scanf("%d, &msg.message");

    /* envoi du message vers le serveur */
    if ((write(socket_descriptor, msg, strlen(msg))) < 0) {
	perror("erreur : impossible d'ecrire le message destine au serveur.");
	exit(1);
    }else
    {
        write(sockPipe, (char*)&msg, sizeof(msg));
        printf ("Message envoyé !");
    }
};

void Disconnection ()
{
    //type = 2 signifie au serveur une volonté de se déconnecter
    msg->type = 2;
    msg->message = NULL;
    msg->pseudo = NULL;
    write(sockPipe, (char*)&msg, sizeof(msg));

    printf("\nfin de la reception.\n");
    printf("Vous quittez le chat féminin , au revoir et à bientôt !\n");
        
    close(socket_descriptor);
    
    printf("connexion avec le serveur fermee, fin du programme.\n");
    
    exit(0);
};

//Fonction qui affiche la liste des connectés au chat
void readConnect ()
{
    /* Lexique */
    listCos *aux;

    /* Corps */ 
    read(sockPipe, (char*)&pseudosClients, sizeof(pseudosClients));
    printf("La liste des connectés :");
    do 
    {
        printf("\n%s\n",listCos.listePseudos[MAX_LOGGED][15]);
    } while (aux != NULL);
}



//faire fonction lecture des connectés puis affichage
