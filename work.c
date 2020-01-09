/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Type de Procédure ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void Connection ();
void SendAMessage ();
void Disconnection ();
int verificationId(char pseudo [15]);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Programme Principal ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// à mettre dans le main

/* Lexique général */
int answer;
char pseudoTemp [15]; 
int valid;

typedef struct _msg
{
    int type; 
    
    char pseudo[15]; //destinataire msg0.
    char message[255];
} msg;

typedef struct _list
{

};


/*envoie message :
write(sockPipe, (char*)&msg, sizeof(msg))
read(sockPipe, (char*)&msg, sizeof(msg));*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Menu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

printf("Bonjour ! /n");
do
{
    printf("Quel est votre pseudo de connection ? 15 caractères maximum");
    scanf("%d, &pseudo");
    printf ("Vous avez rentré : &d",&pseudo);//construction à vérifier

    initMPD; // ?
    valid = verificationId(pseudo);
    if (valid==1)
    {
        printf("pseudo enregistré !");
    } else if (valid ==0)
    {
        printf("pseudo déjà utilisé !");
    }
} while (valid = 1);

Connection();

do{
printf("  ===========================================/n");
printf ("  ||           Menu             ||/n";)
printf ("  || Que voulez vous faire ?    ||/n");
printf ("  || 1 : Envoyer un message     ||/n");
printf ("  || 2 : Déconnection           ||/n");
printf("  ===========================================")
scanf("%d",&answer);
} while (answer <3 & answer >0 )

switch (answer) {
	case 1: SendAMessage ;
    case 2: Disconnection;
    break;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Définition des fonctions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int verificationId(char pseudoTemp[15], struc msg)
{
    //lexique local
    int valid=0;
 //envoie pseudo server et attente reponse du server pour savoir si pseudo existe déjà.
    msg.type = 0;       //à vérifier !!
    msg.pseudo = pseudo;    //à vérifier !!
    write(sockPipe, (char*)&msg, sizeof(msg));

    //lecture retour serveur !!

    if (rep ==6 )
    {
        return valid;
    } else
    {
        valid = 1;
        return valid;
    }
};

void Connection () // passer pseudo en paramètre ??
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

    printf("connexion etablie avec le serveur. \n");

    return(0);
};

void SendAMessage ()
{ 
    msg.type=1;
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
        printf ("Message envoyé !")
    }
};

void Disconnection ()
{
    //envoie msg avec type=2 au serveur
    printf("\nfin de la reception.\n");
    
    close(socket_descriptor);
    
    printf("connexion avec le serveur fermee, fin du programme.\n");
    
    exit(0);
    
};

//faire fonction lecture des connectés puis affichage
