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

int createSemaphore(key_t key, int n_sem);

int createSemaphore(key_t key, int n_sem);

void removeSemaphore(int semid);

void printSemaphoreValue(int semid, unsigned short semVal[]);