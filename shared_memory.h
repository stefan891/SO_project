/// @file shared_memory.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#pragma once

#define _SHARED_MEMORY_HH

#include <stdlib.h>

int alloc_shared_memory(key_t shmKey, size_t size);

void *get_shared_memory(int shmid, int shmflg);

void free_shared_memory(void *ptr_sh);

void remove_shared_memory(int shmid);



