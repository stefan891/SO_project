/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"
#include "err_exit.h"
char Buffer[PATH_MAX];

//variabile globale per readdir, con reset di supporto
int reset=0;

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
        printf("\nWARNING <divide by 4>open failed, revert file to 0 byte");

    //conto il numero caratteri nel file
    resto=dimensione%4;

    //calcolo la dimensione ed eventuale resto da aggiungere all'ultima parte
    //inserisco le 4 parti nella struttura
    if(fd!=-1)
    {
        br+=read(fd,divide.part1,dimensione/4);
        divide.part1[br]='\0';
        br+=read(fd,divide.part2,dimensione/4);
        divide.part2[br]='\0';
        br+=read(fd,divide.part3,dimensione/4);
        divide.part3[br]='\0';
        br+=read(fd,divide.part4,(dimensione/4)+resto);
        divide.part4[br]='\0';

        close(fd);

        if(br!=dimensione)
            ErrExit("\n<divide by 4>byte read not equal to file size");

    }
    else
    {
        divide.part1[0]='\0';
        divide.part2[0]='\0';
        divide.part3[0]='\0';
        divide.part4[0]='\0';
    }


    return divide;
}



int readDir(const char *dirname,char **legit_files_path, int legit)
{
    struct dirent *dentry;
    struct stat statbuf;


    printf("\n<readDir> reading files in: %s",dirname);

    //apertura cartella dirname
    DIR *dirp= opendir(dirname);

    //caso base
    if(dirp==NULL){
        ErrExit("path inesistente");
        return 0;
    }

    //leggo il contenuto della cartella passata da opendir
    dentry= readdir(dirp);

    while(dentry != NULL)
    {

        //se il file letto da reddir è una cartella, e NON è una sovra cartella (. ..)
        //allora ci entro e la leggo (serve per non ciclare all'infinito tra radice e foglia)
        if(dentry->d_type==DT_DIR && strcmp(".",dentry->d_name)!=0 && strcmp("..",dentry->d_name)!=0)
        {
            //costruisco il path per entrare nella sottocartella
            char temp_path[PATH_SIZE]={0};
            strcat(temp_path,dirname);
            strcat(temp_path,"/");
            strcat(temp_path,dentry->d_name);

            //chiamata ricorsiva a readDir
            legit=readDir(temp_path,legit_files_path,legit);
        }
        //se è un file normale
        else if(dentry->d_type==DT_REG)
        {
            //specifiche file per debug (il numero indica il tipo di file)
           // printf("\n%hhd %s/%s", dentry->d_type, dirname, dentry->d_name);

            //leggo le statistiche del file tramite il temp_path= path(corrente)+/+nome file letto da readdir
            char temp_path1[PATH_SIZE]={};
            strcat(temp_path1,dirname);
            strcat(temp_path1,"/");
            strcat(temp_path1,dentry->d_name);
            stat(temp_path1, &statbuf);

            // aggiungo solo files minori di 4kb e che iniziano per "sendme_" e il cui path è <100 caratteri
            if (statbuf.st_size < 4096 && strncmp("sendme_",dentry->d_name,7)==0 && strlen(temp_path1)<100)
            {
                legit++;
                printf("\n %s LEGIT %d",dentry->d_name,legit);
                fflush(stdout);

                //mi è stato passato un array di puntatori a stringhe, ne inizializzo 1 per volta
                unsigned long len=strlen(temp_path1);
                legit_files_path[legit-1]= malloc(len*sizeof (char ));
                strcpy(legit_files_path[legit-1],temp_path1);


                // resetto il contenuto di filepath a /path, se no mi aggiunge dietro tutti i nomi dei file attaccati
                strcpy(temp_path1, dirname);
                strcat(temp_path1, "/");
            }
        }
        //leggo il prossimo file o cartella
        dentry= readdir(dirp);

    }
    //chiudo il file e ritorno il numero di files letti, resettando prima la variabile legit
    closedir(dirp);

    return legit;
}



int FileReconstruct(struct Responce *source,struct Responce **dest,int *count,int n_file)
{
    printf("\nfilepath sorgente %s",source->filepath);
    for(int a=0;a<=n_file;a++)
    {
        //se trovo una corrispondenza di un file già scritto, aggiungo il pezzo nuovo
        if(source->file_number<=4 && a<n_file) {

            //controllo tutte le righe e colonne per vedere se il pezzo di file è completamente nuovo oppure
            //se ne stavo già scrivendo un pezzo
            if (strcmp(source->filepath, dest[a][0].filepath) == 0 || strcmp(source->filepath, dest[a][1].filepath) == 0 ||
                    strcmp(source->filepath, dest[a][2].filepath) == 0 || strcmp(source->filepath, dest[a][3].filepath) == 0) {
                printf("\ntrovata corrisp: %s#\nnumber %d",source->filepath,source->file_number);
                fflush(stdout);
                //parto da file_number-1 perchè si inizia da 0
                strcpy(dest[a][source->file_number - 1].filepath, source->filepath);
                strcpy(dest[a][source->file_number - 1].content, source->content);

                dest[a][source->file_number - 1].file_number = source->file_number;
                dest[a][source->file_number - 1].additional = source->additional;
                break;
            }
        }
        //se non trovo corrispondenza, allora è un file nuovo e lo aggiungo alla prima locazione libera
        if(a==n_file && *count<n_file)
        {
            printf("\nFILE NUOVO %d: %s#\nnumber %d",*count,source->filepath,source->file_number);

            fflush(stdout);
            strcpy(dest[*count][source->file_number-1].filepath,source->filepath);
            strcpy(dest[*count][source->file_number-1].content,source->content);

            dest[*count][source->file_number-1].file_number=source->file_number;
            dest[*count][source->file_number-1].additional=source->additional;
            *count=*count+1;
        }

    }

    return 1;
}

char* getDirectoryPath(){

    getcwd(Buffer, PATH_MAX);
    strcat(Buffer, "/");
    strcat(Buffer, "myDir");
    printf("%s\n", Buffer);
    fflush(stdout);

    return Buffer;

}

void print_msg(char * msg){
    if (write(STDOUT_FILENO, msg, strlen(msg)) == -1){
        ErrExit("write stdout failed");
    }
}

void blockFD(int fd, bool blocking){

    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        ErrExit("error blokFD");

    if(blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;


    if (fcntl(fd, F_SETFL, flags) == -1)
        ErrExit("fifo non resa bloccante");

}

