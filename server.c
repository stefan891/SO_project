/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

int global_fd1;
int global_fd2;



void sigHandler(int signal) {
    printf("\n/server/:ricevuto segnale sigint,termino\n");

    close_FIFO(global_fd1, "fifo1");
    close_FIFO(global_fd2, "fifo2");

    kill(getpid(), SIGTERM);

}

int main(int argc, char *argv[]) {
    printf("PROCESS ID %d\n", getpid());
    fflush(stdout);

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        ErrExit("signal handler failed");

    make_FIFO("fifo1");
    make_FIFO("fifo2");

    //shared memory

    //alloco la schared memory per rispondere al client, poi lo sblocco
    int shm_id = alloc_shared_memory(SHMKEY1, 50 * sizeof(struct Responce));
    struct Responce *shm_ptr = (struct Responce*)get_shared_memory(shm_id, 0);
    DEBUG_PRINT("memoria condivisa allocata e connessa\n");

    //inizializzazione array di supporto protetto da mutex
    /*int semid_shm_mutex = createSemaphore(SHMKEY_SUPP_MUTEX, 1, IPC_CREAT);
    semSetVal(semid_shm_mutex, 1);
    semOp(semid_shm_mutex, (unsigned short)0, -1, 0);
    for(int i = 0; i<sizeof(shm_support_array); i++)
        shm_support_array[i] = true;
    semOp(semid_shm_mutex, 0, 1, 0);*/
    //creazione mutex per shared memory
    int shm_mutex = createSemaphore(SEMMUTEXKEY1, 1, IPC_CREAT);


    //message queue

    //..

    //comunicazione con il client_0
    global_fd1 = open_FIFO("fifo1", O_RDONLY);              //mi metto in ascolto del client su fifo1
    struct Responce risposta = read_FIFO(global_fd1);      //risposta del client_0 sul numero di files
    int n_file = risposta.file_number;
    shm_ptr[0].file_number = n_file;

    DEBUG_PRINT("\n<server>ricevuto n di file %d, additional %d", n_file,risposta.additional);
    DEBUG_PRINT("memoria allocata");

    //leggo il semaforo per le 4 ipc creato dal client
    //FIFO_1 FIFO_2  MSGQ  SHMEM
    int semaforo_ipc = createSemaphore(SEMIPCKEY,4,0);

    //leggo il semaforo creato dal client e lo sblocco
    int semaforo_supporto = createSemaphore(SEMKEY1, 1,0);
    semOp(semaforo_supporto, 0, 1, 0);
    printSemaphoreValue(semaforo_supporto,1);

    int count=n_file;
    DEBUG_PRINT("nfile = %d", n_file);

    global_fd2= open_FIFO("fifo2",O_RDONLY);

    while(count>0)
    {
        sleep(1);
        risposta=read_FIFO(global_fd1);
        printf("[parte %d,del file %s, spedita da processo %d tramite fifo1]\n%s",
               risposta.file_number,risposta.filepath,risposta.additional,risposta.content);
        semOp(semaforo_ipc,0,1,0);

        risposta= read_FIFO(global_fd2);
        printf("parte %d,del file %s, spedita da processo %d tramite fifo2\n%s",
               risposta.file_number,risposta.filepath,risposta.additional,risposta.content);
        semOp(semaforo_ipc,1,1,0);
        count--;
        //risposta = shm_ptr[3];

        /*semOp(semid_shm_mutex, 0, -1, 0);
        int i = 0;
        bool flag = false;
        while (!flag)
        {
            if(shm_support_array[i])
                i++;
            else
                flag = true;
        }
        printf("parte %d,del file %s, spedita da processo %d tramite shared memory\n%s",
               shm_ptr[i].file_number,shm_ptr[i].filepath,shm_ptr[i].additional,shm_ptr[i].content);
        shm_support_array[i] = true;
        semOp(semid_shm_mutex, 0, 1, 0);

        */
    }
    fflush(stdout);

    pause();
    removeSemaphore(semaforo_ipc);
    free_shared_memory(shm_ptr);

    return 0;
}






