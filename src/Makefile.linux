CC      =gcc
PROF    =-O0 -g3 -DNVADERS_IN_FED #      -pg -DUSE_MCHECK
LIBS    =-lncurses #-lgd
L_FLAGS =$(PROF) -ggdb3 -o0  -D_GNU_SOURCE
O_FLAGS =$(PROF)
C_FLAGS =-Wall -Wstrict-prototypes -Wpointer-arith -Wno-char-subscripts -Wmissing-declarations -Wmissing-prototypes -Wshadow -Wformat=2 -Waggregate-return  \
         -Winline $(O_FLAGS) # -DLARGEFILE_SOURCE=1

.SUFFIXES:
.SUFFIXES: .c .o

O_FILES = b64.o cli.o clipboard.o config.o dashboard.o display.o edit.o errors.o lang.o gbuffer.o graphics.o help.o input.o io.o main.o plugin.o strings.o timer.o undo.o nInvaders.o

fed.new: $(O_FILES)
	$(CC) $(L_FLAGS) -o fed $(O_FILES) $(LIBS)
#	$(CC) $(L_FLAGS) -o fed *.o

.c.o: fed.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f *.o fed
