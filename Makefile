CC = g++
OBJ = main.o command.o consumer.o
CFLAGS= -O -Wall -pthread

prodcon: $(OBJ)
	$(CC) $(CFLAGS) -o prodcon $(OBJ)
main.o: 
	$(CC) $(CFLAGS) -c main.cpp
command.o:
	$(CC) $(CFLAGS) -c command.cpp
consumer.o:
	$(CC) $(CFLAGS) -c consumer.cpp
clean:
	rm -f prodcon $(OBJ)