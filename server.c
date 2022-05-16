/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

int global_fd1;
int global_fd2;

void sigHandler(int signal)
{
    printf("\n/server/:ricevuto segnale sigint,termino\n");

    close_FIFO(global_fd1, "fifo1");
    close_FIFO(global_fd2, "fifo2");

    kill(getpid(), SIGTERM);

}

int main(int argc, char *argv[])
{
    printf("PROCESS ID %d\n", getpid());
    fflush(stdout);

    make_FIFO("fifo1");
    make_FIFO("fifo2");

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        ErrExit("signal handler failed");

    global_fd1=open_FIFO("fifo1", O_WRONLY);

    char file_path[PATH_MAX];
    getcwd(file_path, PATH_MAX);
    strcat(file_path, "/myDir/sendme_1");
    int fd=open(file_path,O_RDWR|S_IWUSR|S_IRUSR);
    if(fd==-1)
        ErrExit("open failed");


    char test[20]="casa/myfile";

    write_FIFO(global_fd1,fd,2,23,test);
    close(fd);


    pause();

    return 0;
}







