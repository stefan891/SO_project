/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include <sys/shm.h>
#include <sys/stat.h>
#include "err_exit.h"
#include "shared_memory.h"


int alloc_shared_memory(key_t shmKey, size_t size) {

    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1)
        ErrExit("shmget failed");

    return shmid;
}

void *get_shared_memory(int shmid, int shmflg) {
    //esegue l'attach della memoria condivisa
    void *ptr_sh = shmat(shmid, NULL, shmflg);
    if (ptr_sh == (void *) -1)
        ErrExit("shmat failed");

    return ptr_sh;
}

/**
 * It frees a shmemory pointer
 *
 * @param ptr_sh pointer to the shared memory
 */
void free_shared_memory(void *ptr_sh) {
    //esegue il detach della memoria condivisa
    if (shmdt(ptr_sh) == -1)
        ErrExit("shmdt failed");
}

void remove_shared_memory(int shmid) {
    //cancella il segmento di memoria condivisa
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        ErrExit("remove failed");
}
