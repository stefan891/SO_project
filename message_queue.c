
#include "message_queue.h"

// id diversi
int createMessageQueue(key_t messqueKey)
{

    int id_queue = msgget(messqueKey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (id_queue == -1)
        ErrExit("msgget failed");

    return id_queue;
}

void removeMessageQueue(int msqid)
{
    if (msgctl(msqid, IPC_RMID, NULL) == -1)
        ErrExit("msgctl failed");
    else
        printf("\nmessage queue removed successfully\n");
}


void invioMessaggio(int id_queue, struct MsgQue MessQue)
{
    size_t mSize = sizeof(MessQue) - sizeof(long);

    if (msgsnd(id_queue, &MessQue, mSize, IPC_NOWAIT) == -1)
    {
        ErrExit("msgsnd failed");
    }
    else
    {
        printf("\nmsg send\n");
    }
}

void ricezioneMessaggio(int id_queue, struct MsgQue punt)
{

    size_t mSize = sizeof(struct MsgQue) - sizeof(long);

    if (msgrcv(id_queue, &punt, mSize, 0, 0) == -1)
    {
        ErrExit("msgrcv failed");
    }
    else
    {
        printf("\nreceive msg\n");
    }
}

struct msqid_ds msqGetStats(int msqid){

    struct msqid_ds ds;

    if (msgctl(msqid, IPC_STAT, &ds) == -1)
        ErrExit("msgctl STAT");

    return ds;
}


void msqSetStats(int msqid, struct msqid_ds ds){

    if (msgctl(msqid, IPC_SET, &ds) == -1) {
        if(errno != EPERM) {
            ErrExit("msgctl SET");
        }
        else {
            DEBUG_PRINT("Couldn't set new config: not enough permissions. Continuing anyway\n");
        }
    }
}
