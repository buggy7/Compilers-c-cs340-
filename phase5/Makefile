CC = g++
DEBUG = -ggdb

all: main.cpp VirtualMachine.h VirtualMachine.cpp VirtualMachineMemory.h VirtualMachineMemory.cpp
	$(CC) $(DEBUG) -std=c++11 -o vm main.cpp VirtualMachine.h VirtualMachine.cpp VirtualMachineMemory.h VirtualMachineMemory.cpp

clean:
	rm -rf vm
