#include "defines.h"
#include "semaphore.h"
#include "err_exit.h"
#define KEY 43

int main(int argc, char const *argv[])
{
    key_t firstKey = ftok(NULL, KEY);
    int semid = createSemaphore(firstKey, 1, IPC_CREAT);
    
    union semun arg;
    arg.val = 1;
    
    if(semctl(semid, 0 , SETVAL, arg)==-1)
        ErrExit("semctl failed");

    printSemaphoreValue(semid, 1);
    for(int i = 0; i< 4; i++ ){
        pid_t pid = fork();
        if(pid ==-1 )
            ErrExit("child not created\n");
        else if(pid == 0){
            semOp(semid, 0, -1, 0);
            printf("figlio%d:\n", i);
            printf("%d\n", i);
            printf("%d\n", i);
            printf("%d\n", i);
            sleep(2);
            semOp(semid, 0, 1, 0);
            return 0;
        }
    }

    while (wait(NULL) != -1);
    removeSemaphore(semid);
    printSemaphoreValue(semid, 1);

    printf("\nparent terminated\n");
    
    return 0;
}
