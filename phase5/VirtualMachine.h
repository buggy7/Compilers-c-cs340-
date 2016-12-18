#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include "VirtualMachineMemory.h"
#include <map>
#include <cmath>

class VirtualMachine {
private:
#define AVM_ENDING_PC codeSize
#define AVM_NUMACTUALS_OFFSET +4
#define AVM_SAVEDPC_OFFSET +3
#define AVM_SAVEDTOP_OFFSET +2
#define AVM_SAVEDTOPSP_OFFSET +1
#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic
#define execute_jle execute_cmp
#define execute_jge execute_cmp
#define execute_jlt execute_cmp
#define execute_jgt execute_cmp

	const static uint AVM_STACKENV_SIZE = 4;
	const static uint AVM_MAX_INSTRUCTIONS = nop_v + 1;
	typedef void (VirtualMachine::*execute_func_t)(instruction*);
	typedef void (VirtualMachine::*library_func_t)();
	typedef string(VirtualMachine::*tostring_func_t)(avm_memcell*);
	typedef double (VirtualMachine::*arithmetic_func_t)(double, double);
	typedef bool (VirtualMachine::*cmp_func_t)(double, double);
	typedef bool (VirtualMachine::*tobool_func_t)(avm_memcell*);

	VirtualMachineMemory* memory;
	map<string, library_func_t> libfuncs;

	uint pc = 0;
	uint currLine = 0;
	uint codeSize = 0;
	uint totalActuals = 0;
	string typeStrings[8];
	
	execute_func_t executeFuncs[AVM_MAX_INSTRUCTIONS];
	tostring_func_t tostringFuncs[8];	
	arithmetic_func_t arithmeticFuncs[5];	
	cmp_func_t cmpFuncs[4];	
	tobool_func_t toboolFuncs[8];

	void avm_warning(string s);
	void avm_error(string s);
	string avm_tostring(avm_memcell*);
	bool avm_tobool(avm_memcell*);
	void avm_assign(avm_memcell* lv, avm_memcell* rv);
	void avm_calllibfunc(string funcName);
	void avm_callsaveenvironment();
	uint avm_get_envvalue(uint i);
	library_func_t avm_getlibraryfunc(string id);
	userfunc* avm_getfuncinfo(uint address);
	uint avm_totalactuals();
	avm_memcell* avm_getactual(uint i);
	void avm_registerlibfunc(string id, library_func_t addr);

	bool jle_impl(double x, double y);
	bool jge_impl(double x, double y);
	bool jlt_impl(double x, double y);
	bool jgt_impl(double x, double y);

	bool number_tobool(avm_memcell*);
	bool string_tobool(avm_memcell*);
	bool bool_tobool(avm_memcell*);
	bool table_tobool(avm_memcell*);
	bool userfunc_tobool(avm_memcell*);
	bool libfunc_tobool(avm_memcell*);
	bool nil_tobool(avm_memcell*);
	bool undef_tobool(avm_memcell*);

	string number_tostring(avm_memcell*);
	string string_tostring(avm_memcell*);
	string bool_tostring(avm_memcell*);
	string table_tostring(avm_memcell*);
	string userfunc_tostring(avm_memcell*);
	string libfunc_tostring(avm_memcell*);
	string nil_tostring(avm_memcell*);
	string undef_tostring(avm_memcell*);

	double add_impl(double x, double y);
	double sub_impl(double x, double y);
	double mul_impl(double x, double y);
	double div_impl(double x, double y);
	double mod_impl(double x, double y);

	void execute_assign(instruction*);
	void execute_arithmetic(instruction*);
	void execute_jeq(instruction*);
	void execute_jne(instruction*);
	void execute_cmp(instruction*);
	void execute_jump(instruction*);
	void execute_call(instruction*);
	void execute_pusharg(instruction*);
	void execute_funcenter(instruction*);
	void execute_funcexit(instruction*);
	void execute_newtable(instruction*);
	void execute_tablegetelem(instruction*);
	void execute_tablesetelem(instruction*);
	void execute_nop(instruction*);

	void libfunc_print();
	void libfunc_typeof();
	void libfunc_totalarguments();
    void libfunc_sqrt();
    void libfunc_argument();
	void libfunc_sin();
	void libfunc_cos();

	void avm_push_envvalue(uint val);
	void avm_dec_top();
public:
	bool executionFinished = false;

	VirtualMachine(string file);
	~VirtualMachine();

	avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg);
	void execute_cycle();



};

#endif