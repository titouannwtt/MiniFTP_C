CFLAGS=-std=c99 -Wall -pedantic

server: mftp.o
	gcc mftp.o -o server
	gcc mftp.o -o client

mftp.o: mftp.c
	gcc -c $(CFLAGS) mftp.c

clean:
		rm mftp *.o
