/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "err_exit.h"
#include "defines.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include <sys/msg.h>

char *global_path;       // variabile globale per passare argv[1] al sigHandler
char **legit_files_path; // matrice di stringhe per salvare il path dei soli file "legali"

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
        //------------------------------------------------------------------------------------
        char path[PATH_MAX];

        printf("\nciao %s ora inizio l'invio dei file contenuti in ", getlogin());

        getcwd(path, PATH_MAX);
        strcat(path, "/");
        strcat(path, global_path);
        printf("%s\n", path);
        //------------------------------------------------------------------------------------

        // lettura files nella directory
        //------------------------------------------------------------------------------------
        DIR *dp = opendir(path);
        if (dp == NULL)
            ErrExit("directory inesistente");

        errno = 0;
        struct dirent *dentry;
        struct stat statbuf;
        int legit_files = 0;

        char filepath[PATH_MAX]; // variabile di supporto uguale a path+nome di ogni file in fondo
        strcpy(filepath, path);
        strcat(filepath, "/");

        while ((dentry = readdir(dp)) != NULL) // ad ogni iterazione la funzione readdir avanza automaticamente con la lettura dei files
        {

            if (dentry->d_type == DT_REG && strncmp("sendme_", dentry->d_name, 7) == 0)
            {
                stat(strcat(filepath, dentry->d_name), &statbuf); // ad ogni ciclo prendo il path e aggiungo il nome file per recavarne lo statbuf

                if (statbuf.st_size < 4000) // aggiungo solo files minori di 4kb
                {

                    printf("\n%s", dentry->d_name);
                    legit_files++;
                    printf(" file size: %ld", statbuf.st_size);

                    // con questo modo molto figo, alloco dinamicamente un vettore di stringhe man mano
                    // che trovo i file, in modo da non dover allocare sempre un vettore di 100 stringhe
                    legit_files_path = realloc(legit_files_path, sizeof(char *));
                    legit_files_path[legit_files - 1] = malloc(PATH_MAX * sizeof(char));
                    strcpy(legit_files_path[legit_files - 1], filepath); // copio il file path di ogni file "legit" nel vettore di stringhe che sto creando

                    strcpy(filepath, path); // resetto il contenuto di filepath a /myDir, se no mi aggiunge dietro tutti i nomi dei file attaccati
                    strcat(filepath, "/");
                }
            }
            errno = 0;
        }
        if (errno != 0)
            ErrExit("error reading dir.\n");
        //----------------------------------------------------------------------------------------

        //######################################################## //verifica del vettore di stringhe, ma la puoi cancellare
        printf("\nverifica lettura file legit");
        for (int i = 0; i < legit_files; i++)
        {
            printf("\n%s", legit_files_path[i]);
            fflush(stdout);
        }
        //#######################################################

            //creazione e settaggio semaforo di supporto
        //--------------------------------------------------------------------------
        int semaforo_supporto=createSemaphore(ftok(NULL,SEMKEY1),1,IPC_CREAT);
        union semun arg;
        arg.val=(unsigned short)0;
        if(semctl(semaforo_supporto,0,SETVAL,arg)==-1)
            ErrExit("semctl failed");
        //---------------------------------------------------------------------------


        //mi metto in attesa del server su fifo1 per scrivere il n di file
        int global_fd1= open_FIFO("fifo1",O_WRONLY);
        write_FIFO(global_fd1,0,legit_files,0,NULL);

        semOp(semaforo_supporto,(unsigned short)0,-1,0);

        int id_memoria=alloc_shared_memory(SHMKEY1,50 * 5120 * sizeof(char));
        char *ptr= get_shared_memory(id_memoria,0);

       // removeSemaphore(semaforo_supporto);

        if(atoi(ptr)>0)
        {
            //setto un semaforo mutex per una mutua esclusione sui processi figli
            int semaforo_mutex=createSemaphore(ftok(NULL,SEMMUTEXKEY1),1,IPC_CREAT);
            arg.val=(unsigned short )1;
            if(semctl(semaforo_mutex,0,SETVAL,arg)==-1)
                ErrExit("semctl failed");

            //setto un altro semaforo per bloccare i processi alla creazione del file
            int semaforo_nfiles=createSemaphore(ftok(NULL,SEMKEY2),1,IPC_CREAT);
            union semun arg3;
            arg3.val=(unsigned short)legit_files;
            if(semctl(semaforo_nfiles,0,SETVAL,arg3)==-1)
                ErrExit("semctl failed");



            //divisione file in 4 e creazione figli
        //-------------------------------------------------------------------------------------
            for(int i=0;i<legit_files;i++)
            {
                pid_t pid=fork();
                if(pid==-1)
                    ErrExit("fork failed");

                //codice eseguito dal child--------------

                else if(pid==0)
                {
                    semOp(semaforo_mutex,(unsigned short)0,-1,0);

                    printf("\nfiglio %d",i);

                    //divido il file in 4 parti, mi viene ritornata una struttura con 4 stringhe
                    struct Divide divide;
                    divide= divideByFour(legit_files_path[i]);
                    printf("\npart1: %s\npart2: %s\npart3: %s\npart4: %s",divide.part1,divide.part2,divide.part3, divide.part4);
                    fflush(stdout);

                    sleep(1);

                   // semOp(semaforo_nfiles,(unsigned short )0,-1,IPC_NOWAIT);

                    semOp(semaforo_mutex,(unsigned short)0,1,0);

                    //aspetto che il semaforo arrivi a 0 per liberare tutti i figli
                    //semOp(semaforo_nfiles,(unsigned short)0,0,0);
                    printSemaphoreValue(semaforo_nfiles,1);
                    printf("\nfiglio %d finito",i);
                    fflush(stdout);
                    exit(0);
                }
            }
        //-------------------------------------------------------------------------------------
            //codice eseguito dal parent

        }
        else
            ErrExit("no files to read");

    }
}


//#########################################################################################################################
                                                        //MAIN
//#########################################################################################################################

int main(int argc, char *argv[])
{

    printf("PROCESS ID %d\n", getpid());
    fflush(stdout);

    if (argc != 2)
    {
        printf("usage: ./client_0 myDir/\n");
        exit(1);
    }
    else
        global_path = argv[1];

    sigset_t set_segnali;

    //setting maschera segnali per SIGINT e SIGUSR1
    sigfillset(&set_segnali);
    sigdelset(&set_segnali, SIGUSR1);
    sigdelset(&set_segnali, SIGINT);
    sigprocmask(SIG_SETMASK, &set_segnali, NULL);

    if (signal(SIGINT, sigHandler) == SIG_ERR || signal(SIGUSR1, sigHandler) == SIG_ERR)
        ErrExit("signal handler failed");

    //attendo ricezione di segnale SIGINT o SIGUSR1
    pause();

    while (wait(NULL) != -1);

    printf("\n<parent>end\n");

    return 0;
}