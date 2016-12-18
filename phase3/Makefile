CC = g++
DEBUG = -ggdb
FLEX = flex
BISON = bison
WINDOWS = --nounistd --never-interactive


all: scanner.cpp SymbolTable.cpp parser.cpp alpha.cpp
	$(CC) $(DEBUG) -std=c++11 -o al scanner.cpp parser.cpp SymbolTable.cpp alpha.cpp

scanner.cpp: scanner.l parser.cpp
	$(FLEX) $(WINDOWS) --outfile=scanner.cpp scanner.l

parser.cpp: parser.y SymbolTable.h alpha.h
	$(BISON) -d -v -y --output=parser.cpp parser.y

clean:
	rm -rf al parser.cpp scanner.cpp parser.hpp parser.output