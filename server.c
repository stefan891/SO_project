/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "message_queue.h"

int global_fd1;
int global_fd2;
int id_msgqueue = -1;
ssize_t mSize;

void sigHandler(int signal)
{
    printf("\n/server/:ricevuto segnale sigint,termino\n");

    close_FIFO(global_fd1, "fifo1");
    close_FIFO(global_fd2, "fifo2");

}

int main(int argc, char *argv[])
{
    DEBUG_PRINT("PROCESS ID %d\n", getpid());

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        ErrExit("signal handler failed");

    make_FIFO("fifo1");
    make_FIFO("fifo2");

    // shared memory
    // alloco la schared memory per rispondere al client, poi lo sblocco
    int shm_id = alloc_shared_memory(SHMKEY1, 50 * sizeof(struct Responce));
    struct Responce *shm_ptr = (struct Responce *)get_shared_memory(shm_id, 0);
    DEBUG_PRINT("memoria condivisa allocata e connessa\n");

    // inizializzazione shared memory di supporto
    int shm_data_ready = alloc_shared_memory(SHM_SUPP, 50 * sizeof(bool));
    bool *data_ready = (bool *)get_shared_memory(shm_data_ready, 0);

    // message queue

    id_msgqueue = createMessageQueue(MSGQKEY);
    DEBUG_PRINT("MESSAGE QUEUE ID: %d", id_msgqueue);

    struct MsgQue msg_queue_responce;

    // comunicazione con il client_0
    global_fd1 = open_FIFO("fifo1", O_RDONLY);        // mi metto in ascolto del client su fifo1
    struct Responce risposta = read_FIFO(global_fd1); // risposta del client_0 sul numero di files
    int n_file = risposta.file_number;
    shm_ptr[0].file_number = n_file;

    DEBUG_PRINT("<server>ricevuto n di file %d, additional %d", n_file, risposta.additional);
    DEBUG_PRINT("memoria allocata");

    /// leggo il semaforo per le 4 ipc creato dal client
    /// FIFO_1 FIFO_2  MSGQ  SHMEM
    int semaforo_ipc = createSemaphore(SEMIPCKEY, 4, 0);

    // leggo il semaforo creato dal client e lo sblocco
    int semaforo_supporto = createSemaphore(SEMKEY1, 1, 0);
    semOp(semaforo_supporto, 0, 1, 0);


    //rappresenta il numero totale di parti di file da ricevere
    int count = n_file*3;
    DEBUG_PRINT("nfile = %d", n_file);

    global_fd2 = open_FIFO("fifo2", O_RDONLY);

    //struttura per ricostruire i file, n_file righe, 4 colonne
    struct Responce **ricostruzione_file=(struct Responce**)malloc(n_file*sizeof (struct Responce*));
    for(int i=0;i<n_file+1;i++)
        ricostruzione_file[i]=(struct Responce*) malloc(4*sizeof (struct Responce));


    int file_count=0;

    // leggo dalle 4 IPC (fifo 1-2,msgq,shmemory)
    while (count > 0)
    {
        sleep(1);

        /// FIFO 1
        risposta = read_FIFO(global_fd1);
        //printf("\n[parte %d,del file %s, spedita da processo %d tramite fifo1]\n%s",
          //     risposta.file_number, risposta.filepath, risposta.additional, risposta.content);

        FileReconstruct(&risposta, ricostruzione_file, &file_count, n_file);

        count--;
        semOp(semaforo_ipc, 0, 1, 0);

        /// FIFO 2
        risposta = read_FIFO(global_fd2);
       // printf("\n[parte %d,del file %s, spedita da processo %d tramite fifo2]\n%s",
         //      risposta.file_number, risposta.filepath, risposta.additional, risposta.content);

        FileReconstruct(&risposta, ricostruzione_file, &file_count, n_file);

        count--;
        semOp(semaforo_ipc, 1, 1, 0);

        // MESSAGE QUEUE
        //struct MsgQue support;
        mSize = sizeof(struct MsgQue) - sizeof(long);
        if (msgrcv(id_msgqueue, &msg_queue_responce, mSize, 0, IPC_NOWAIT) == -1)
        {
            ErrExit("msgrcv failed");
        }
        else
        {
            printf("\n[parte %d,del file %s, spedita da processo %d tramite message queue]\n%s",
                   msg_queue_responce.file_number, msg_queue_responce.filepath, msg_queue_responce.additional, msg_queue_responce.content);
            fflush(stdout);
        }
        semOp(semaforo_ipc, 2, 1, 0);

        /// SHARED MEMORY
        // mutua esclusione lettura su shmem
        semOp(semaforo_supporto, 0, -1, 0);
        // ciclo su array di supporto finchè non trovo la prima partizione occupata (true)
        for (int j = 0; j < 50; j++)
        {
            if (data_ready[j])
            {
                semOp(semaforo_ipc, 3, 1, 0);
            //    printf("\n[parte %d,del file %s, spedita da processo %d tramite shared memory]\n%s",
              //         shm_ptr[j].file_number, shm_ptr[j].filepath, shm_ptr[j].additional, shm_ptr[j].content);

                //copio i dati nella struttura responce
                risposta.file_number=shm_ptr[j].file_number;
                strcpy(risposta.filepath,shm_ptr[j].filepath);
                risposta.additional=shm_ptr[j].additional;
                strcpy(risposta.content,shm_ptr[j].content);
                data_ready[j] = false;

                FileReconstruct(&risposta, ricostruzione_file, &file_count, n_file);
                break;
            }
        }

        count--;
        semOp(semaforo_supporto, 0, 1, 0);

    }
    DEBUG_PRINT("\nSTAMPA FINITA\nverifica sturttura\n\n");
    fflush(stdout);

    char numero[8];


    for(int i=0;i<n_file;i++)
    {
        int fd= open(strcat(ricostruzione_file[i][0].filepath,"_out"),O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        //int fd= open(strcat(ricostruzione_file[i][0].filepath,"_out"),O_CREAT|O_RDWR|S_IWUSR|S_IRUSR);
        if(fd==-1)
            ErrExit("open failed");

        char riga[MSG_BYTES+150];
        strcpy(riga,"");

        for(int a=0;a<3;a++)
        {
            printf("\n[parte %d file %s pid %d]\n%s",ricostruzione_file[i][a].file_number,ricostruzione_file[i][a].filepath,
                   ricostruzione_file[i][a].additional,ricostruzione_file[i][a].content);
            fflush(stdout);

            strcpy(riga,"\n[parte ");
            sprintf(numero,"%d",ricostruzione_file[i][a].file_number);
            strcat(riga,numero);
            strcat(riga,",del file ");
            strcat(riga,ricostruzione_file[i][0].filepath);
            strcat(riga,", spedita dal processo ");
            sprintf(numero,"%d",ricostruzione_file[i][0].additional);
            strcat(riga,numero);
            strcat(riga," tramite ");
            switch(ricostruzione_file[i][a].file_number)
            {
                case 1:
                    strcat(riga,"FIFO1]\n");
                    break;
                case 2:
                    strcat(riga,"FIFO2]\n");
                    break;
                case 3:
                    strcat(riga,"MsgQueue]\n");
                    break;
                case 4:
                    strcat(riga,"ShdMem]\n");
                    break;
                default:
                    strcat(riga,"NULL]\n");

            }
            strcat(riga,ricostruzione_file[i][a].content);

            write(fd,riga, strlen(riga));
        }
        close(fd);

    }



    pause();

    free(ricostruzione_file);
    removeSemaphore(semaforo_supporto);
    removeSemaphore(semaforo_ipc);
    remove_shared_memory(shm_id);
    remove_shared_memory(shm_data_ready);
    free_shared_memory(shm_ptr);
    free_shared_memory(data_ready);

    kill(getpid(), SIGTERM);

    return 0;
}
