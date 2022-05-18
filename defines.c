/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"
#include "err_exit.h"
char Buffer[PATH_MAX];



struct Divide divideByFour(char *path)
{
    //apro il file con fopen per leggere solo i caratteri
    FILE *f= fopen(path,"r");
    struct Divide divide;
    divide.part1[0]='\0';
    divide.part2[0]='\0';
    divide.part3[0]='\0';
    divide.part4[0]='\0';

    long resto=0;
    int ch=0;
    long dimensione=0;
    long br=0;

    if(f==NULL)
        ErrExit("\n<divide by 4>open failed");

    //conto il numero caratteri nel file
    while (1) {
        ch = fgetc(f);
        if (ch == EOF)
            break;
        ++dimensione;
    }
    fclose(f);

    resto=dimensione%4;

    //riapro il file con la open normale
    int fd= open(path,O_RDONLY);

    //calcolo la dimensione ed eventuale resto da aggiungere all'ultima parte



    //inserisco le 4 parti nella struttura
    br+=read(fd,divide.part1,dimensione/4);
    divide.part1[br]='\0';
    br+=read(fd,divide.part2,dimensione/4);
    divide.part2[br]='\0';
    br+=read(fd,divide.part3,dimensione/4);
    divide.part3[br]='\0';
    br+=read(fd,divide.part4,(dimensione/4)+resto);
    divide.part4[br]='\0';

    //if(br!=dimensione)
      //  ErrExit("<divide by 4>byte read not equal to file size");

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
