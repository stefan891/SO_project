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

    global_fd1 = open_FIFO("fifo1", O_RDONLY);    //mi metto in ascolto del client su fifo1
    struct Responce risposta = read_FIFO(global_fd1);      //risposta del client_0 sul numero di files
    int n_file = risposta.file_number;

    printf("\n<server>ricevuto n di file %d, additional %d", n_file,risposta.additional);
    fflush(stdout);


    //alloco la schared memory per rispondere al client, poi lo sblocco

    int id_memoria = alloc_shared_memory(SHMKEY1, 50 * 5120 * sizeof(char));
    char *shmptr = get_shared_memory(id_memoria, 0);
    if(n_file>0)
        strcpy(shmptr,"1\0");
    else
        strcpy(shmptr,"-1\0");

    printf("\nmemoria allocata");
    fflush(stdout);


    //leggo il semaforo creato dal client e lo sblocco
    int semaphore_id = createSemaphore(ftok(NULL,SEMKEY1), 1,0);
    semOp(semaphore_id, 0, 1, 0);
    printSemaphoreValue(semaphore_id,1);

    int count=n_file;

    global_fd2= open_FIFO("fifo2",O_RDONLY);
    while(count>0)
    {
        risposta=read_FIFO(global_fd1);
        printf("parte %d,del file %s, spedita da processo %d tramite fifo1\n%s",
               risposta.file_number,risposta.filepath,risposta.additional,risposta.content);
        risposta= read_FIFO(global_fd2);
        printf("parte %d,del file %s, spedita da processo %d tramite fifo1\n%s",
               risposta.file_number,risposta.filepath,risposta.additional,risposta.content);
        count--;
    }

    pause();

    free_shared_memory(shmptr);

    return 0;
}






