// librairies
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <setjmp.h>
#include <string.h>

// préprocesseur
#define PTS_VIE 6

// structure générale d'un personnage
struct Personnage // Config par défaut :
{                 // Luc         Vador
    char nom[20]; // SkyWalker   Dark Vador
    int PV;       // PTS_VIE     PTS_VIE
    int attSpeed; // 2           3
    int attDeg;   // 1           3
};

// variables globales
const struct Personnage sw = {"Skywalker", PTS_VIE, 2, 1};
const struct Personnage dv = {"Dark Vador", PTS_VIE, 3, 3};
struct Personnage perso;
sigjmp_buf env1;
sigjmp_buf env2;
sigjmp_buf env3;

// fonctions en lien avec la gestion des signaux
void handlerGod() // handler du signal d'interruption (quand dieu veut arréter le combat)
{
    int i;
    for (i = 1; i <= 31; i++) // ignore tous les signaux
        signal(i, SIG_IGN);
    if (strcmp(perso.nom, dv.nom) == 0) // n'affiche cette phrase que dans le processus de Vador pour ne pas dédoubler le printf
        printf("\nDieu (la force) : Cessez cette bagarre ridicule !\n[Fin du combat]\n");
    exit(EXIT_SUCCESS);
}

void handlerUsr1() // handler du signal sigusr1 (attaques au sabre laser)
{
    signal(SIGUSR1, SIG_IGN);
    siglongjmp(env1, 1);
}

void handlerUsr2() // handler du signal sigusr2 (mort de l'autre joueur)
{
    if (strcmp(perso.nom, dv.nom) == 0) // renvoi sur le bout de code qui convient
        siglongjmp(env2, 1);
    else
        siglongjmp(env3, 1);
}

void sigConfig() // configuration des handler des signaux
{
    int i;
    for (i = 1; i <= 31; i++)
        if (i != 2 && i != 9 && i != 12 && i != 17 && i != 19)
            signal(i, SIG_IGN);
    signal(SIGINT, handlerGod);
    signal(SIGUSR2, handlerUsr2);
}

// fonctions en lien avec la gestion des personnages
void lanceAtt(int pid, struct Personnage perso) // procedure de lancement d'une attaque
{
    printf("%s : Je lance une attaque !\n", perso.nom);
    signal(SIGUSR1, handlerUsr1); // baisse sa garde
    sleep(perso.attSpeed);        // temps de charge de l'attaque et attente d'un potentiel coup de l'adversaire
    kill(pid, SIGUSR1);           // coup de sabre laser
    signal(SIGUSR1, SIG_IGN);     // se remet en garde
}

void subitAttaques(struct Personnage *perso, struct Personnage sw, struct Personnage dv) // quand le joueur subit une attaque
{
    if (strcmp(perso->nom, sw.nom) == 0) // décompte des points de vie
        perso->PV -= dv.attDeg;
    else if (strcmp(perso->nom, dv.nom) == 0)
        perso->PV -= sw.attDeg;
}

int generateRandom(int min, int max) // génère une valeur aléatoire comprise dans l'interval [min;max]
{
    float result;
    if (min < 0) // pour les besoins du programme, je ne veux pas généner de valeur négative
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

// fonction main
int main()
{
    // déclaration des variables
    int pidAdverse;
    int i;

    // gestion des signaux
    sigConfig();

    // message du début (avant le fork() pour n'apparaitre qu'une seule fois)
    printf("[Debut du combat]\n");

    // obtention des pid et selection des personnages
    pidAdverse = fork();
    if (pidAdverse == 0)
    { // initialise 'perso' avec les valeurs de Luc
        for (i = 0; i < 20; i++)
            perso.nom[i] = sw.nom[i];
        perso.PV = sw.PV;
        perso.attSpeed = sw.attSpeed;
        perso.attDeg = sw.attDeg;
        pidAdverse = getppid();
    }
    else
    { // initialise 'perso' avec les valeurs de Vador
        for (i = 0; i < 20; i++)
            perso.nom[i] = dv.nom[i];
        perso.PV = dv.PV;
        perso.attSpeed = dv.attSpeed;
        perso.attDeg = dv.attDeg;
    }

    // initialisation de la génération de nombres aléatoires
    srand(time(NULL));
    // les nombres aléatoires ne sont pas vraiment aléatoires : il forment une série. Pour éviter que Vador et Luc aient le même temps d'attente entre 2 attaques, je décale la série du coté de Luc en générant un nombre aléatoire de valeur aléatoire
    if (strcmp(perso.nom, sw.nom) == 0)
    {
        for (i = 0; i < generateRandom(5, 100); i++)
            generateRandom(1, 5);
    }

    // déroulement du jeu
    while (1)
    {
        if (sigsetjmp(env1, 1) == 1) // si le joueur subit une attaque
        {
            printf("%s : [Attaque echouee]\n%s : Je subis une attaque !\n", perso.nom, perso.nom);
            printf("%s : Decompte des degats...\n", perso.nom);
            subitAttaques(&perso, sw, dv);
            printf("%s : Mes PV : %i\n", perso.nom, perso.PV);
            if (perso.PV <= 0) // si le joueur doit mourir
            {
                printf("%s : Je suis mort.\n", perso.nom);
                kill(pidAdverse, SIGUSR2); // dit à l'autre joueur que celui-ci est mort
                break;
            }
        }

        if (sigsetjmp(env2, 1) == 1) // si c'est Luc qui meurt
        {
            printf("%s : Mon fils est mort...\n%s : Adieu !\n", perso.nom, perso.nom);
            printf("[Fin du combat]\n");
            break;
        }

        if (sigsetjmp(env3, 1) == 1) // si c'est Vador qui meurt
        {
            printf("%s : Mon pere est mort...\n%s : Adieu !\n", perso.nom, perso.nom);
            printf("[Fin du combat]\n");
            break;
        }

        int cooldown = generateRandom(1, 5);
        printf("%s : Prochaine attaque dans %is\n", perso.nom, cooldown);
        sleep(cooldown);

        lanceAtt(pidAdverse, perso);
    }

    return EXIT_SUCCESS; // fin du programme
}