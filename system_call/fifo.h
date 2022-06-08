/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.
#pragma once

#include "defines.h"

struct File_piece file_piece;   //struttura per scrivere la struct da inviare
static const struct File_piece empty_file_piece;

/**
 * It creates a FIFO file in the IPCS directory
 *
 * @param name the name of the FIFO
 */
void make_FIFO(char *name);

/**
 * It opens a FIFO with the given name (directory/IPCS), in the given mode (read_or_write)
 *
 * @param name the name of the FIFO
 * @param read_or_write 0 for read, 1 for write (O_RDONLY/O_WRONLY)
 *
 * @return The file descriptor of the opened FIFO.
 */
int open_FIFO(char *name, int read_or_write);

/**
 * close the fifo with the file descriptor
 * if name!=NULL, also
 * deletes the file of the fifo with the name
 *
 * @param fd the file descriptor of the FIFO
 * @param name the name of the FIFO (not the entire path)
 */
void close_FIFO(int fd, char *name);

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
struct Responce read_FIFO(int FIFO_fd,long *error);

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
void write_FIFO(int FIFO_fd, char *source_string, int file_number, int additional, char *path);