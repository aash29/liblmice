OBJ=main.o nc.o

BIN=zmouse
TEST=test

CFLAGS = -Wall -pipe -o0 -g -pthread -W -std=c99 
LIBS = -pthread /usr/lib/liblmice.so -lncurses -ldl 

CC = gcc 

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $< 

all: $(OBJ) test.o
	$(CC) -o $(BIN) $(OBJ) $(LIBS)
	$(CC) -o $(TEST) test.o $(LIBS)

clean:
	rm *.o $(BIN) $(TEST)
