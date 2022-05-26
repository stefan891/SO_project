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

void semOp(int semid, unsigned short sem_num, short sem_op, short flg);


int createSemaphore(key_t key, int n_sem,short flag);

void removeSemaphore(int semid);

void printSemaphoreValue(int semid, int n_sem);

void getSemaphoreId(int semid, char string[]);

void semSetAll(int semid, short unsigned int values[], char *err);

int semOpNoBlocc(int semid, unsigned short sem_num, short sem_op);

int semWaitNoBloc(int semid, int sem_num);

void semSetVal(int semid, int values, char *err);