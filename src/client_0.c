/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "../inc/err_exit.h"
#include "../inc/defines.h"
#include "../inc/fifo.h"

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

        char filepath[PATH_MAX]; // variabile di supporto guguale a path+nome di ogni file in fondo
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

        int fd=open_FIFO("fifo1",O_RDONLY); //prova apertura fifo creata dal server
        read_FIFO(fd,STDOUT_FILENO);
        fflush(stdout);
        
    }
}

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

    sigfillset(&set_segnali);
    sigdelset(&set_segnali, SIGUSR1);
    sigdelset(&set_segnali, SIGINT);
    sigprocmask(SIG_SETMASK, &set_segnali, NULL);

    if (signal(SIGINT, sigHandler) == SIG_ERR || signal(SIGUSR1, sigHandler) == SIG_ERR)
        ErrExit("signal handler failed");

    pause();

    printf("\n\nend\n");

    return 0;
}
