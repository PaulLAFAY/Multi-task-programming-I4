#include <stdio.h>
#include <signal.h>

int main(){

	int i;
	for(i = 1; i <= 31; i++){
		signal(i, SIG_IGN);
		if(i == 18 && signal(18, SIG_IGN) == SIG_ERR) printf("18 pas ignorable\n");
		else printf("18 ignorable\n");
	}

	//while(1) printf("le processus est reveille\n");

}
