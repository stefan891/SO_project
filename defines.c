/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"

char Buffer[PATH_MAX];

void divideByFour()
{
    printf("file diviso\n");
}

char* getDirectoryPath(){

    getcwd(Buffer, PATH_MAX);
    strcat(Buffer, "/");
    strcat(Buffer, "myDir");
    printf("%s\n", Buffer);
    fflush(stdout);

    return Buffer;

}
