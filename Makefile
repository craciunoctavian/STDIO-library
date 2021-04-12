CC = gcc
CFLAGS = -fPIC -g -Wall

all: build

build: so_stdio.o
	$(CC) -shared $^ -o libso_stdio.so

so_stdio.o: so_stdio.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf *.o *.so
