CC = g++
DEBUG = -ggdb
FLEX = flex
BISON = bison



all: scanner.cpp SymbolTable.cpp parser.cpp
	$(CC) $(DEBUG) -std=c++11 -o al scanner.cpp parser.cpp SymbolTable.cpp

scanner.cpp: scanner.l parser.cpp
	$(FLEX) --outfile=scanner.cpp scanner.l

parser.cpp: parser.y SymbolTable.h
	$(BISON) -d -v -y --output=parser.cpp parser.y

clean:
	rm -rf al parser.cpp scanner.cpp parser.hpp parser.output