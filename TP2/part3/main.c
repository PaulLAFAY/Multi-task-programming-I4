#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    int Tube[2];
    pipe(Tube);

    if (fork() == 0) {     //processus fils
        char c[40];
        while (strcmp(c, "exit") != 0) {
            printf("Reception d'un message : ");
            read(Tube[0], c, 40);
            printf("%s\n", c);
        }

    } else {               //processus parent
        char c[40];
        while (strcmp(c, "exit") != 0) {
            printf("Envoi d'un message : ");
            fflush(stdin);
            if (scanf("%s", c) == -1) {
                printf("EOF\n");
                write(Tube[1], "exit", 40);
                break;
            }
            write(Tube[1], c, 40);
            sleep(1);
        }

        sleep(2);
        printf("Mon fils est mort.\n");

    }

    return EXIT_SUCCESS;
}