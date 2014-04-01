SRC=main.c drawcircle.c
EXE=dotdemo
CC=gcc

all:
	$(CC) $(SRC) -o $(EXE) `pkg-config --libs --cflags opencv`

clean:
	rm -f $(EXE)
