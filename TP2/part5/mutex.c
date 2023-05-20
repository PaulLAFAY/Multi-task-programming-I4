// include all the librairies
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// include the dependency
#include "sematube.h"

int main(int argc, char **argv)
{
    // variables statements
    sematube sem;
    const int nbFork = 3;
    int i;
    pid_t pid;
    char debug;

    // handling the program's arguments
    if (argc > 1 && strcmp(argv[1], "--help") == 0)
    {
        printf("-d    Debug mode.\n");
        return EXIT_SUCCESS;
    }
    else if (argc > 1 && strcmp(argv[1], "-d") == 0)
        debug = 1;
    else
        debug = 0;

    // set up the sematube
    e(&sem, 1, debug);

    // create nbFork child processes
    for (i = 0; i < nbFork; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            // procedure to check that the sematube is working (spoiler : no)
            printf("child(%i#%i) forked.\n", i, getpid());
            P(&sem);
            printf("  child(%i#%i) is in critical section.\n", i, getpid());
            sleep(1);
            V(&sem);
            printf("  child(%i#%i) is no longer in critical section.\n", i, getpid());
            break;
        }
    }

    // the father process wait the death of his children
    if (pid != 0)
        sleep(5);

    return EXIT_SUCCESS;
}