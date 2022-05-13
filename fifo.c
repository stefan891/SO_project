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


struct File_piece file_piece;

 /**
  * It reads from the FIFO and writes to the destination file
  *
  * @param FIFO_fd the file descriptor of the FIFO
  * @param dest_fd the file descriptor of the file to write to
  */
 void read_FIFO(int FIFO_fd,int dest_fd)
 {
     char buffer[MSG_BYTES+1];
     ssize_t byte_read=-1;
     ssize_t size;
     //leggo la dimensione della struct
     byte_read =read(FIFO_fd,&size,sizeof (ssize_t));
     int piece=0;
     //leggo il numero del file
     read(FIFO_fd,&piece,sizeof (int ));
     printf("\nnumero file %d\n\n",piece);
     fflush(stdout);

     do{

         if(byte_read==-1)
             printf("\nWARNING FIFO nothing read");
         else
         {
             byte_read= read(FIFO_fd,buffer,size);
             if(byte_read==size)
             {
                 //buffer[size]='\0';
                 write(dest_fd,buffer,byte_read);
             }
             else
             {
                 buffer[size]='\0';
                 printf("WARNING FIFO byte read not equal to byte write");
             }
         }
     }while(byte_read>0);


 }
 void write_FIFO(int FIFO_fd,int source_fd,int file_number)
 {
    do {
        file_piece.piece=file_number;

        file_piece.size=read(source_fd,file_piece.content,sizeof (file_piece.content));
        if(file_piece.size==-1)
            ErrExit("\nFIFO write failed");
        if(file_piece.size>0)
        {
            ssize_t byte_to_send=sizeof (file_piece.size)+sizeof(file_piece.piece)+file_piece.size;

            ssize_t byte_write=write(FIFO_fd,&file_piece,byte_to_send);
            if(byte_write!=byte_to_send)
                printf("\nWARNING FIFO byte write not equal to byte read");
        }

    }while(file_piece.size>0);

}
















