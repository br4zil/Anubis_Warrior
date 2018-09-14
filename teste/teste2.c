#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

int thread1, thread2, thread3, thread4;
csem_t sem;

void* func0() {
    printf("cyield thread1\n");
    cyield();
    printf("\nTerminando thread1\n");
    return 0;
}

void* func1() {
    printReady();
    printf("\ncwait thread2\n");
    cwait(&sem);
    printf("cyield thread2");
    cyield();
    printf("\nTerminando thread2\n");
    return 0;
}

void* func2() {

    printf("\ncwait thread3\n");
    printf("Thread3 e bloqueada, pois thread2 esta em sessao critica\n");
    cwait(&sem);
    printf("\nTerminando thread3\n");
    return 0;
}

void* func3() {
    printBlocked();
    printf("\ncyield thread4\n");
    cyield();
    printf("\ncsignal, thread3 volta a apto\n");
    csignal(&sem);
    printReady();
    printf("\nTerminando thread4");
    return 0;
}



int main()
{
    printf("\nFUNCOES TESTADAS: CIDENTIFY, CCREATE, CYIELD, CSEM_INIT, CWAIT, CSIGNAL\n");
    printf("DIGITE ALGO PARA INICIAR\n");
    getchar();

    char * name = (char*) malloc(sizeof(char)*72);

    cidentify (name, 72);

    printf("%s\n", name);

    printf("#main: criando 4 threads\n");
    thread1 = ccreate (func0, (void *) NULL, 0);
    thread2 = ccreate (func1, (void *) NULL, 0);
    thread3 = ccreate (func2, (void *) NULL, 0);
    thread4 = ccreate (func3, (void *) NULL, 0);

    printf("Iniciando semaforo\n");
    csem_init(&sem, 1);

    printReady();

    printf("cyield main\n");
    cyield();

    printf("\nDe volta a main\n");
    printf("cyield main\n");
    cyield();

    printf("\n\nDe volta a main\n");
    printf("cyield main\n");
    cyield();

    printf("\nTerminando #main\n");

    return 0;
}
