OBJ=lmice.o result.o 
GDEP=lmice.h liblmice.h

PREFIX=/usr
PLUGINDIR=liblmice
INSTDIR=$(PREFIX)/lib/$(PLUGINDIR)

BIN=liblmice.so
INC=liblmice.h

DEFS=-DPLUGINDIR=\"$(INSTDIR)\"
CFLAGS = -Wall -pipe -O2 -g -pthread -W -std=c99  $(DEFS)
LIBS = -shared -lm -ldl

CC = gcc 
LD = ld

ifdef DEBUG
   CFLAGS += -DDEBUG=true
endif

export INSTDIR

%.o: %.c %.h liblmice.h lmice.h
	$(CC) $(CFLAGS) -o $@ -c $< 

all: recursive $(OBJ) $(GDEP)
	$(LD) -o $(BIN) $(OBJ) $(LIBS)

doc:
	doxygen Doxyfile

dir:
	echo $(PREFIX)/lib/$(PLUGINDIR)/

dist: strip install
	@echo "Done"

strip: recursive
	strip $(BIN)
	
install: recursive 
	cp $(BIN) $(PREFIX)/lib
	cp $(INC) $(PREFIX)/include

uninstall: recursive
	rm $(PREFIX)/lib/$(BIN)
	rm $(PREFIX)/include/$(INC)

clean: recursive
	@rm *.o $(BIN)

clean_doc:
	@rm -r html latex rtf xml

	
recursive:
	@cd plugins; make $(MAKECMDGOALS) ;cd ..
	
#speed dev.
run: all install
	cd ../testlib/ ; make ; 	./zmouse
