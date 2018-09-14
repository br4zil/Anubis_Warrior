#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

int thread1, thread2, thread3;

void* func0() {
	printf("\nEu sou a thread1\n");
	printf("#thread1 suspendendo #thread3\n");
	csuspend(thread3);
	printReady();
	printReadySus();
	printf("#thread1 resumindo #thread3\n");
	cresume(thread3);
	printReady();
	printReadySus();
        printf("Terminando #thread1\n");
	return 0;
}

void* func1() {
    printf("\nEu sou a thread2\n");
    printf("#thread2 cjoin thread3\n");
    cjoin(thread3);
    printf("\nTerminando #thread2\n");
    return 0;
}

void* func2() {
    printf("\nEu sou a thread3\n");
    printBlocked();
    printf("#thread3 suspendendo #thread2\n");
    csuspend(thread2);
    printBlocked();
    printBlockedSus();
    printf("Terminando thread3\n");
	return 0;
}



int main()
{
    printf("\nFUNCOES TESTADAS: CIDENTIFY, CCREATE, CJOIN, CSUSPEND, CRESUME\n");
    printf("DIGITE ALGO PARA INICIAR\n");
    getchar();

    char * name = (char*) malloc(sizeof(char)*72);

    cidentify (name, 72);

    printf("%s\n", name);

    printf("#main: criando 3 threads\n");
    thread1 = ccreate (func0, (void *) NULL, 0);
    thread2 = ccreate (func1, (void *) NULL, 0);
    thread3 = ccreate (func2, (void *) NULL, 0);

    printReady();

    printf("#main chamando cjoin thread1\n");
    cjoin(thread1);
    
    printf("\nDe volta a main\n");
    printReadySus();
    printf("#main resumindo thread2\n");
    cresume(thread2);
    printReady();

    printf("#main cjoin thread2\n");
    cjoin(thread2);

    printf("\nTerminando #main\n");

    return 0;
}

