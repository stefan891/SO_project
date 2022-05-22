/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"
#include "err_exit.h"
char Buffer[PATH_MAX];


static const struct Divide empty_divide;

struct Divide divideByFour(char *dirname)
{
    struct Divide divide;
    divide=empty_divide;
    //apro il file con fopen per leggere solo i caratteri
    int fd=open(dirname,O_RDONLY);
    struct stat buf;
    fstat(fd,&buf);
    long resto=0;
    long dimensione=buf.st_size;
    long br=0;

    if(fd==-1)
        ErrExit("\n<divide by 4>open failed");

    //conto il numero caratteri nel file
    resto=dimensione%4;

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

    if(br!=dimensione)
        ErrExit("\n<divide by 4>byte read not equal to file size");

    close(fd);

    return divide;
}





/*
 *legge tutti i files legittimi in una sottocartella
 * @param *dirname: la stringa del path corrente fino a questa cartella, senza / finale
 * @param legit_files_path: puntatore alla variabile globale nel main
 */
//essendo la funzione ricorsiva, legit files va fuori per non essere re inizializzato ad ogni chiamata
int legit_files=0;

int readDir(const char *dirname,char **legit_files_path)
{
    struct dirent *dentry;
    struct stat statbuf;


    printf("\n<readDir> reading files in: %s",dirname);

    //apertura cartella dirname
    DIR *dirp= opendir(dirname);

    //caso base
    if(dirp==NULL)
        return 0;

    //leggo il contenuto della cartella passata da opendir
    dentry= readdir(dirp);

    while(dentry != NULL)
    {

        //se il file letto da reddir è una cartella, e NON è una sovra cartella (. ..)
        //allora ci entro e la leggo (serve per non ciclare all'infinito tra radice e foglia)
        if(dentry->d_type==DT_DIR && strcmp(".",dentry->d_name)!=0 && strcmp("..",dentry->d_name)!=0)
        {
            //costruisco il path per entrare nella sottocartella
            char temp_path[100]={0};
            strcat(temp_path,dirname);
            strcat(temp_path,"/");
            strcat(temp_path,dentry->d_name);

            //chiamata ricorsiva a readDir
            readDir(temp_path,legit_files_path);
        }
        //se è un file normale
        else if(dentry->d_type==DT_REG)
        {
            //specifiche file per debug (il numero indica il tipo di file)
           // printf("\n%hhd %s/%s", dentry->d_type, dirname, dentry->d_name);

            //leggo le statistiche del file tramite il temp_path= path(corrente)+/+nome file letto da readdir
            char temp_path1[100]={};
            strcat(temp_path1,dirname);
            strcat(temp_path1,"/");
            strcat(temp_path1,dentry->d_name);
            stat(temp_path1, &statbuf);

            // aggiungo solo files minori di 4kb e che iniziano per "sendme_"
            if (statbuf.st_size < 4096 && strncmp("sendme_",dentry->d_name,7)==0)
            {
                legit_files++;
                printf("\n %s LEGIT %d",dentry->d_name,legit_files);
                fflush(stdout);

                //mi è stato passato un array di puntatori a stringhe, ne inizializzo 1 per volta
                unsigned long len=strlen(temp_path1);
                legit_files_path[legit_files-1]= malloc(len*sizeof (char ));
                strcpy(legit_files_path[legit_files-1],temp_path1);


                // resetto il contenuto di filepath a /path, se no mi aggiunge dietro tutti i nomi dei file attaccati
                strcpy(temp_path1, dirname);
                strcat(temp_path1, "/");
            }
        }
        //leggo il prossimo file o cartella
        dentry= readdir(dirp);

    }
    //chiudo il file e ritoprno il numero di files letti
    closedir(dirp);
    return legit_files;

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
