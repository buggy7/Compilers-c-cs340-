#include "VirtualMachine.h"


int main(int argc, char** argv){
	string file;
	if (argc > 1){
		file = argv[1];
	}
	else file = "binary";

	VirtualMachine* vm = new VirtualMachine(file);
	while (!vm->executionFinished){
		vm->execute_cycle();
	}


	return 0;
}