CC = g++
DEBUG = -g
FLEX = flex


all: al.cpp
	$(CC) $(DEBUG) -o al al.cpp

al.cpp: scanner.l
	$(FLEX) --outfile=al.cpp scanner.l

clean:
	rm -rf al al.cpp