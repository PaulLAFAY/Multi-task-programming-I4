#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Traite_Sig(int sigId){

	printf("Fonction Traite_Sig : j'ai reçu le signal %i - %s\n", sigId, strsignal(sigId));

}

int main(int argc, char *argv[]){

	int N = 31;
	int i;

	for (i = 1; i <= N; i++)
		if (signal(i, Traite_Sig) == SIG_ERR)
			printf("Le signal %i - %s ne peut pas etre traite specifiquement.\n", i, strsignal(i));

	while (1)
		sleep(1);

	return EXIT_SUCCESS;

}
