CC = gcc
ECHO = echo
CFLAGS =  -c -g -Wall
MAPFLAG = -DMAP
SRC = $(wildcard *.c)
HDR = $(wildcard *.h)
OBJ = $(SRC:.c=.o)
EXE = awale

all: $(SRC) $(OBJ) $(EXE)

$(EXE): $(OBJ)
	@echo "\n\e[0;35m\033[1mEdition des liens\033[0m"
	$(CC) $(CLAGS) $^ -o $@

%.o : %.c $(HDR)
	@echo "\e[1;33m\033[1mCompilation de" $< "\033[0m"
	$(CC)   $(CFLAGS) $< -o $@ $(MAPFLAG)

clean:
	rm *.o $(EXE) 
