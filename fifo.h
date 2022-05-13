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

char* read_FIFO(int fd);
void write_FIFO(int fd,char*buffer);