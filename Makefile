OBJ = p2.o
SOURCE = p2.cpp
CC = g++-4.9
DEBUG = -g
CFLAGS =  -Wall -std=c++11 -ltbb -c -Wl,-rpath=/usr/local/gcc481/lib64 $(DEBUG)
LFLAGS =  -Wall -std=c++11 -ltbb -Wl,-rpath=/usr/local/gcc481/lib64 $(DEBUG)

p2: $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) -o p2

p2.o: $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE)

clean:
	\rm *.o *~ p2
