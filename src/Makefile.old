IDIR = ../inc
CC = gcc
CFLAGS = -I$(IDIR) -lssh

ODIR = obj

_DEPS = verify_knownhost.h execute_remote_cmd.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = verify_knownhost.o execute_remote_cmd.o webttyd.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

webttyd: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
