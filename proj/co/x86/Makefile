CC=gcc
LCC_LIB= -lpthread -lrt

.SILENT: all

all: test app

test:
	$(CC) -o test test.c -lpthread
app:
	$(CC) -o app main.c $(LCC_LIB)
clean:
	rm test app
