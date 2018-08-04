OBJS = main.o db-accessor.o exceptions.o read_named_pipe.o utilities.o \
       write_named_pipe.o
CC = g++
CFLAGS = -c -I/usr/include/postgresql -std=c++11
LFLAGS = -L/usr/lib/postgresql/9.3/lib

a.out : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -lpq

main.o : main.cpp ../constants.h ../read_named_pipe.h ../write_named_pipe.h \
		../db-accessor.h ../exceptions.h ../utilities.h
	$(CC) $(CFLAGS) main.cpp

db-accessor.o : ../db-accessor.cpp ../db-accessor.h ../utilities.h
	$(CC) $(CFLAGS) ../db-accessor.cpp

exceptions.o : ../exceptions.cpp ../exceptions.h
	$(CC) $(CFLAGS) ../exceptions.cpp

read_named_pipe.o : ../read_named_pipe.cpp ../read_named_pipe.h \
		../named_pipe.h ../constants.h ../exceptions.h
	$(CC) $(CFLAGS) ../read_named_pipe.cpp

utilities.o : ../utilities.cpp ../utilities.h
	$(CC) $(CFLAGS) ../utilities.cpp

write_named_pipe.o : ../write_named_pipe.cpp ../write_named_pipe.h \
		../named_pipe.h ../constants.h ../exceptions.h
	$(CC) $(CFLAGS) ../write_named_pipe.cpp

clean:
	rm a.out *.o ../FREE_PORT_FIFO
