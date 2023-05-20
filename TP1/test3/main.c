#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void fonc(int sigId) {
    printf("Signal recu : %i - %s\n", sigId, strsignal(sigId));
}

void fonc1(int sigId) {
    fonc(sigId);
    system("who");
}

void fonc2(int sigId) {
    fonc(sigId);
    system("df .");
}

int main(int argc, char *argv[]) {

    int N = 31;
    int i;

    printf("Pid du programme : %i\n", getpid());

    for (i = 1; i <= N; i++)
        if (i != 9 && i != 10 && i != 12 && i != 19)
            signal(i, fonc);
    
    signal(10, fonc1);
    signal(12, fonc2);

    while (1)
        sleep(1);

}