SRC= ddemo_helpers.c main.c ddclientlib/ddclient.c ddclientlib/ddhelpers.c
LIBS=-lSDL2_gfx -lpthread -lm
PKG_CFG_LIBS=sdl2
EXE=dotdemo
CC=gcc


.PHONY: all

all: prereq
	$(CC) $(SRC) -o $(EXE) `pkg-config --libs --cflags $(PKG_CFG_LIBS)` $(LIBS)

debug: prereq
	$(CC) $(SRC) -o $(EXE) -g `pkg-config --libs --cflags $(PKG_CFG_LIBS)` $(LIBS)

prereq:
	@if ! [ -d ddclientlib ]; then echo "Missing ./ddclientlib. Add it with 'ln -s /path/to/dotdetector/ddclientlib/ .'"; exit 1; fi

clean:
	rm -f $(EXE)
