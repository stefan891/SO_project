/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include <sys/shm.h>
#include <sys/stat.h>
#include "err_exit.h"
#include "shared_memory.h"

/**
 * It creates a shared memory segment of the specified size, and returns the shared memory ID
 *
 * @param shmKey The key to be used to identify the shared memory segment.
 * @param size The size of the shared memory segment in bytes.
 *
 * @return The shared memory ID.
 */
int alloc_shared_memory(key_t shmKey, size_t size) {

    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1)
        ErrExit("shmget failed");

    return shmid;
}

/**
 * It attaches the shared memory segment identified by the shmid parameter to the address space of the calling process
 *
 * @param shmid the shared memory identifier
 * @param shmflg This is a bit mask that specifies the permissions for the shared memory segment. The following are the
 * possible values:
 *
 * @return A pointer to the shared memory.
 */
void *get_shared_memory(int shmid, int shmflg) {
    //esegue l'attach della memoria condivisa
    void *ptr_sh = shmat(shmid, NULL, shmflg);
    if (ptr_sh == (void *) -1)
        ErrExit("shmat failed");

    return ptr_sh;
}

/**
 * It creates a shared memory segment of size `size` and returns a pointer to it
 *
 * @param ptr_sh pointer to the shared memory
 */
void free_shared_memory(void *ptr_sh) {
    //esegue il detach della memoria condivisa
    if (shmdt(ptr_sh) == -1)
        ErrExit("shmdt failed");
}

/**
 * It creates a shared memory segment of size `size` and returns the shared memory identifier
 *
 * @param shmid the shared memory segment ID
 */
void remove_shared_memory(int shmid) {
    //cancella il segmento di memoria condivisa
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        ErrExit("remove failed");
}
