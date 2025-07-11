#include <sys/queue.h>
#include <pthread.h>
#include <stdlib.h>

#include "message_quque.h"

struct msg_entry {
  char c;
  TAILQ_ENTRY(msg_entry)
  entries; /* List. */
};

TAILQ_HEAD(msg_listhead, msg_entry);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct msg_listhead msg_head;

void MsgQueueInit() {
    TAILQ_INIT(&msg_head); /* Initialize the list. */
}

void MsgQueuePush(char c) {
    struct msg_entry *np = (struct msg_entry *)malloc(sizeof(struct msg_entry)); /* Insert at the head. */
    np->c = c;

    pthread_mutex_lock(&mutex);
    TAILQ_INSERT_TAIL(&msg_head, np, entries);
    pthread_mutex_unlock(&mutex);
}

char MsgQueuePop() {
    struct msg_entry *np = TAILQ_FIRST(&msg_head);
    char c = np->c;

    pthread_mutex_lock(&mutex);
    TAILQ_REMOVE(&msg_head, np, entries);
    pthread_mutex_unlock(&mutex);

    free(np);

    return c;
}

bool MsgQueueIsEmpty() {
    return TAILQ_FIRST(&msg_head) == NULL;
}
