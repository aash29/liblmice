OBJ=main.o 

BIN=setres
PREFIX=/usr/bin

CFLAGS = -Wall -pipe -o2 -g -pthread -W  
LIBS = -lusb 

CC = gcc 

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $< 

all: $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LIBS)

install:
	cp $(BIN) $(PREFIX)/$(BIN) 

uninstall:
	rm $(PREFIX)/$(BIN)

clean:
	rm *.o $(BIN)
