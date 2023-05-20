#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

int a = 1;
int b = 1;
sem_t sem;

int generateRandom(int min, int max)
{
    float result;
    if (min < 0)
    {
        printf("Erreur : fonction generateRandom : valeur min negative.\n");
        exit(EXIT_FAILURE);
    }
    do
    {
        result = (rand() % 101) / (100 / max);
    } while (result < min || result > max);
    return (int)result;
}

void *thread_routine(void *arg)
{
    int thread_num = *((long *)arg);

    sem_wait(&sem);

    sleep(generateRandom(0, 1));
    switch (thread_num)
    {
    case 0:
        a++;
        break;
    case 1:
        a *= 2;
        break;
    case 2:
        a--;
        break;

    default:
        break;
    }
    sleep(generateRandom(0, 1));
    switch (thread_num)
    {
    case 0:
        b++;
        break;
    case 1:
        b *= 2;
        break;
    case 2:
        b--;
        break;

    default:
        break;
    }
    sleep(generateRandom(0, 1));
    printf("Thread %i : a = %i | b = %i\n", thread_num, a, b);

    sem_post(&sem);

    sleep(1);

    pthread_exit(EXIT_SUCCESS);
}

int main()
{
    struct thread_info
    {
        pthread_t thread_id;
        int thread_num;
        void *retval;
    };

    int i, num_thread = 3;
    struct thread_info *tinfo;

    printf("Pere : pid = %i\n", getpid());

    tinfo = calloc(num_thread, sizeof(pthread_t));

    srand(time(NULL));

    sem_init(&sem, 0, 1);

    for (i = 0; i < num_thread; i++)
    {
        tinfo[i].thread_num = i;
        pthread_create(&tinfo[i].thread_id, NULL, &thread_routine, &tinfo[i].thread_num);
    }

    for (i = 0; i < num_thread; i++)
    {
        pthread_join(tinfo[i].thread_id, &tinfo[i].retval);
        printf("Pere : Le thread %i retourne %i\n", i, (int *)tinfo[i].retval);
    }
}