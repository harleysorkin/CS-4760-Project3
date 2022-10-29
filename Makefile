CC = gcc
CFLAGS = -c

all: oss worker

oss: oss.o
	$(CC) -o oss oss.o

worker: worker.o
	$(CC) -o worker worker.o

oss.o: oss.c
	$(CC) $(CFLAGS) oss.c

worker.o: worker.c
	$(CC) $(CFLAGS) worker.c

.PHONY: clean
clean:
	/bin/rm -f *.o oss worker
