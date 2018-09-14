#include <stdio.h>
#include <stdlib.h>

#include "../include/cthread.h"
#include "../include/cdata.h"


PFILA2 readyQueue;
PFILA2 blockedQueue;
PFILA2 finishedQueue;
PFILA2 readySuspendedQueue;
PFILA2 blockedSuspendedQueue;

TCB_t * executingThread;

ucontext_t DispatcherContext;
int tCounter = 0;
int has_init = 0;
int hasThreadEnded = 0;


PNODE2 returnNode(int tid, PFILA2 queue)
{
    TCB_t *thread;
    PNODE2 current;

    if (FirstFila2(queue) != 0)
    {

        printf("Erro ao setar para o primeiro da fila\n");
        return NULL;
    }

    do
    {
        current = (PNODE2)GetAtIteratorFila2(queue);
        thread = (TCB_t *) current->node;
        if(thread->tid == tid)
        {
            return current;
        }
    }
    while(NextFila2(queue) == 0);
    return NULL;
}

// -----------------------------------------------------------------------------

int searchThread(int tid, PFILA2 queue)
{
    TCB_t *thread;
    PNODE2 current;

    if (FirstFila2(queue) != 0)
    {
        //printf("Fila vazia ou erro ao setar para o primeiro da fila\n");
        return FALSE;
    }

    do
    {
        current = (PNODE2)GetAtIteratorFila2(queue);
        thread = (TCB_t *) current->node;
        if(thread->tid == tid)
        {
            return TRUE;
        }
    }
    while(NextFila2(queue) == 0);
    return FALSE;
}

// -----------------------------------------------------------------------------

TCB_t* returnTCB(int tid, PFILA2 queue)
{
    TCB_t *thread;
    PNODE2 current;

    if (FirstFila2(queue) != 0)
    {
        //printf("Fila vazia ou erro ao setar para o primeiro da fila\n");
        return NULL;
    }

    do
    {
        current = (PNODE2)GetAtIteratorFila2(queue);
        thread = (TCB_t *) current->node;
        if(thread->tid == tid)
        {
            return thread;
        }
    }
    while(NextFila2(queue) == 0);
    return NULL;
}

// -----------------------------------------------------------------------------

int changeQueue(int tid, PFILA2 queueSource, PFILA2 queueDest, int state)
{

    TCB_t * change = NULL;

    change = returnTCB(tid, queueSource);
    PNODE2 changeNode = malloc(sizeof(PNODE2));
    changeNode->node = change;
    change->state = state;

    if (AppendFila2(queueDest, changeNode) != 0)
    {
        printf("Erro ao trocar de fila\n");
        return RETURN_ERROR;
    }

    if(DeleteAtIteratorFila2(queueSource) != 0)
    {
        printf("Erro ao deletar da fila de origem\n");
        return RETURN_ERROR;
    }

    return RETURN_OK;
}


// -----------------------------------------------------------------------------

TCB_t* findThread(int tid)
{
    TCB_t* tcb;

    if(searchThread(tid, readyQueue))
    {
        tcb = returnTCB(tid, readyQueue);
    }

    else if(searchThread(tid, blockedQueue))
    {
        tcb = returnTCB(tid, blockedQueue);
    }

    else if(searchThread(tid, readySuspendedQueue))
    {
        tcb = returnTCB(tid, readySuspendedQueue);
    }

    else if(searchThread(tid, blockedSuspendedQueue))
    {
        tcb = returnTCB(tid, blockedSuspendedQueue);
    }

    else
    {
        //printf("Thread nao existe\n");
        return NULL;
    }

    return tcb;
}
// -----------------------------------------------------------------------------

int dispatcher()
{
    //printf("\nMae to na globo");
    PNODE2 nextNode;
    TCB_t * thread;

    if (hasThreadEnded)
    {

        PNODE2 newReadyNode = NULL;
        TCB_t * newThread;
        executingThread->state = PROCST_TERMINO;
        int tid = executingThread->waitedBy;

        if(searchThread(tid, blockedQueue))
        {

            if (LastFila2(blockedQueue) == 0)
            {
                newReadyNode = returnNode(tid, blockedQueue);
            }


            if (newReadyNode != NULL)
            {
                newThread = (TCB_t *) newReadyNode->node;
                newThread->state = PROCST_APTO;
                DeleteAtIteratorFila2(blockedQueue);
                AppendFila2(readyQueue, newReadyNode);

            }
        }

        else if(searchThread(tid, blockedSuspendedQueue))
        {


            if (LastFila2(blockedSuspendedQueue) == 0)
            {
                newReadyNode = returnNode(tid, blockedSuspendedQueue);
            }


            if (newReadyNode != NULL)
            {
                newThread = (TCB_t *) newReadyNode->node;
                newThread->state = PROCST_APTO_SUS;
                DeleteAtIteratorFila2(blockedSuspendedQueue);
                AppendFila2(readySuspendedQueue, newReadyNode);

            }

        }

    }

    if (FirstFila2(readyQueue) != 0)
    {
        printf("opa\n");
        exit (RETURN_ERROR);
    }
    nextNode = (PNODE2)GetAtIteratorFila2(readyQueue);
    hasThreadEnded = 1;

    if (nextNode != NULL)
    {
        thread = (TCB_t*) nextNode->node;
        executingThread = thread;
        thread->state = PROCST_EXEC;
        DeleteAtIteratorFila2(readyQueue);
        setcontext(&(thread->context));
        return RETURN_OK;
    }
    printf("ninguém tá pronto");

    return 0;
} // end method

// -----------------------------------------------------------------------------

void createDispatcherContext()
{

    getcontext(&DispatcherContext);
    char stack_dispatcher[SIGSTKSZ];
    DispatcherContext.uc_stack.ss_sp = stack_dispatcher;
    DispatcherContext.uc_stack.ss_size = sizeof(stack_dispatcher);
    makecontext( &DispatcherContext, (void (*)(void))dispatcher, 0);
} // end method

// -----------------------------------------------------------------------------

TCB_t* createThread(int prio)
{

    TCB_t * newThread = malloc(sizeof(TCB_t));

    if(newThread != NULL)
    {
        newThread->tid = tCounter;
        newThread->state = PROCST_CRIACAO;
        newThread->prio = prio;
        newThread->waitedBy = -1;
        newThread->waitingFor = -1;
        tCounter++;

        newThread->context.uc_stack.ss_sp = malloc(SIGSTKSZ);
        newThread->context.uc_stack.ss_size = SIGSTKSZ;
    }
    return newThread;
}

// -----------------------------------------------------------------------------

int createMainThread(ucontext_t MainContext)
{
    int prio = 0;
    TCB_t * mainThread = createThread(prio);

    if(mainThread != NULL)
    {
        mainThread->context = MainContext;
        mainThread->context.uc_link = NULL;
        mainThread->state = PROCST_EXEC;
        executingThread = mainThread;
        return mainThread->tid;
    }
    return RETURN_ERROR;
}

// -----------------------------------------------------------------------------

int init()
{

    int returnCode;

    readyQueue = malloc(sizeof(FILA2));
    blockedQueue = malloc(sizeof(FILA2));
    finishedQueue = malloc(sizeof(FILA2));
    readySuspendedQueue = malloc(sizeof(FILA2));
    blockedSuspendedQueue = malloc(sizeof(FILA2));

    CreateFila2(readyQueue);
    CreateFila2(blockedQueue);
    CreateFila2(finishedQueue);
    CreateFila2(readySuspendedQueue);
    CreateFila2(blockedSuspendedQueue);

    executingThread = NULL;

    createDispatcherContext();

    ucontext_t MainContext;
    returnCode = getcontext(&MainContext);

    if(returnCode == RETURN_OK)
    {
        returnCode = createMainThread(MainContext);
        has_init = 1;
        return(returnCode);
    }
    return RETURN_ERROR;

} // end method

// -----------------------------------------------------------------------------

int cidentify (char *name, int size)
{
    char id_grupo[72] = "Amanda Goveia 242259\nIsadora Oliveira 264109\nVictoria Elizabetha 261575\n";
    int i = 0;

    if (size >= 72)
    {
        while (i <= 72){

            name[i] = id_grupo[i];
            i++;
        }

        return RETURN_OK;
    }



    return RETURN_ERROR;
} // end method

// -----------------------------------------------------------------------------

int ccreate (void *(*start)(void *), void *arg, int prio)
{
    if(!has_init)
    {
        init();
    }

    TCB_t *newThread = createThread(prio);

    getcontext(&newThread->context);

    newThread->context.uc_link = &DispatcherContext;

    makecontext(&newThread->context, (void (*)(void)) start, 1, arg);
    newThread->state = PROCST_APTO;

    PNODE2 newThreadNode = malloc(sizeof(PNODE2));
    newThreadNode->node = newThread;

    if(AppendFila2(readyQueue, newThreadNode) != 0)
    {
        printf("Erro ao criar thread %d\n", newThread->tid);
        return RETURN_ERROR;
    }

    return newThread->tid;

} // end method


// -----------------------------------------------------------------------------


int cjoin(int tid)
{
    //printf("join the club\n");

    if(!has_init)
    {
        init();
    }

    TCB_t* waitFor = NULL;

    TCB_t * waiting;
    waiting = executingThread;

    waitFor = findThread(tid);

    if(waitFor != NULL && waitFor->waitedBy == -1)
    {

        PNODE2 blockedNode = malloc(sizeof(PNODE2));
        blockedNode->node = waiting;

        AppendFila2(blockedQueue, blockedNode);
        executingThread->state = PROCST_BLOQ;
        waitFor->waitedBy = waiting->tid;
        waiting->waitingFor = waitFor->tid;
        hasThreadEnded = 0;

        swapcontext(&(executingThread->context), &(DispatcherContext));
        return RETURN_OK;
    }

    else
    {
        return RETURN_ERROR;
    }

}

// -----------------------------------------------------------------------------

int cyield(void)
{
    if (!has_init)
        init();

    PNODE2 exe = malloc(sizeof(PNODE2));
    exe->node = executingThread;
    executingThread->state = PROCST_APTO;
    if (AppendFila2(readyQueue, exe) != 0)
        return RETURN_ERROR;
    hasThreadEnded = 0;

    if (swapcontext(&(executingThread->context), &(DispatcherContext)) == -1)
        return RETURN_ERROR;

    return RETURN_OK;

} // end method

// -----------------------------------------------------------------------------

int csuspend(int tid)
{

    if(!has_init)
    {
        init();
    }

    if(searchThread(tid, readyQueue))
    {
        if(changeQueue(tid, readyQueue, readySuspendedQueue, PROCST_APTO_SUS) != 0)
        {
            printf("csuspend: Erro ao passar para apto suspenso\n");
            return RETURN_ERROR;
        }

    }

    else if(searchThread(tid, blockedQueue))
    {
        if(changeQueue(tid, blockedQueue, blockedSuspendedQueue, PROCST_BLOQ_SUS) != 0)
        {
            printf("csuspend: Erro ao passar para bloqueado suspenso\n");
            return RETURN_ERROR;
        }
    }

    else
    {

        printf("csuspend: thread nao existe em apto nem em bloqueado\n");
        return RETURN_ERROR;
    }

    return RETURN_OK;
}


// -----------------------------------------------------------------------------

int cresume(int tid)
{

    if(!has_init)
    {
        init();
    }

    if(searchThread(tid, readySuspendedQueue))
    {
        if(changeQueue(tid, readySuspendedQueue, readyQueue, PROCST_APTO) != 0)
        {
            // printf("cresume: Erro ao passar para apto\n");
            return RETURN_ERROR;
        }

    }

    else if(searchThread(tid, blockedSuspendedQueue))
    {
        if(changeQueue(tid, blockedSuspendedQueue, blockedQueue,PROCST_BLOQ) != 0)
        {
            // printf("cresume: Erro ao passar para bloqueado\n");
            return RETURN_ERROR;
        }
    }

    else
    {

        printf("cresume: thread nao existe em apto suspenso nem em bloqueado suspenso\n");
        return RETURN_ERROR;
    }

    return RETURN_OK;
}
//-------------------------------------------------------------------------------------

int csem_init (csem_t *sem, int count)
{

    if(!has_init)
    {
        init();
    }

    sem->fila = malloc(sizeof(FILA2));

    if(CreateFila2(sem->fila) == 0)
    {
        sem->count = count;
        return RETURN_OK;
    }

    return RETURN_ERROR;
}

//-------------------------------------------------------------------------------------
int cwait (csem_t *sem)
{
    if(!has_init)
    {
        init();
    }

    if(sem->fila == NULL){

        printf("Sem fila do semaforo\n");
        return RETURN_ERROR;
    }

    sem->count--;

    if(sem->count >= 0)
    {
        return RETURN_OK;
    }

    TCB_t* thread = executingThread;
    thread->state = PROCST_BLOQ;

    PNODE2 node = malloc(sizeof(PNODE2));
    node->node = thread;

    if(AppendFila2(sem->fila, (void *)node) == 0)
    {

        if(AppendFila2(blockedQueue, (void *)node) == 0)
        {
            hasThreadEnded = 0;
            swapcontext(&(thread->context), &(DispatcherContext));
            return RETURN_OK;
        }
    }

    printf("Deu ruim na cwait\n");
    return RETURN_ERROR;
}
//-------------------------------------------------------------------------------------


int csignal(csem_t *sem)
{
    TCB_t* thread;

    if(!has_init)
    {
        init();
    }

    if(sem->fila == NULL){

        printf("Sem fila do semaforo\n");
        return RETURN_ERROR;
    }

    sem->count++;

    if(sem->count <= 0){ //ainda tem gente na fila

        if(FirstFila2(sem->fila) == 0){

            PNODE2 node;
            node = GetAtIteratorFila2(sem->fila);
            thread = (TCB_t *) node->node;

            if(searchThread(thread->tid, blockedQueue)){

                changeQueue(thread->tid, blockedQueue, readyQueue, PROCST_APTO);
                DeleteAtIteratorFila2(sem->fila);
            }

            else if(searchThread(thread->tid, blockedSuspendedQueue)){

                changeQueue(thread->tid, blockedSuspendedQueue, readySuspendedQueue, PROCST_APTO_SUS);
                DeleteAtIteratorFila2(sem->fila);
            }

            else{

                printf("csignal: nao achou a thread em bloqueado nem em bloqueado suspenso\n");
                return RETURN_ERROR;
            }
        }

        else
            return RETURN_ERROR;
    }

    return RETURN_OK;
}


//-------------------------------------------------------------------------------------

//Funções de teste

void printFila(PFILA2 queue)
{

    TCB_t *thread;
    PNODE2 current;

    if (FirstFila2(queue) != 0)
    {
        printf("fila vazia\n");
    }

    else{

        do
        {
            current = (PNODE2)GetAtIteratorFila2(queue);
            thread = (TCB_t *) current->node;
            printf("%d ", thread->tid);
        }
        while(NextFila2(queue) == 0);
        printf("\n");
    }
}



void printReadySus()
{
    printf("Fila de aptos suspensos: ");
    printFila(readySuspendedQueue);
}


void printReady()
{
    printf("Fila de aptos: ");
    printFila(readyQueue);
}

void printBlocked()
{
    printf("Fila de bloqueados: ");
    printFila(blockedQueue);
}

void printBlockedSus()
{
    printf("Fila de bloqueados suspensos: ");
    printFila(blockedSuspendedQueue);
}



/*int removeFromQueue(int tid, PFILA2 queue)
{
    TCB_t *thread;
    PNODE2 current;

    if(FirstFila2(queue) == 0)
    {

        do
        {
            current = (PNODE2)GetAtIteratorFila2(queue);
            thread = (TCB_t *) current->node;
            if(thread->tid == tid)
            {
                printf("Achou para deletar\n");
                DeleteAtIteratorFila2(queue);
                return RETURN_OK;
            }
        }
        while(NextFila2(queue) == 0);
    }


    return RETURN_ERROR;
}*/
