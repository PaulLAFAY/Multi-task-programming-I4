#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

struct thread_info
{
    pthread_t thread_id;
    int thread_num;
};

void *thread_start(void *arg)
{
    struct thread_info *tinfo = arg;
    printf("Thread %i : pid = %i\n", tinfo->thread_num, getpid());
    sleep(10);
    pthread_exit((void *)EXIT_SUCCESS);
}

int main()
{
    int i;
    int num_thread = 3;
    struct thread_info *tinfo;
    pthread_attr_t attr;
    void *retval;

    system("clear");

    printf("Taille d'un pointeur void : %i\n", sizeof(void *));
    printf("2^%i-1 valeurs soit %i valeurs differentes peuvent etre stockees dans le pointeur void\n", sizeof(void *), (int)pow(256, sizeof(void *)) - 1);

    printf("Pere : pid = %lu\n", getpid());

    tinfo = calloc(num_thread, sizeof(struct thread_info));

    system("ps -L H");

    pthread_attr_init(&attr);

    for (i = 0; i < num_thread; i++)
    {
        tinfo[i].thread_num = i + 1;
        pthread_create(&tinfo[i].thread_id, &attr, &thread_start, &tinfo[i]);
    }

    system("ps -L H");

    for (i = 0; i < num_thread; i++)
    {
        pthread_join(tinfo[i].thread_id, &retval);
        printf("Le thread retourne %i\n", (int *)retval);
    }

    return EXIT_SUCCESS;
}