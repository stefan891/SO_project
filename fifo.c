/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include "err_exit.h"
#include "fifo.h"
#include "defines.h"

char FIFO_path[PATH_MAX]; // path assoluto file FIFO

void make_FIFO(char *name)
{
    errno = 0;
    getcwd(FIFO_path, PATH_MAX);
    strcat(FIFO_path, "/IPCS/");
    strcat(FIFO_path, name);

    int res = mkfifo(FIFO_path, S_IWUSR | S_IRUSR);

    if (res == -1)
    {
        if (errno == EEXIST)
            printf("\nWARNING %s,: %s\n",FIFO_path, strerror(errno));
        else
            ErrExit("mkfifo failed:");
    }
    else
        printf("\nFIFO: creata in %s", FIFO_path);
}

int open_FIFO(char *name, int read_or_write)
{
    getcwd(FIFO_path, PATH_MAX);
    strcat(FIFO_path, "/IPCS/");
    strcat(FIFO_path, name);

    int fd = open(FIFO_path, read_or_write);
    if (fd == -1)
    {
        ErrExit("\nfifo open failed");
        return fd;
    }

    else
    {
        printf("\nFIFO: %s aperta con successo", FIFO_path);
        fflush(stdout);
        return fd;
    }
}

void close_FIFO(int fd, char *name)
{
    getcwd(FIFO_path, PATH_MAX);
    strcat(FIFO_path, "/IPCS/");
    strcat(FIFO_path, name);

    close(fd);
    unlink(FIFO_path);
    printf("\nFIFO: %s rimossa", FIFO_path);
    fflush(stdout);
}

char buffer[100];

 char* read_FIFO(int fd)
 {
     ssize_t Br;
     do{
         Br=read(fd,buffer,sizeof (buffer));
     }while(Br>0);

     return buffer;

 }
 void write_FIFO(int fd,char*buffer)
{
    write(fd,buffer,strlen(buffer));

}
















