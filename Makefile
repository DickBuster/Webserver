CC = gcc
CFLAGS = -g
TARGET = server
server: main.o request.o queue.o 
		$(CC) $(CFLAGS) -o server main.o request.o queue.o 
%.o : %.c
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@