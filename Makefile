SOURCES=$(wildcard *.cpp)
HEADERS=$(SOURCES:.cpp=.h)
FLAGS=-DDEBUG -g
#FLAGS=-g

all: main tags

main: $(SOURCES) $(HEADERS) Makefile
	mpiCC $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main a.out

tags: ${SOURCES} ${HEADERS}
	ctags -R .

run: main Makefile tags
	mpirun -oversubscribe -np 4 ./main
