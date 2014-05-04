GDEP=plmouse.h 

BIN=dummy.so devfs.so libusb.so

CFLAGS = -Wall -pipe -O2 -g -pthread -W -rdynamic

ifdef DEBUG
   CFLAGS += -DDEBUG
endif

all: plmouse.o $(BIN) $(GDEP) 

plmouse.o: plmouse.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $< 

%.so: %.o
	$(LD) -shared -lusb -o $@ $< plmouse.o


doc:
	doxygen Doxyfile

strip:
	strip $(BIN)

dist: strip install
	@echo "Installed stripped plugins"

install: 
	@mkdirhier $(INSTDIR)
	cp $(BIN) $(INSTDIR)/

uninstall:
	cd $(INSTDIR) ; rm $(BIN)
	rmdir $(INSTDIR)

clean:
	@rm $(BIN) plmouse.o

#speed dev.
run: all install
	cd ../testlib/ ; make ; 	./zmouse
