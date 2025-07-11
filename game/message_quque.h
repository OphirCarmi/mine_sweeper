#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE

#include <stdbool.h>

void MsgQueueInit();

void MsgQueuePush(char c);

char MsgQueuePop();

bool MsgQueueIsEmpty();

#endif  // MESSAGE_QUEUE