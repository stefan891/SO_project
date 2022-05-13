/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"


void divideByFour()
{
    printf("file diviso\n");
}

struct file_piece{
    ssize_t size;
    int piece;
    char content[50];

};