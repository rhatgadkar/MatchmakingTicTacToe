OBJS = matchserver.o connection.o db-accessor.o queue.o
CC = gcc
CFLAGS = -c -I/usr/include/postgresql
LFLAGS = -L/usr/lib/postgresql/9.3/lib

matchserver : $(OBJS)
	$(CC) $(LFLAGS) -o matchserver $(OBJS) -lpq -lpthread

matchserver.o : matchserver.c connection.h
	$(CC) $(CFLAGS) matchserver.c

connection.o : connection.c connection.h db-accessor.h
	$(CC) $(CFLAGS) connection.c

db-accessor.o : db-accessor.c db-accessor.h
	$(CC) $(CFLAGS) db-accessor.c

queue.o : queue.c connection.h queue.h
	$(CC) $(CFLAGS) queue.c

clean:
	rm *.o matchserver fifo
