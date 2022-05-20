/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"
#include "err_exit.h"

char Buffer[PATH_MAX];
struct stat buffer_file;


struct Divide divideByFour(char *path)
{
    //apro il file con fopen per leggere solo i caratteri
    int fd= open(path,O_RDONLY);
    fstat(fd, &buffer_file);
    struct Divide divide;

    long resto=0;
    long dimensione=buffer_file.st_size;
    long br=0;

    if(fd==-1)
        ErrExit("\n<divide by 4> open failed");

    resto=dimensione%4;


    //inserisco le 4 parti nella struttura
    br+=read(fd,divide.part1,dimensione/4);
    divide.part1[br]='\0';
    br+=read(fd,divide.part2,dimensione/4);
    divide.part2[br]='\0';
    br+=read(fd,divide.part3,dimensione/4);
    divide.part3[br]='\0';
    br+=read(fd,divide.part4,((dimensione/4)+resto));
    divide.part4[br]='\0';

    if(br!=dimensione)
        ErrExit("<divide by 4>byte read not equal to file size");

    close(fd);

    return divide;
}



//funzione da usare in caso per le chiavi con la ftok (non la stiamo usando)
char* getDirectoryPath(){

    getcwd(Buffer, PATH_MAX);
    strcat(Buffer, "/");
    strcat(Buffer, "myDir");
    printf("%s\n", Buffer);
    fflush(stdout);

    return Buffer;

}
