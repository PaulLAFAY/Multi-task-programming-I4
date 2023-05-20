CC = gcc
EXEC = ex1.exe
SRC_PATH = ../src/
SRC = $(wildcard $(SRC_PATH)*.c)
OBJ = $(SRC:.c=.o)
LIBS = -lglut -lGL -lm -lpthread

%$(SRC_PATH).o : %$(SRC_PATH).c
	$(CC) -o $(OBJ) $(SRC)

$(EXEC) : $(OBJ)
	$(CC) -o $(EXEC) $(OBJ) $(LIBS)

clear :
	rm $(EXEC) $(SRC_PATH)*.o