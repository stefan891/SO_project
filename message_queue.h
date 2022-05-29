#pragma once
#include "err_exit.h"
#include "defines.h"
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * It creates a message queue with the given key
 *
 * @param messqueKey the key of the message queue
 *
 * @return The id of the message queue
 */
int createMessageQueue(key_t messqueKey);

/**
 * It removes the message queue with the given id
 *
 * @param msqid The message queue identifier returned by msgget().
 */
void removeMessageQueue(int msqid);


/**
 * It sends a message to the queue
 *
 * @param id_queue the queue identifier
 * @param MessQue the message queue structure
 */
void invioMessaggio(int id_queue, struct MsgQue MessQue);


/**
 * It receives a message from the queue
 *
 * @param id_queue the queue identifier
 * @param punt pointer to the message structure
 */
void ricezioneMessaggio(int id_queue, struct MsgQue punt);

/**
 * It returns a copy of the data structure that describes the state of the message queue
 * 
 * @param msqid The message queue identifier.
 * 
 * @return A struct msqid_ds
 */
struct msqid_ds msqGetStats(int msqid);

/**
 * It sets the new configuration of the message queue
 * 
 * @param msqid the message queue identifier
 * @param ds the struct containing the new configuration
 */
void msqSetStats(int msqid, struct msqid_ds ds);
