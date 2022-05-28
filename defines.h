/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#define MSG_BYTES 1024
#define PATH_SIZE 150
#define SHMKEY1 20
#define SHMKEY_SUPP_MUTEX 21
#define SHM_SUPP 40

#define SEMKEY1 11
#define SEMIPCKEY 14
#define SEMKEY2 32
#define SEMMUTEXKEY1 12     //chiave per mutex shared memory
#define MSGQKEY 30

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <unistd.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
//#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include "debug.h"

//non modificare il content
//message queue è grande massimo per 3 messaggi
//la message queue vine occupata da 3 messaggi
//faremo un semaforo da 3
struct File_piece{
    ssize_t content_size;
    ssize_t filepath_size;
    int piece;
    int additional;
    char content[MSG_BYTES+PATH_SIZE];

};

// ms_type > 0 legge tutti i messaggi con quel message type
struct Responce{

    char content[MSG_BYTES];    //contenuto del messaggio
    char filepath[PATH_SIZE];         //path del mittente
    int file_number;            //segment of the file delivered
    int additional;             //pid of the trocess

};

struct Divide{

    char part1[MSG_BYTES+1];
    char part2[MSG_BYTES+1];
    char part3[MSG_BYTES+1];
    char part4[MSG_BYTES+1];

};

struct MsgQue{
  long mtype;               //messaggi della message queue
  char content[50];
  char filepath[50];
  int file_number;         //pezzo di file
  int additional;          //pid

};

/**
 * It opens the file, reads it, and divides it into four parts
 *
 * @param dirname the name of the file to be divided
 *
 * @return A struct containing 4 strings.
 */
struct Divide divideByFour(char *path);

/**
 * It recursively reads a directory and its subdirectories, and returns the number of files that are
 * smaller than 4kb and that start with "sendme_".
 * The problem is that the function returns the correct number of files, but the array of strings that
 * I pass to it is not filled correctly.
 * essendo la funzione ricorsiva, legit files va fuori per non essere re inizializzato ad ogni chiamata
 * Here's the code that calls the function:
 * 
 * @param dirname the path to the directory to be read
 * @param legit_files_path an array of strings, each string is a path to a file
 * 
 * @return The number of files found.
 */
int readDir(const char *dirname,char **legit_files_path);

/**
 * funzione da usare in caso per le chiavi con la ftok (non la stiamo usando)
 * It gets the current working directory, appends the string "myDir" to it, and returns the result
 *
 * @return The path to the directory.
 */
char* getDirectoryPath();

/**
 * The function `print_msg` writes the string `msg` to the standard output
 * 
 * @param msg The message to be printed.
 */
void print_msg(char * msg);
int FileReconstruct(struct Responce *source,struct Responce **dest,int *count,int n_file);

char* getDirectoryPath();
















