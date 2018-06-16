#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PARENT_PORT 4950
#define BACKLOG 20
#define MAXBUFLEN 100
#define MAX_CHILD_SERVERS 100
#define FREE_PORT_FIFO_NAME "/home/rishabh/Documents/MatchmakingTicTacToe/cpp-server/FREE_PORT_FIFO"
#define WAITING_SERVERS_FIFO_NAME "/home/rishabh/Documents/MatchmakingTicTacToe/cpp-server/WAITING_SERVERS_FIFO"
#define EMPTY_SERVERS_FIFO_NAME "/home/rishabh/Documents/MatchmakingTicTacToe/cpp-server/EMPTY_SERVERS_FIFO"
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
