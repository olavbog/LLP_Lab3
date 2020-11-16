#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#
CC = arm-cortexm3-uclinuxeabi-gcc
CFLAGS += -O2 -std=c99 -g -lm -Wall #-Wsign-compare -Wfloat-equal -Wformat-security #-Werror
CPPFLAGS += 
LDFLAGS += -lm


all: game

game: $(OBJ)
	${CC} ${CFLAGS} game.c display_functions.c $(OBJ) -o game ${LDFLAGS}
clean:
	-rm -f game

install:


.PHONY: all install clean

# end of development makefile
