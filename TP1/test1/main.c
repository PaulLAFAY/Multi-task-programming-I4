#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){

	int N = 31;
	int i;

	for(i = 1; i <= N; i++)
		printf("%i - %s\n", i, strsignal(i));

	for(i = 1; i <= N; i++)
		if(signal(i, SIG_IGN) == SIG_ERR)
			printf("Le signal %i - %s ne peut pas etre ignore.\n", i, strsignal(i));

	while(1)
		sleep(1);

	return EXIT_SUCCESS;

}
