/// @file shared_memory.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#pragma once

#define _SHARED_MEMORY_HH

#include <stdlib.h>

/**
 * It creates a shared memory segment of the specified size, and returns the shared memory ID
 *
 * @param shmKey The key to be used to identify the shared memory segment.
 * @param size The size of the shared memory segment in bytes.
 *
 * @return The shared memory ID.
 */
int alloc_shared_memory(key_t shmKey, size_t size);

/**
 * It attaches the shared memory segment identified by the shmid parameter to the address space of the calling process
 *
 * @param shmid the shared memory identifier
 * @param shmflg This is a bit mask that specifies the permissions for the shared memory segment. The following are the
 * possible values:
 *
 * @return A pointer to the shared memory.
 */
void *get_shared_memory(int shmid, int shmflg);

/**
 * It creates a shared memory segment of size `size` and returns a pointer to it
 *
 * @param ptr_sh pointer to the shared memory
 */
void detach_shared_memory(void *ptr_sh);

/**
 * It creates a shared memory segment of size `size` and returns the shared memory identifier
 *
 * @param shmid the shared memory segment ID
 */
void remove_shared_memory(int shmid);



