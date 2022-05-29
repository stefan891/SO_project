/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "err_exit.h"
#include "defines.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include <sys/msg.h>

// variabile globale per passare argv[1] al sigHandler
char *global_path;

// funzione per creare subito tutti i semafori

void sigHandler(int signal)
{
    if (signal == SIGUSR1)
    {
        kill(getpid(), SIGTERM);
        printf("ricevuto segnale sigusr1\n");
    }
    else if (signal == SIGINT)
    {

        // procedura per salutare l'utente
        char path[PATH_SIZE];

        printf("\nciao %s ora inizio l'invio dei file contenuti in ", getlogin());

        getcwd(path, PATH_MAX);
        strcat(path, "/");
        strcat(path, global_path);
        printf("%s\n", path);

        // lettura files nella directory
        char *legit_files_path[PATH_SIZE] = {};
        int legit_files = readDir(path, legit_files_path);


        // creazione e settaggio semaforo di supporto da condividere col server
        int semaforo_supporto = createSemaphore(SEMKEY1, 1, IPC_CREAT);
        semSetVal(semaforo_supporto, 0, "semaforo_supporto primo");
        DEBUG_PRINT("Semafori: ottenuto il set di semafori DI SUPPORTO\n");

        // creo un set da 4 semafori da 50 per le IPC, da sincronizzare col server
        // FIFO_1 FIFO_2  MSGQ  SHMEM
        int semaforo_ipc = createSemaphore(SEMIPCKEY, 4, IPC_CREAT);
        unsigned short sem_ipc_initVal[] = {3, 3, 3, 3};
        semSetAll(semaforo_ipc, sem_ipc_initVal, "sem_ipc");

        DEBUG_PRINT("semaforo_ipc %d", semaforo_ipc);

        // mi metto in attesa del server su fifo1 per scrivere il n di file
        int global_fd1 = open_FIFO("fifo1", O_WRONLY);
        write_FIFO(global_fd1, NULL, legit_files, 0, NULL);

        // mi blocco per aspettare che il server crei la shmem per leggerci
        semOp(semaforo_supporto, (unsigned short)0, -1, 0);

        // mi aggancio alla shmem creata dal server (SUPPORTO E VERA)
        int id_memoria = alloc_shared_memory(SHMKEY1, 50 * sizeof(struct Responce));
        struct Responce *ptr = (struct Responce *)get_shared_memory(id_memoria, 0);

        int shm_data_ready = alloc_shared_memory(SHM_SUPP, 50 * sizeof(bool));
        bool *data_ready = (bool *)get_shared_memory(shm_data_ready, 0);

        // mi assicuro che il file number sia corretto
        if (ptr[0].file_number > 0)
        {
            // creo un semaphore set da 2 mutex per una mutua esclusione sui processi figli
            int semaforo_mutex = createSemaphore(IPC_PRIVATE, 2, IPC_CREAT);
            unsigned short mutex_semInitVal[] = {1, legit_files};
            semSetAll(semaforo_mutex, mutex_semInitVal, "sem_mutex");

            DEBUG_PRINT("semafor_mutex %d", semaforo_mutex);

            int global_fd2 = open_FIFO("fifo2", O_WRONLY);

            // ri-setto il semaforo di supporto per ls shared memory
            semSetVal(semaforo_supporto, 1, "semaforo_supporto");


            // divisione file in 4 e creazione figli
            for (int i = 0; i < legit_files; i++)
            {
                pid_t pid = fork();
                if (pid == -1)
                    ErrExit("fork failed");

                // CHILD
                else if (pid == 0)
                {
                    // mutua esclusione sul primo semaforo
                    semOp(semaforo_mutex, 0, -1, 0);

                    printf("\n\nfiglio %d", i);
                    fflush(stdout);

                    // divido il file in 4 parti, mi viene ritornata una struttura con 4 stringhe
                    struct Divide divide;
                    divide = divideByFour(legit_files_path[i]);
                    printf("\npart1: %s\npart2: %s\npart3: %s\npart4: %s", divide.part1, divide.part2, divide.part3, divide.part4);
                    fflush(stdout);
                    // decremento il secondo semaforo(legit)
                    semOp(semaforo_mutex, 1, -1, IPC_NOWAIT);
                    // sblocco figlio successivo (primo semaforo)
                    semOp(semaforo_mutex, 0, 1, 0);
                    // aspetto che il secondo semaforo arrivi a 0 per liberare tutti i figli
                    semOp(semaforo_mutex, 1, 0, 0);

                    // tutti i figli,una volta letto le 4 parti, vengono liberati insieme
                    DEBUG_PRINT("figlio %d finito", i);

                    // ciclo while per mandare i messaggi
                    int count = 3;

                    static const struct Responce empty_responce;

                    while (count > 0)
                    {
                        semOp(semaforo_ipc, 0, -1, 0);
                        write_FIFO(global_fd1, divide.part1, 1, getpid(), legit_files_path[i]);
                        count--;

                        semOp(semaforo_ipc, 1, -1, 0);
                        write_FIFO(global_fd2, divide.part2, 2, getpid(), legit_files_path[i]);
                        count--;

                        // mutua esclusione scrittura su shared memory
                        semOp(semaforo_supporto, 0, -1, 0);
                        semOp(semaforo_ipc, 3, -1, IPC_NOWAIT);

                        // ciclo su array di supporto finchè non trovo la prima partizione libera (true)
                        for (int j = 0; j < 50; j++)
                        {
                            if (!data_ready[j])
                            {
                                ptr[j] = empty_responce;        //pulizia struct su cui scrivere
                                strcpy(ptr[j].content, divide.part3);
                                strcpy(ptr[j].filepath, legit_files_path[i]);
                                ptr[j].additional = getpid();
                                ptr[j].file_number = 3;
                                data_ready[j] = true;
                                count--;
                                break;
                            }
                        }
                        semOp(semaforo_supporto, 0, 1, 0);
                    }
                    DEBUG_PRINT("figlio %d file inviati", i);

                    exit(0);
                }
            }

            /// PARENT
            // aspetto tutti i figli e rimuovo i semafori del client
            while (wait(NULL) != -1);

            remove_shared_memory(id_memoria);
            remove_shared_memory(shm_data_ready);
            free_shared_memory(ptr);
            free_shared_memory(data_ready);
            removeSemaphore(semaforo_mutex);
            return;
        }
        else
            ErrExit("no files to read");
    }
}


///MAIN
int main(int argc, char *argv[])
{

    DEBUG_PRINT("PROCESS ID %d\n", getpid());

    if (argc != 2)
    {
        printf("usage: ./client_0 myDir/\n");
        exit(1);
    }
    else
        global_path = argv[1];

    sigset_t set_segnali;

    // setting maschera segnali per SIGINT e SIGUSR1
    sigfillset(&set_segnali);
    sigdelset(&set_segnali, SIGUSR1);
    sigdelset(&set_segnali, SIGINT);
    sigprocmask(SIG_SETMASK, &set_segnali, NULL);

    if (signal(SIGINT, sigHandler) == SIG_ERR || signal(SIGUSR1, sigHandler) == SIG_ERR)
        ErrExit("signal handler failed");

    // attendo ricezione di segnale SIGINT o SIGUSR1
    pause();


    printf("\n<parent>end\n");

    return 0;
}