/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include "err_exit.h"
#include "fifo.h"
#include "defines.h"

char FIFO_path[PATH_MAX]; // path assoluto file FIFO

/**
 * It creates a FIFO file in the IPCS directory
 *
 * @param name the name of the FIFO
 */
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

/**
 * It opens a FIFO with the given name, in the given mode (read_or_write)
 *
 * @param name the name of the FIFO
 * @param read_or_write 0 for read, 1 for write (O_RDONLY/O_WRONLY)
 *
 * @return The file descriptor of the opened FIFO.
 */
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


/**
 * close the fifo with the file descriptor
 * if name!=NULL, also
 * deletes the file of the fifo with the name
 *
 * @param fd the file descriptor of the FIFO
 * @param name the name of the FIFO (not the entire path)
 */
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



/**
 * It reads from the FIFO file descriptor and reconstructs the Responce structure
 * struct responce content:
 * responce.content: the content of the message read
 * responce.filepath: the content of the filepath of the message read
 * responce.file_number: the content of the file_number of the message read
 * responce.additional: the content of number additional of the message read
 *
 * @param FIFO_fd the file descriptor of the FIFO
 *
 * @return a struct Responce.
 */
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

    printf("\n<read> content size: %ld filepath size: %ld\n", content_size,filepath_size);//debug

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



struct File_piece file_piece;   //struttura per scrivere la struct da inviare

/**
 * It reads a piece of file from source_fd, a filepath from *path, and additional information.
 * puts all in a struct and send it on the FIFO_fd
 *
 * @param FIFO_fd the file descriptor of the FIFO
 * @param source_fd file descriptor of the file to be read (if no ned to read file, put 0)
 * @param file_number the number of the file being sent
 * @param additional number for any additional information
 * @param path the path of where the file belongs (if no need to write any path, put NULL)
 */
void write_FIFO(int FIFO_fd,int source_fd,int file_number,int additional,char *path)
{
    //leggo il messaggio (lungo massimo 1024) lo metto in file_piece.content
    // e salvo la dimensione effettiva letta in file_piece.size
    if(source_fd>0)
        file_piece.content_size=read(source_fd,&file_piece.content,sizeof (file_piece.content));
    else
        file_piece.content_size=0;


    //copio la stinga del path passata, in file_piece.content, subito dopo il messaggio
    //e la dimensione in .filepath_size
    if(path!=NULL)
    {
        strcat(file_piece.content,path);
        file_piece.filepath_size= strlen(path);
    }
    else
        file_piece.filepath_size=0;


    printf("\n<write> content size: %ld filepath size: %ld\n", file_piece.content_size,file_piece.filepath_size);//debug
    fflush(stdout);

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















