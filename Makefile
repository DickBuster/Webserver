CC = gcc
CFLAGS = -g
TARGET = all
OBJS = main.o request.o queue.o multiply.o
all: multiply.cgi server


server: main.o request.o queue.o 
		$(CC) $(CFLAGS) -o server main.o request.o queue.o 
multiply.cgi: multiply.o 
		$(CC) $(CFLAGS) -o contentdir/multiply.cgi multiply.o
%.o : %.c
		$(CC) -c $(CFLAGS) $< -o $@

clean:
		rm -f ${OBJS} server contentdir/multiply.cgi
