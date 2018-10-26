CC=gcc
CFLAGS=-O3 -Wall
LIBS=-lncurses

CFILES=nInvaders.c
HFILES=nInvaders.h
OFILES=nInvaders.o
all:		nInvaders

nInvaders: 	$(OFILES) $(HFILES)
		$(CC) $(LDFLAGS) -o$@ $(OFILES) $(LIBS)

.c.o:
		$(CC) -c  -I. $(CFLAGS) $(OPTIONS) $<
clean:
		rm -f nInvaders $(OFILES)
