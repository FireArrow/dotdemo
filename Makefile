SRC=main.c ddclientlib/ddclient.c ddclientlib/ddhelpers.c
LIBS=-lSDL2_gfx
PKG_CFG_LIBS=sdl2
EXE=dotdemo
CC=gcc


.PHONY: all

all:
	@if ! [ -d ddclientlib ]; then echo "Missing ./ddclientlib. Add it with 'ln -s /path/to/dotdetector/ddclientlib/ .'"; exit 1; fi
	$(CC) $(SRC) -o $(EXE) -lm `pkg-config --libs --cflags $(PKG_CFG_LIBS)` $(LIBS)

clean:
	rm -f $(EXE)
