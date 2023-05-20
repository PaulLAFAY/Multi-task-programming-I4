#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

int generateRandom(int max)
{
    float result;
    result = rand()%(max+1);
    return (int)result;
}

sem_t sem_client;
sem_t sem_homme;
sem_t sem_femme;
sem_t sem_attente_coiffeur;

const int nbSiege = 5;
int nbH = 0;
int nbF = 0;

void *thread_client(void *arg)
{
    int sexe = *((int *)arg);
    if (sexe == 0)
        printf("Un client arrive.\n");
    else
        printf("Une cliente arrive.\n");
    sem_wait(&sem_client);
    if ((nbH + nbF) >= nbSiege)
    {
        sem_post(&sem_client);
        printf("Le/la client.e repart car il n'y a pas de place.\n");
        pthread_exit(EXIT_FAILURE);
    }
    else if (sexe == 0) // client homme
    {
        nbH++;
        sem_post(&sem_client);
        sem_post(&sem_attente_coiffeur);
        sem_wait(&sem_homme);
        sem_wait(&sem_client);
        nbH--;
        sem_post(&sem_client);
    }
    else // cliente femme
    {
        nbF++;
        sem_post(&sem_client);
        sem_post(&sem_attente_coiffeur);
        sem_wait(&sem_femme);
        sem_wait(&sem_client);
        nbF--;
        sem_post(&sem_client);
    }
}

void *thread_coiffeurH()
{
    printf("Coiffeur cree.\n");
    while (1)
    {
        sem_wait(&sem_attente_coiffeur);
        if (nbH > 0)
        {
            sem_post(&sem_homme);
            printf("Le coiffeur coiffe un homme.\n");
        }
        else
        {
            sem_post(&sem_femme);
            printf("Le coiffeur coiffe une femme.\n");
        }
        sleep(generateRandom(15));
    }
}

void *thread_coiffeurF()
{
    printf("Coiffeuse creee.\n");
    while (1)
    {
        sem_wait(&sem_attente_coiffeur);
        if (nbF > 0)
        {
            sem_post(&sem_femme);
            printf("La coiffeuse coiffe une femme.\n");
        }
        else
        {
            sem_post(&sem_homme);
            printf("La coiffeuse coiffe un homme.\n");
        }
        sleep(generateRandom(15));
    }
}

int main()
{
    pthread_t thread;

    sem_init(&sem_client, 0, 1);
    sem_init(&sem_homme, 0, 0);
    sem_init(&sem_femme, 0, 0);
    sem_init(&sem_attente_coiffeur, 0, 0);

    srand(time(NULL));

    pthread_create(&thread, NULL, &thread_coiffeurH, NULL);
    pthread_create(&thread, NULL, &thread_coiffeurF, NULL);

    while (1)
    {
        int sexe = generateRandom(1);
        pthread_create(&thread, NULL, &thread_client, &sexe);
        printf("nbH : %i | nbF : %i | nbClient : %i\n", nbH, nbF, nbH+nbF);
        sleep(generateRandom(3));
    }

    return EXIT_SUCCESS;
}