//
// Created by francesco on 5/19/22.
//
#include "defines.h"
#include "semaphore.h"
#include "err_exit.h"

int main(int argc, char *argv[]) {

    printf("hello\n");

    //1 semaforo semid
    int semid= createSemaphore(12,1,IPC_CREAT);
    if(semid==-1)
        ErrExit("semget failed");
    unsigned short array[]={1};
    union semun arg;
    arg.array=array;

    if (semctl(semid, 0 /*ignored*/, SETALL, arg) == -1)
        ErrExit("semctl SETALL failed");
    printf("semaforo semid %d\n",semid);
    fflush(stdout);

    //2 semaforo n_files
    int n_files= createSemaphore(6,1,IPC_CREAT);
    if(n_files==-1)
        ErrExit("semget failed");
    array[0]=4;
    arg.array=array;

    if (semctl(n_files, 0 /*ignored*/, SETALL, arg) == -1)
        ErrExit("semctl SETALL failed");
    printf("semaforo n_files %d\n",n_files);
    fflush(stdout);

    char prova;
    scanf("%c",&prova);



    for(int i=0;i<4;i++)
    {
        pid_t pid=fork();
        if(pid==-1)
            ErrExit("fork failed");
        //codice figlio---------------------------------------------
        else if(pid==0)
        {
            semOp(semid,0,-1,0);

            semOp(n_files,0,-1,0);

            printf("child %d\n",i);
            printf("ciao\n");
            fflush(stdout);
            sleep(1);

            semOp(semid,0,1,0);

            printf("mi fermo\n");
            fflush(stdout);
            semOp(n_files,0,0,IPC_NOWAIT);
            printf("LIBERO\n");
            exit(0);

        }
        //----------------------------------------------------------
    }

    //parent wait for all childs

    while(wait(NULL)!=-1);
    printf("\nparent finished");
    removeSemaphore(semid);
    removeSemaphore(n_files);
    return(0);
}
