#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

sigjmp_buf mark;

void traite_FPE(int sigId)
{
    printf("Detection d'une erreur : %i - %s\n", sigId, strsignal(sigId));
    siglongjmp(mark, 1);
}

int main()
{
    float res;
    int num = 2;
    int den = 0;

    signal(SIGFPE, traite_FPE);

    if (sigsetjmp(mark, 1) == 1)
        printf("Reprise sur erreur\n");

    sleep(1);
    printf("Attention a la division par zero\n");
    res = num / den;
    printf("Resultat : %f", res);

    return EXIT_SUCCESS;
}