CXX=g++
RM=del

CXXFLAGS=-Wall

ifneq (${STATIC},)
CXXFLAGS:=${CXXFLAGS} --static
endif

ifneq (${DEBUG},)
CXXFLAGS:=${CXXFLAGS} -ggdb
else
CXXFLAGS:=${CXXFLAGS} -O2
endif

all: lisp

main.o: main.cpp
	${CXX} -c -o main.o main.cpp ${CXXFLAGS}

lisp: main.o
	${CXX} -o lisp main.o ${CXXFLAGS}

.PHONY: clean

clean:
	${RM} *.o lisp.exe
