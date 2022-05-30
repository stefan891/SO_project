/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#pragma once
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

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
void semOp(int semid, unsigned short sem_num, short sem_op, short flg);

/**
 * It creates a semaphore set with the given key and number of semaphores
 *
 * @param key the key to be used to identify the semaphore set.
 * @param n_sem number of semaphores in the set
 *
 * @return The semaphore ID
 */
int createSemaphore(key_t key, int n_sem,short flag);

/**
 * It removes the semaphore with the given semid
 *
 * @param semid the semaphore ID
 */
void removeSemaphore(int semid);

/**
 * It prints the value of each semaphore in the semaphore set
 * it have sense if you use it when everything is blocked, otherwise
 * will be printed old values
 *
 * @param semid the semaphore set identifier
 * @param semVal the array of semaphore values
 */
void printSemaphoreValue(int semid, int n_sem);

/**
 * It prints the semaphore id to the screen
 *
 * @param semid The semaphore ID.
 * @param string The string to print out before the semaphore ID.
 */
void getSemaphoreId(int semid, char string[]);

/**
 * This function sets the values of all the semaphores in the set to the values in the array pointed to
 * by the second argument.
 * 
 * @param semid the semaphore set identifier
 * @param values an array of short unsigned integers, one for each semaphore in the set.
 * @param err a string to be used in the error message if the semctl call fails.
 */
void semSetAll(int semid, short unsigned int values[], char *err);

/**
 * It performs a semaphore operation on the semaphore with the given identifier, sem_num, and sem_op, but it doesn't block
 * if the operation would block
 *
 * @param semid the semaphore set identifier
 * @param sem_num the index of the semaphore in the semaphore set
 * @param sem_op the value to add to the semaphore. If negative, the semaphore is decremented. If positive, the semaphore
 * is incremented.
 *
 * @return The return value is the value of the semaphore after the operation.
 */
int semOpNoBlocc(int semid, unsigned short sem_num, short sem_op);

/**
 * It waits for a semaphore to be available, but it doesn't block if it's not
 *
 * @param semid the semaphore set identifier
 * @param sem_num The semaphore number.
 *
 * @return The return value is the value of the semaphore.
 */
int semWaitNoBloc(int semid, int sem_num);

/**
 * It sets the value of the semaphore to the value of the second argument
 * 
 * @param semid the semaphore set identifier
 * @param values the value to set the semaphore to
 * @param err the error message to print if the semaphore fails to be created
 */
void semSetVal(int semid, int values, char *err);