#ifndef VIRTUALMACHINEMEMORY_H
#define VIRTUALMACHINEMEMORY_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
//#define NDEBUG
#include <assert.h>


typedef unsigned int uint;
using namespace std;

enum vmopcode {
	assign_v, add_v, sub_v, mul_v, div_v, mod_v, //uminus_v, and_v, or_v, not_v, 
	jeq_v, jne_v, jle_v, jge_v, jlt_v, jgt_v, jump_v, call_v, pusharg_v,
	funcenter_v, funcexit_v, newtable_v, tablegetelem_v, tablesetelem_v, nop_v
};

enum vmarg_t{
	label_a, global_a, formal_a, local_a, number_a, string_a, bool_a, nil_a, userfunc_a, libfunc_a, retval_a
};

struct vmarg{
	vmarg_t type;
	uint value;
};

struct instruction{
	vmopcode opcode;
	vmarg arg1;
	vmarg arg2;
	vmarg result;
	uint srcLine;
};

struct userfunc{
	uint address;
	uint localSize;
	string id;
};

enum avm_memcell_t{
	number_m, string_m, bool_m, table_m, userfunc_m, libfunc_m, nil_m, undef_m
};

class avm_table;
struct avm_memcell{
	avm_memcell_t type;
	union {
		double numVal;
		string* strVal;
		bool boolVal;
		avm_table* tableVal;
		uint funcVal;
		string* libfuncVal;
	} data;
};

struct avm_table_bucket{
	avm_memcell key;
	avm_memcell value;
	avm_table_bucket* next;
};

class avm_memcellclear{
	typedef void(avm_memcellclear::*memclear_func_t)(avm_memcell*);
private:
	void memclear_string(avm_memcell*);
	void memclear_table(avm_memcell*);

public:
	memclear_func_t memclearFuncs[8];

	avm_memcellclear(avm_memcell* mem);

};

class avm_table{
private:
	const static int AVM_TABLE_HASHSIZE = 211;

    typedef int (avm_table::*hash_func_t)(avm_memcell*);
    typedef bool (avm_table::*compare_func_t)(avm_memcell*, avm_memcell*);

    hash_func_t hashFuncs[undef_m + 1];
    compare_func_t compareFuncs[undef_m + 1];

    int number_hashimpl(avm_memcell*);
    int string_hashimpl(avm_memcell*);
    int bool_hashimpl(avm_memcell*);
    int table_hashimpl(avm_memcell*);
    int userfunc_hashimpl(avm_memcell*);
    int libfunc_hashimpl(avm_memcell*);
    int nil_hashimpl(avm_memcell*);
    int undef_hashimpl(avm_memcell*);

    bool number_compareimpl(avm_memcell*, avm_memcell*);
    bool string_compareimpl(avm_memcell*, avm_memcell*);
    bool bool_compareimpl(avm_memcell*, avm_memcell*);
    bool table_compareimpl(avm_memcell*, avm_memcell*);
    bool userfunc_compareimpl(avm_memcell*, avm_memcell*);
    bool libfunc_compareimpl(avm_memcell*, avm_memcell*);
    bool nil_compareimpl(avm_memcell*, avm_memcell*);
    bool undef_compareimpl(avm_memcell*, avm_memcell*);

    void delete_elem(avm_memcell* key, int hash);

public:
	uint refCounter;
	avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket* numIndexed[AVM_TABLE_HASHSIZE];
	uint totalStr;
	uint totalNum;
	uint totalActuals = 0;

	avm_table();
	~avm_table();
	void avm_tablebucketsdestroy(avm_table_bucket** p);
	avm_memcell* avm_tablegetelem(avm_memcell* key);
	void avm_tablesetelem(avm_memcell* key, avm_memcell* value);
	void avm_tablebucketsinit(avm_table_bucket** p);
	inline void avm_tableincrefcounter() { refCounter++; }
	inline void avm_tabledecrefcounter() { if (--refCounter <= 0) delete this; }
};

class VirtualMachineMemory {
private:
	ifstream file;
	uint globalCount = 0;

	void avm_initstack();
	void avm_wipeout(avm_memcell* t);

	void loadFile(string f);
	double readDouble();
	uint readNum();
	string readString();

public:
	vector<instruction> instructions;
	vector<double> numConsts;
	vector<string> stringConsts;
	vector<string> namedLibFuncs;
	vector<userfunc> userFuncs;

	const static int AVM_STACKSIZE = 4096;
	avm_memcell stack[AVM_STACKSIZE];

	avm_memcell ax, bx, cx;
	avm_memcell retval;
	uint top, topsp;

	//VirtualMachineMemory();
	VirtualMachineMemory(string file);
	~VirtualMachineMemory();

	inline double consts_getNumber(uint index){ return numConsts[index]; }
	inline string consts_getString(uint index){ return stringConsts[index]; }
	inline string libfuncs_getUsed(uint index){ return namedLibFuncs[index]; }
};

#endif