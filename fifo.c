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
            printf("\nWARNING <make_fifo> %s,: %s\n",FIFO_path, strerror(errno));
        else
            ErrExit("mkfifo failed:");
    }
    else
        printf("\nFIFO: creata in %s", FIFO_path);

    fflush(stdout);
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
    }
    fflush(stdout);
    return fd;
}


void close_FIFO(int fd, char *name)
{
    getcwd(FIFO_path, PATH_MAX);
    strcat(FIFO_path, "/IPCS/");
    strcat(FIFO_path, name);

    close(fd);
    printf("\nFIFO:file descriptor %s chiuso", FIFO_path);
    fflush(stdout);

    if(name!=NULL)
    {
        unlink(FIFO_path);
        printf("\nFIFO: %s rimossa", FIFO_path);
        fflush(stdout);
    }

}




struct Responce read_FIFO(int FIFO_fd)
{
    struct Responce responce;
    //dicharo tutte le variabili per ricostruire la struttura come era
    //stata mandata
    ssize_t content_size=-1;
    ssize_t filepath_size=-1;


    // leggo la dimensione del messaggio (.content_size struttura)
    if(read(FIFO_fd, &content_size, sizeof(ssize_t))==-1)
        ErrExit("FIFO read failed");
    //leggo la dimensione del filepath (.filepath_size struttura)
    if(read(FIFO_fd,&filepath_size,sizeof (ssize_t))==-1)
        ErrExit("FIFO read failed");

    //printf("\n<read> content size: %ld filepath size: %ld\n", content_size,filepath_size);//debug

    //leggo il numero di file
    read(FIFO_fd,&responce.file_number,sizeof (int ));
    //leggo il campo additional
    read(FIFO_fd,&responce.additional,sizeof (int ));


    //leggo la parte di stringa del contenuto
    read(FIFO_fd,responce.content,content_size);
    responce.content[content_size]='\0';

    //leggo la restante parte della stringa, contenente il filepath
    read(FIFO_fd,responce.filepath,filepath_size);
    responce.filepath[filepath_size]='\0';

    fflush(stdout);

    return responce;

}

void write_FIFO(int FIFO_fd,char *source_string,int file_number,int additional,char *path)
{
    file_piece=empty_file_piece;

    ///leggo il messaggio (lungo massimo 1024) lo metto in file_piece.content
    /// e salvo la dimensione effettiva letta in file_piece.size
    if(source_string!=NULL)
    {
        strcpy(file_piece.content,source_string);
        file_piece.content_size= strlen(source_string);
    }


    ///copio la stinga del path passata, in file_piece.content, subito dopo il messaggio
    ///e la dimensione in .filepath_size
    if(path!=NULL)
    {
        strcat(file_piece.content,path);
        file_piece.filepath_size= strlen(path);
    }
    else
        file_piece.filepath_size=0;


   // printf("\n<write> content size: %ld filepath size: %ld\n", file_piece.content_size,file_piece.filepath_size);//debug
    //fflush(stdout);

    if(file_piece.content_size==-1)
        ErrExit("FIFO source file read failed");

    //copio le informazioni addizionali nella struct
    file_piece.piece=file_number;
    file_piece.additional=additional;

    //se non ci sono stati errori (file vuoto, oppure con dei caratteri letti)
    if(file_piece.content_size>=0)
    {
        //calcolo dimensione totale byte struttura da mandare
        ssize_t byte_to_send= sizeof(file_piece.content_size)+sizeof (file_piece.filepath_size)+sizeof (file_piece.piece)+
                              sizeof(file_piece.additional)+file_piece.content_size+file_piece.filepath_size;

        //invio struttura tramite fifo
        ssize_t  byte_write=write(FIFO_fd,&file_piece,byte_to_send);
        fflush(stdout);

        if(byte_write!=byte_to_send)
            ErrExit("FIFO write failed");
    }
    else
        printf("\nWARNING <write_fifo> file not written to FIFO");
}















