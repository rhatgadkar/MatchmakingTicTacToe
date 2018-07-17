OBJS = child_server.o main.o server_child_connection.o db-accessor.o \
       exceptions.o read_named_pipe.o utilities.o write_named_pipe.o
CC = g++
CFLAGS = -c -I/usr/include/postgresql -std=c++11
LFLAGS = -L/usr/lib/postgresql/9.3/lib

a.out : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -lpq -lpthread

child_server.o : child_server.cpp child_server.h ../write_named_pipe.h \
		../utilities.h ../constants.h ../db-accessor.h \
		../exceptions.h
	$(CC) $(CFLAGS) child_server.cpp

main.o : main.cpp child_server.h server_child_connection.h ../utilities.h
	$(CC) $(CFLAGS) main.cpp

server_child_connection.o : server_child_connection.cpp \
		server_child_connection.h ../utilities.h ../constants.h \
		../exceptions.h
	$(CC) $(CFLAGS) server_child_connection.cpp

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
	rm *.o
