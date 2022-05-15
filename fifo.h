/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once


/*crea la fifo nella current working directory/IPCS
@param name: il nome che si vuole dare alla fifo per identificarla */
void make_FIFO(char* name);


/*apre la fifo nella current working directory/IPCS
@param name: il nome della fifo che si vuole aprire
@param read_or_write: O_RDNLY O_WRONLY */
int open_FIFO(char* name, int read_or_write);

void close_FIFO(int fd,char*name);

struct Responce read_FIFO(int FIFO_fd);
void write_FIFO(int FIFO_fd,int source_fd,int file_number,int additional,char* path);