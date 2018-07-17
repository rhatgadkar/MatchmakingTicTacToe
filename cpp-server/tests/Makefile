OBJS = main.o child_server.o mock_child_connection.o \
       server_child_connection.o mock_parent_connection.o parent_server.o \
       server_parent_connection.o db-accessor.o exceptions.o \
       read_named_pipe.o utilities.o write_named_pipe.o
CC = g++
CFLAGS = -c -I/usr/include/postgresql -std=c++11
LFLAGS = -L/usr/lib/postgresql/9.3/lib

a.out : $(OBJS)
	$(CC) $(OBJS) -lpq -lpthread

main.o : main.cpp ../parent-server/mock_parent_connection.h \
		../parent-server/parent_server.h \
		../child-server/mock_child_connection.h \
		../child-server/child_server.h ../write_named_pipe.h \
		../constants.h ../utilities.h
	$(CC) $(CFLAGS) main.cpp

child_server.o : ../child-server/child_server.cpp \
		../child-server/child_server.h ../write_named_pipe.h \
		../utilities.h ../constants.h ../db-accessor.h ../exceptions.h
	$(CC) $(CFLAGS) ../child-server/child_server.cpp

mock_child_connection.o : ../child-server/mock_child_connection.cpp \
		../child-server/mock_child_connection.h ../exceptions.h
	$(CC) $(CFLAGS) ../child-server/mock_child_connection.cpp

server_child_connection.o : ../child-server/server_child_connection.cpp \
		../child-server/server_child_connection.h ../utilities.h \
		../constants.h ../exceptions.h
	$(CC) $(CFLAGS) ../child-server/server_child_connection.cpp

mock_parent_connection.o : ../parent-server/mock_parent_connection.cpp \
		../parent-server/mock_parent_connection.h ../exceptions.h
	$(CC) $(CFLAGS) ../parent-server/mock_parent_connection.cpp

parent_server.o : ../parent-server/parent_server.cpp \
		../parent-server/parent_server.h \
		../parent-server/parent_connection.h \
		../child-server/child_server.h \
		../child-server/server_child_connection.h ../utilities.h \
		../constants.h ../exceptions.h ../read_named_pipe.h \
		../write_named_pipe.h ../db-accessor.h
	$(CC) $(CFLAGS) ../parent-server/parent_server.cpp

server_parent_connection.o : ../parent-server/server_parent_connection.cpp \
		../parent-server/server_parent_connection.h ../utilities.h \
		../constants.h ../exceptions.h
	$(CC) $(CFLAGS) ../parent-server/server_parent_connection.cpp

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
	rm *.o ../EMPTY_SERVERS_FIFO ../FREE_PORT_FIFO ../WAITING_SERVERS_FIFO
