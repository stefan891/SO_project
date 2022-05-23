/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#define MSG_BYTES 1024
#define SHMKEY1 20
#define SHMKEY_SUPP_MUTEX 21

#define SEMKEY1 11
#define SEMMUTEXKEY1 14
#define SEMKEY2 12
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
#include <limits.h>
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
    char content[MSG_BYTES+100];

};

// ms_type > 0 legge tutti i messaggi con quel message type
struct Responce{

    char content[MSG_BYTES];    //contenuto del messaggio
    char filepath[100];         //path del mittente
    int file_number;            //segment of the file delivered
    int additional;             //pid of the trocess

};

struct Divide{

    char part1[MSG_BYTES+1];
    char part2[MSG_BYTES+1];
    char part3[MSG_BYTES+1];
    char part4[MSG_BYTES+1];

};

bool shm_support_array[50];

struct Divide divideByFour(char *path);

int readDir(const char *dirname,char **legit_files_path);

char* getDirectoryPath();
