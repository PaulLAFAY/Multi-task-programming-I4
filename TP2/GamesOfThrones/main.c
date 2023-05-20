// inclusion des bibliothèques
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// définition et instanciation des variables globalent (ici ce sont des constantes)
const char *tube = "/tmp/myfifo";
const int nbSoldat = 10;

// définition de la structure transmise dans le tube
struct pidInfo
{
    pid_t pidSoldat;
    pid_t pidMarcheurBlanc;
};

// génère une valeur aléatoire comprise dans l'interval [min;max]
int generateRandom(int min, int max)
{
    float result;
    srand(time(NULL)); // initialisation de la graine de génération de nombres aléatoires
    if (min < 0)       // pour les besoins du programme, je ne veux pas généner de valeur négative
    {
        printf("Erreur : fonction generateRandom : valeur min negative.\n");
        exit(EXIT_FAILURE);
    }
    do
    {
        result = (rand() % 101) / (100 / max); // formule pour obtenir un nombre dans la range voulue
    } while (result < min || result > max);
    return (int)result;
}

// handler du SIGUSR2
void handler()
{
    exit(EXIT_SUCCESS);
}

// fonction qui gère le processus du soldat (fils du Roi)
void soldat(int index)
{
    int fd = open(tube, O_WRONLY); // ouverture du descripteur de fichier (= ouverture du tube)
    struct pidInfo info;

    printf("Soldat %i : J'attaque john ! (pid : %i)\n", index, getpid());

    signal(SIGUSR2, handler); // configuration du handler pour le SIGUSR2

    // initialisation des données à envoyer
    info.pidSoldat = getpid();
    if (index == nbSoldat)
        info.pidMarcheurBlanc = getppid();
    else
        info.pidMarcheurBlanc = 0;

    write(fd, &info, sizeof(struct pidInfo)); // écriture des données dans le tube
    close(fd);                                // fermeture du descripteur de fichier (= fermeture du tube)
    pause();                                  // le processus attend de recevoir un signal (ici SIGUSR2)
}

// fonction qui gère le processus du Roi (fils du Dieu)
void marcheurBlanc(int pidJohn)
{
    int nbSoldatGenere = 0;

    while (nbSoldatGenere < nbSoldat) // boucle tant que le processus Roi n'a pas généré assez de processus soldat
    {
        if (fork() == 0) // processus soldat
            soldat(nbSoldatGenere + 1);
        else // processus Roi
        {
            nbSoldatGenere++;
            wait(NULL); // attend que son fils meurt avant d'en générer un nouveau
        }
    }
    printf("Marcheur : tous les soldats ont ete envoyes et sont mort (nombre de soldat genere : %i)\n", nbSoldatGenere);

    printf("Marcheur : John Snow, tu vas mourir !\n");
    sleep(generateRandom(1, 5)); // attend une durée aléatoire entre 1 et 5 secondes
    kill(pidJohn, SIGKILL);      // envoie SIGKILL au pid du processus de John
    sleep(1);                    // attend 1 seconde par sécurité pour ne pas que ce processus se termine avant celui de John et que Dieu récupère le mauvais pid avec son wait()
    exit(EXIT_SUCCESS);          // termine le processus
}

// fonction qui gère le processus de John
void johnSnow()
{
    struct pidInfo info = {0, 0};
    while (1) // boucle à l'infinie
    {
        int fd = open(tube, O_RDONLY);           // ouverture du tube
        read(fd, &info, sizeof(struct pidInfo)); // lecture du tube
        if (info.pidSoldat != 0)                 // s'il y a une info dans le tube = un soldat arrive
        {
            printf("John : Je recois un soldat ! (pid soldat = %i | pid Marcheur = %i)\n", info.pidSoldat, info.pidMarcheurBlanc);
            kill(info.pidSoldat, SIGUSR2); // envoie de SIGUSR2 au pid du soldat
            info.pidSoldat = 0;
        }
        close(fd);                      // fermeture du tube
        if (info.pidMarcheurBlanc != 0) // si john reçoit le pid du Roi, il faut quitter la boucle
            break;
    }
    printf("John : Dernier soldat mort ! Pid du Marcheur Blanc : %i\n", info.pidMarcheurBlanc);

    printf("John : Marcheur Blanc, tu vas mourir !\n");
    sleep(generateRandom(1, 5));          // attend une durée aléatoire entre 1 et 5 secondes
    kill(info.pidMarcheurBlanc, SIGKILL); // envoie SIGKILL au pid du processus du Roi
    sleep(1);                             // attend 1 seconde par sécurité pour ne pas que ce processus se termine avant celui de John et que Dieu récupère le mauvais pid avec son wait()
    exit(EXIT_SUCCESS);                   // termine le processus
}

int main()
{
    pid_t pidJohn, pidMarcheur, retPid; // déclaration des variables
    mkfifo(tube, 0666);                 // création du FIFO (= du tube) au chemin voulu et avec les permissions voulues.

    printf("--- Debut de la bataille de Durlieu ---\n");

    pidJohn = fork();
    if (pidJohn == 0) // processus fils --> John Snow
        johnSnow();
    else
    {
        pidMarcheur = fork();
        if (pidMarcheur == 0) // processus fils --> Marcheur blanc
            marcheurBlanc(pidJohn);
        else // processus père --> Dieu
        {
            retPid = wait(NULL);   // attend la mort d'un fils et recupère son pid
            if (retPid == pidJohn) // compare les pid pour déclarer le vainqueur
                printf("Marcheur : J'ai gagne !\n");
            else if (retPid == pidMarcheur)
                printf("John : J'ai gagne !\n");
        }
    }

    printf("--- Fin de la bataille de Durlieu ---\n");

    return EXIT_SUCCESS;
}