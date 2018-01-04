#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PARENT_PORT 4950
#define BACKLOG 20
#define MAXBUFLEN 100
#define MAX_CHILD_SERVERS 100
#define FIFO_NAME "FIFO_NAME"
#define PORT_LEN 4
#define MAX_CHILD_POP 2

// Keep next line commented to run the actual program.  Uncomment it to run unit tests.
//#define TEST

#ifndef TEST
#define FOREVER 1
#define THREAD_INTERVAL 10
#else
#define FOREVER 0
#define THREAD_INTERVAL 0
#endif

#endif  // CONSTANTS_H
