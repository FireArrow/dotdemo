SRC=main.c ddclientlib/ddclient.c
LIBS=-lSDL2_gfx
EXE=dotdemo
CC=gcc

all:
	$(CC) $(SRC) -o $(EXE) `pkg-config --libs --cflags sdl2` $(LIBS)

clean:
	rm -f $(EXE)
