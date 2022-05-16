/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.


#include "err_exit.h"
#include "semaphore.h"


void semOp(int semid, unsigned short sem_num, short sem_op){
    struct sembuf sop = {.sem_op = sem_op, .sem_num = sem_num, .sem_flg = 0};

    if(semop(semid, &sop, 1) == -1)
        ErrExit("semop failed");
}

/**
 * It creates a semaphore set with the given key and number of semaphores
 *
 * @param key the key to be used to identify the semaphore set.
 * @param n_sem number of semaphores in the set
 *
 * @return The semaphore ID
 */
int createSemaphore(key_t key, int n_sem){
    int semid = semget(key, n_sem,IPC_CREAT | S_IRUSR | S_IWUSR);

    if(semid == -1)
        ErrExit("creating semaphore failed");

    return semid;
}

/**
 * It removes the semaphore with the given semid
 *
 * @param semid the semaphore ID
 */
void removeSemaphore(int semid){
    if(semctl(semid, 0, IPC_RMID, NULL) == -1)
        ErrExit("semaphore remove failed");
}

/**
 * It prints the value of each semaphore in the semaphore set
 * it have sense if you use it when everything is blocked, otherwise
 * will be printed old values
 *
 * @param semid the semaphore set identifier
 * @param semVal the array of semaphore values
 */
void printSemaphoreValue(int semid, unsigned short semVal[]){

    union semun arg;
    arg.array = semVal;

    if(semctl(semid, 0, GETALL, arg) == -1)
        ErrExit("semctl GETALL failed");

    printf("semaphore set state:\n");
    for (int i = 0; i < sizeof semVal; i++) {
        printf("id: %d -->%d\n", i, semVal[i]);

    }
}

