IDIR = ../inc
CC = g++
CFLAGS = -I$(IDIR) -lssh
COMMAND = webttyd

ODIR = obj

_DEPS = logger.h socket_helper.h session.h daemon.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = logger.o socket_helper.o session.o daemon.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(COMMAND): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(COMMAND) $(IDIR)/*~
