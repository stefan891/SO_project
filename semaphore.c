/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.


#include "err_exit.h"
#include "semaphore.h"
#include <errno.h>
#include <string.h>


void semOp(int semid, unsigned short sem_num, short sem_op, short flg){
    struct sembuf sop = {.sem_op = sem_op, .sem_num = sem_num, .sem_flg = flg};

    if(semop(semid, &sop, 1) == -1)
        ErrExit("semop failed");
}


int createSemaphore(key_t key, int n_sem, short flag){
    int semid = semget(key, n_sem,flag | S_IRUSR | S_IWUSR);

    if(semid == -1)
        ErrExit("\ncreating semaphore failed");

    return semid;
}


void removeSemaphore(int semid){
    if(semctl(semid, 0, IPC_RMID, NULL) == -1)
        ErrExit("semaphore remove failed");
}


void printSemaphoreValue(int semid, int n_sem){

    union semun arg;
    unsigned short semVal[n_sem];
    arg.array = semVal;

    if(semctl(semid, 0, GETALL, arg) == -1)
        ErrExit("semctl GETALL failed");

    printf("\nsemaphore set state:\n");
    for (int i = 0; i < n_sem; i++) {
        printf("id: %d -->%d\n", i, semVal[i]);

    }
    fflush(stdout);
}


void getSemaphoreId(int semid, char string[]){

    printf("%s: %d\n", string, semid);
    fflush(stdout);
}



void semSetAll(int semid, short unsigned values[], char *err){
    union semun arg;
    arg.array = values;

    if(semctl(semid, 0/*ignored*/, SETALL, arg) == -1)
        ErrExit(strcat("semctl SETALL failed: ", err));
}


int semOpNoBlocc(int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = IPC_NOWAIT};
    if (semop(semid, &sop, 1) == -1){
        if (errno == EAGAIN){
            return -1;
        }
        else{
            ErrExit("semop failed");
        }
    }

    return 0;
}



int semWaitNoBloc(int semid, int sem_num){
    return semOpNoBlocc(semid, sem_num, -1);
}



void semSetVal(int semid, int values, char *err){
    union semun arg;
    arg.val = values;

    if(semctl(semid, 0/*ignored*/, SETVAL, arg) == -1)
        ErrExit(strcat("semctl SETALL failed: ", err));
}
