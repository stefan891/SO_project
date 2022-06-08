/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "message_queue.h"

// id della fifo 1
int global_fd1 = -1;
// id della fifo 2
int global_fd2 = -1;
// id della message queue
int id_msgqueue = -1;
// puntatore della shared memory
struct Responce *shm_ptr = NULL;
// puntatore della shared memory di supporto
bool *data_ready = NULL;
// shared memory di supporto
int shm_data_ready = -1;
// shared memory
int shm_id = -1;
// grandezza della message queue
ssize_t mSize = sizeof(struct MsgQue) - sizeof(long);
// conteggio numero di file per la funzione di ricostruzione
int file_count = 0;

void sigHandler(int signal)
{
    printf("\n/server/:ricevuto segnale sigint,termino\n");

    close_FIFO(global_fd1, "fifo1");
    close_FIFO(global_fd2, "fifo2");
    removeMessageQueue(id_msgqueue);
    detach_shared_memory(shm_ptr);
    detach_shared_memory(data_ready);
    remove_shared_memory(shm_id);
    remove_shared_memory(shm_data_ready);
    exit(0);
}

int main(int argc, char *argv[])
{
    DEBUG_PRINT("PROCESS ID %d\n", getpid());

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        ErrExit("signal handler failed");

    while (1)
    {
        make_FIFO("fifo1");
        make_FIFO("fifo2");
        // shared memory
        // alloco la schared memory per rispondere al client, poi lo sblocco
        shm_id = alloc_shared_memory(SHMKEY1, 50 * sizeof(struct Responce), IPC_CREAT);
        shm_ptr = (struct Responce *)get_shared_memory(shm_id, 0);
        DEBUG_PRINT("memoria condivisa allocata e connessa\n");

        // inizializzazione shared memory di supporto
        shm_data_ready = alloc_shared_memory(SHM_SUPP, 50 * sizeof(bool), IPC_CREAT);
        data_ready = (bool *)get_shared_memory(shm_data_ready, 0);
        for (int i = 0; i < 50; i++)
            data_ready[i] = false;

        /// message queue
        id_msgqueue = createMessageQueue(MSGQKEY, IPC_CREAT);
        DEBUG_PRINT("MESSAGE QUEUE ID: %d", id_msgqueue);

        //variabile per ricevere il messaggio dalla message queue
        struct MsgQue msg_queue_responce;

        /// comunicazione con il client_0
        // mi metto in ascolto del client su fifo1
        global_fd1 = open_FIFO("fifo1", O_RDONLY);

        // risposta del client_0 sul numero di files
        struct Responce risposta = read_FIFO(global_fd1, NULL);
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

        // rappresenta il numero totale di parti di file da ricevere
        int count = n_file * 4;
        DEBUG_PRINT("nfile = %d", n_file);

        // cambio modalità delle 2 fifo, non bloccante
        global_fd2 = open_FIFO("fifo2", O_RDONLY);

        global_fd1 = open_FIFO("fifo1", O_RDONLY | O_NONBLOCK);
        global_fd2 = open_FIFO("fifo2", O_RDONLY | O_NONBLOCK);

        // struttura per ricostruire i file, n_file righe, 4 colonne
        struct Responce **ricostruzione_file = (struct Responce **)malloc(n_file * sizeof(struct Responce *));
        for (int i = 0; i < n_file; i++)
            ricostruzione_file[i] = (struct Responce *)malloc(4 * sizeof(struct Responce));

        file_count = 0;
        long error = 0;

         //limito la coda
         //funziona solo se fatto partire in modalità super user (sudo)
         struct msqid_ds ds = msqGetStats(id_msgqueue);
         ds.msg_qbytes = sizeof(struct MsgQue) * MAX_MESS_CHANNEL;
         msqSetStats(id_msgqueue, ds);

        /// leggo dalle 4 IPC (fifo 1-2,msgq,shmemory)
        while (count > 0)
        {

            /// FIFO 1
            error = 0;
            risposta = read_FIFO(global_fd1, &error);

            if (error != EAGAIN && errno != EINTR)
            {
                DEBUG_PRINT("lettura da fifo1");
                FileReconstruct(&risposta, ricostruzione_file, &file_count, n_file);
                count--;
                semOp(semaforo_ipc, 0, 1, 0);
            }

            /// FIFO 2
            error = 0;
            risposta = read_FIFO(global_fd2, &error);

            if (error != EAGAIN && errno != EINTR)
            {
                DEBUG_PRINT("lettura da fifo2");
                FileReconstruct(&risposta, ricostruzione_file, &file_count, n_file);
                count--;
                semOp(semaforo_ipc, 1, 1, 0);
            }

            /// MESSAGE QUEUE
            mSize = sizeof(struct MsgQue) - sizeof(long);
            if (msgrcv(id_msgqueue, &msg_queue_responce, mSize, 0, IPC_NOWAIT) == -1)
            {
                if (errno == ENOMSG)
                    ;
                else
                    ErrExit("msgrcv failed");
            }
            else
            {
                DEBUG_PRINT("lettura da message queue");
                risposta.file_number = msg_queue_responce.file_number;
                strcpy(risposta.filepath, msg_queue_responce.filepath);
                risposta.additional = msg_queue_responce.additional;
                strcpy(risposta.content, msg_queue_responce.content);
                FileReconstruct(&risposta, ricostruzione_file, &file_count, n_file);
                count--;
                semOp(semaforo_ipc, 2, 1, 0);
            }

            /// SHARED MEMORY
            // mutua esclusione lettura su shmem
            semOp(semaforo_supporto, 0, -1, 0);
            // ciclo su array di supporto finchè non trovo la prima partizione occupata (true)
            for (int j = 0; j < 50; j++)
            {
                if (data_ready[j])
                {
                    DEBUG_PRINT("lettura da shared memory");
                    semOp(semaforo_ipc, 3, 1, 0);

                    // copio i dati nella struttura responce
                    risposta.file_number = shm_ptr[j].file_number;
                    strcpy(risposta.filepath, shm_ptr[j].filepath);
                    risposta.additional = shm_ptr[j].additional;
                    strcpy(risposta.content, shm_ptr[j].content);
                    data_ready[j] = false;

                    FileReconstruct(&risposta, ricostruzione_file, &file_count, n_file);
                    count--;
                    break;
                }
            }

            semOp(semaforo_supporto, 0, 1, 0);
        }
        DEBUG_PRINT("\n\nRICOSTRUZIONE FINITA\nverifica sturttura e creazione files\n");
        fflush(stdout);

        // invio il messaggio di fine lavori al client (message type 2)
        struct MsgQue msg_queue;
        strcpy(msg_queue.filepath, "");
        strcpy(msg_queue.content, "");
        msg_queue.file_number = 0;
        msg_queue.additional = 0;
        msg_queue.mtype = 2;
        if (msgsnd(id_msgqueue, &msg_queue, mSize, 0) == -1)
            ErrExit("msgsnd failed");

        // vettore supporto per trasformare le parti int in char (con sprintf)
        char numero[8];

        for (int i = 0; i < 4; i++)
        {
            printf("\n\nposizione %d", i);
            for (int a = 0; a < n_file; a++)
                printf("\nfilepath aperto:  %s", ricostruzione_file[a][i].filepath);
        }

        for (int i = 0; i < n_file; i++)
        {
            // printf("\nfilepath aperto:  %s",ricostruzione_file[i][1].filepath);
            fflush(stdout);
            int fd = open(strcat(ricostruzione_file[i][0].filepath, "_out"), O_RDWR | O_CREAT | O_TRUNC,
                          S_IRUSR | S_IWUSR);
            if (fd == -1)
                ErrExit("open failed");

            char riga[MSG_BYTES + 150];
            strcpy(riga, "");

            //ricostruzione del file concatenando tutto il necessario
            for (int a = 0; a < 4; a++)
            {
                /*  printf("\n\n[parte %d file %s pid %d]\n%s", ricostruzione_file[i][a].file_number,
                         ricostruzione_file[i][a].filepath,
                         ricostruzione_file[i][a].additional, ricostruzione_file[i][a].content);*/
               // fflush(stdout);

                strcpy(riga, "\n\n[parte ");
                sprintf(numero, "%d", ricostruzione_file[i][a].file_number);
                strcat(riga, numero);
                strcat(riga, ",del file ");
                strcat(riga, ricostruzione_file[i][0].filepath);
                strcat(riga, ", spedita dal processo ");
                sprintf(numero, "%d", ricostruzione_file[i][0].additional);
                strcat(riga, numero);
                strcat(riga, " tramite ");
                switch (ricostruzione_file[i][a].file_number)
                {
                case 1:
                    strcat(riga, "FIFO1]\n");
                    break;
                case 2:
                    strcat(riga, "FIFO2]\n");
                    break;
                case 3:
                    strcat(riga, "MsgQueue]\n");
                    break;
                case 4:
                    strcat(riga, "ShdMem]\n");
                    break;
                default:
                    strcat(riga, "NULL]\n");
                }
                strcat(riga, ricostruzione_file[i][a].content);

                write(fd, riga, strlen(riga));
            }
            close(fd);
        }

        printf("\nPROVA A CHIUDERE TUTTO");
        fflush(stdout);
        close_FIFO(global_fd1, "fifo1");
        close_FIFO(global_fd2, "fifo2");
        removeMessageQueue(id_msgqueue);
        detach_shared_memory(shm_ptr);
        detach_shared_memory(data_ready);
        remove_shared_memory(shm_id);
        remove_shared_memory(shm_data_ready);

        for (int i = 0; i < n_file; i++)
            free(ricostruzione_file[i]);
        free(ricostruzione_file);

        removeSemaphore(semaforo_supporto);
        removeSemaphore(semaforo_ipc);

        // removeMessageQueue(id_msgqueue);

        printf("\nTUTTO CHIUSO");
        fflush(stdout);
    }
}
