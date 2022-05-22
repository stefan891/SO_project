//
// Created by francesco on 5/19/22.
//
#include "defines.h"
#include "semaphore.h"
#include "err_exit.h"

void listFiles(const char* dirname) {


    DIR* dir = opendir(dirname);

    if (dir == NULL)
    {
        return;
    }

    printf("Reading files in: %s\n", dirname);

    struct dirent* entity;
    entity = readdir(dir);
    while (entity != NULL) {
        printf("%hhd %s/%s\n", entity->d_type, dirname, entity->d_name);

        //se entity è una cartella, non è . e neanche ..
        //(che sarebbero sovracartelle) allora ci entro
        if (entity->d_type == DT_DIR && strcmp(entity->d_name, ".") != 0
            && strcmp(entity->d_name, "..") != 0) {

            char path[100] = { 0 };
            strcat(path, dirname);
            strcat(path, "/");
            strcat(path, entity->d_name);
            listFiles(path);
        }
        entity = readdir(dir);
    }



    closedir(dir);
}

int main(int argc, char *argv[]) {
    char buffer[100];
    listFiles(strcat(getcwd(buffer,100),"/myDir"));
    return 0;
}