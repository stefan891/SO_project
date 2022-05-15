/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#define MSG_BYTES 1024
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <unistd.h>
#include <syscall.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

struct File_piece{
    ssize_t content_size;
    ssize_t filepath_size;
    int piece;
    int additional;
    char content[MSG_BYTES+PATH_MAX];

};

struct Responce{

    char content[MSG_BYTES];
    char filepath[PATH_MAX];
    int file_number;
    int additional;

};

void divideByFour();
