/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.


#include "err_exit.h"
#include "semaphore.h"

/**
 * "semOp() is a wrapper for the semop() system call that makes it easier to use."
 *
 * The semop() system call is used to perform operations on semaphores.
 *
 * struct sembuf {
 *     unsigned short sem_num;
 *     short sem_op;
 *     short sem_flg;
 * };
 * The sem_num field specifies the semaphore number within the semaphore set. The sem_op field specifies the operation to
 * be performed on the semaphore. The sem_flg field specifies flags that modify the operation
 *
 * @param semid The semaphore set identifier returned by semget().
 * @param sem_num The index of the semaphore in the semaphore set.
 * @param sem_op The operation to perform on the semaphore. A positive value increments the semaphore, a negative value
 * decrements it, and 0 is a special value that causes the calling process to block until the semaphore's value is 0.
 * @param flg example IPC_NOWAIT to perform operation without blocking (put 0 otherwise)
 */
void semOp(int semid, unsigned short sem_num, short sem_op, int flg){
    struct sembuf sop = {.sem_op = sem_op, .sem_num = sem_num, .sem_flg = flg};

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
void printSemaphoreValue(int semid, int n_sem){

    union semun arg;
    unsigned short semVal[n_sem];
    arg.array = semVal;

    if(semctl(semid, 0, GETALL, arg) == -1)
        ErrExit("semctl GETALL failed");

    printf("semaphore set state:\n");
    for (int i = 0; i < n_sem; i++) {
        printf("id: %d -->%d\n", i, semVal[i]);

    }
}

