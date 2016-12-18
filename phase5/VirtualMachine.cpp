#include "VirtualMachine.h"

VirtualMachine::VirtualMachine(string file){
	memory = new VirtualMachineMemory(file);
	codeSize = (uint)memory->instructions.size();

	avm_registerlibfunc("print", &VirtualMachine::libfunc_print);
	avm_registerlibfunc("typeof", &VirtualMachine::libfunc_typeof);
	avm_registerlibfunc("totalarguments", &VirtualMachine::libfunc_totalarguments);
    avm_registerlibfunc("sqrt", &VirtualMachine::libfunc_sqrt);
    avm_registerlibfunc("argument", &VirtualMachine::libfunc_argument);
	avm_registerlibfunc("sin", &VirtualMachine::libfunc_sin);
	avm_registerlibfunc("cos", &VirtualMachine::libfunc_cos);

	int i = 0;
	executeFuncs[i++]= &VirtualMachine::execute_assign;
	executeFuncs[i++]= &VirtualMachine::execute_add;
	executeFuncs[i++]= &VirtualMachine::execute_sub;
	executeFuncs[i++]= &VirtualMachine::execute_mul;
	executeFuncs[i++]= &VirtualMachine::execute_div;
	executeFuncs[i++]= &VirtualMachine::execute_mod;
	//executeFuncs[i++]= &VirtualMachine::execute_minus;
	//executeFuncs[i++]= &VirtualMachine::execute_and;
	//executeFuncs[i++]= &VirtualMachine::execute_or;
	//executeFuncs[i++]= &VirtualMachine::execute_not;
	executeFuncs[i++] = &VirtualMachine::execute_jeq;
	executeFuncs[i++] = &VirtualMachine::execute_jne;
	executeFuncs[i++] = &VirtualMachine::execute_jle;
	executeFuncs[i++] = &VirtualMachine::execute_jge;
	executeFuncs[i++] = &VirtualMachine::execute_jlt;
	executeFuncs[i++] = &VirtualMachine::execute_jgt;
	executeFuncs[i++] = &VirtualMachine::execute_jump;
	executeFuncs[i++] = &VirtualMachine::execute_call;
	executeFuncs[i++] = &VirtualMachine::execute_pusharg;
	executeFuncs[i++] = &VirtualMachine::execute_funcenter;
	executeFuncs[i++] = &VirtualMachine::execute_funcexit;
	executeFuncs[i++] = &VirtualMachine::execute_newtable;
	executeFuncs[i++] = &VirtualMachine::execute_tablegetelem;
	executeFuncs[i++] = &VirtualMachine::execute_tablesetelem;
	executeFuncs[i++] = &VirtualMachine::execute_nop;

	i = 0;
	typeStrings[i++] = "number";
	typeStrings[i++] = "string";
	typeStrings[i++] = "bool";
	typeStrings[i++] = "table";
	typeStrings[i++] = "userfunc";
	typeStrings[i++] = "libfunc";
	typeStrings[i++] = "nil";
	typeStrings[i++] = "undef";

	i = 0;
	tostringFuncs[i++] = &VirtualMachine::number_tostring;
	tostringFuncs[i++] = &VirtualMachine::string_tostring;
	tostringFuncs[i++] = &VirtualMachine::bool_tostring;
	tostringFuncs[i++] = &VirtualMachine::table_tostring;
	tostringFuncs[i++] = &VirtualMachine::userfunc_tostring;
	tostringFuncs[i++] = &VirtualMachine::libfunc_tostring;
	tostringFuncs[i++] = &VirtualMachine::nil_tostring;
	tostringFuncs[i++] = &VirtualMachine::undef_tostring;

	i = 0;
	arithmeticFuncs[i++] = &VirtualMachine::add_impl;
	arithmeticFuncs[i++] = &VirtualMachine::sub_impl;
	arithmeticFuncs[i++] = &VirtualMachine::mul_impl;
	arithmeticFuncs[i++] = &VirtualMachine::div_impl;
	arithmeticFuncs[i++] = &VirtualMachine::mod_impl;

	i = 0;
	cmpFuncs[i++] = &VirtualMachine::jle_impl;
	cmpFuncs[i++] = &VirtualMachine::jge_impl;
	cmpFuncs[i++] = &VirtualMachine::jlt_impl;
	cmpFuncs[i++] = &VirtualMachine::jgt_impl;

	i = 0;
	toboolFuncs[i++] = &VirtualMachine::number_tobool;
	toboolFuncs[i++] = &VirtualMachine::string_tobool;
	toboolFuncs[i++] = &VirtualMachine::bool_tobool;
	toboolFuncs[i++] = &VirtualMachine::table_tobool;
	toboolFuncs[i++] = &VirtualMachine::userfunc_tobool;
	toboolFuncs[i++] = &VirtualMachine::libfunc_tobool;
	toboolFuncs[i++] = &VirtualMachine::nil_tobool;
	toboolFuncs[i++] = &VirtualMachine::undef_tobool;
}

VirtualMachine::~VirtualMachine(){

}

void VirtualMachine::avm_warning(string s){
	cout << s.c_str() << endl;
}
void VirtualMachine::avm_error(string s){
	cout << s.c_str() << endl;
	executionFinished = true;
}
string VirtualMachine::avm_tostring(avm_memcell* m){
	assert(m->type >= 0 && m->type <= undef_m);
	return (*this.*tostringFuncs[m->type])(m);
}
bool VirtualMachine::avm_tobool(avm_memcell* m){
	assert(m->type >= 0 && m->type <= undef_m);
	return (*this.*toboolFuncs[m->type])(m);
}

void VirtualMachine::avm_assign(avm_memcell* lv, avm_memcell* rv){
	if (lv == rv) return;

	if (lv->type == table_m && rv->type == table_m &&
		lv->data.tableVal == rv->data.tableVal) return;

	if (rv->type == undef_m)
		avm_warning("assigning from 'undef' content!");

	new avm_memcellclear(lv);

	lv->type = rv->type;
	lv->data = rv->data;

	if (lv->type == string_m) lv->data.strVal = new string(*rv->data.strVal);
	else if (lv->type == table_m) lv->data.tableVal->avm_tableincrefcounter();
}

void VirtualMachine::avm_callsaveenvironment(){
	avm_push_envvalue(totalActuals);
	avm_push_envvalue(pc + 1);
	avm_push_envvalue(memory->top + totalActuals + 2);
	avm_push_envvalue(memory->topsp);
}
uint VirtualMachine::avm_get_envvalue(uint i){
	assert(memory->stack[i].type == number_m);
	uint val = (uint)memory->stack[i].data.numVal;
	assert(memory->stack[i].data.numVal == (double)val);
	return val;
}

void VirtualMachine::avm_calllibfunc(string funcName){
	library_func_t f = avm_getlibraryfunc(funcName);
	if (!f){
		avm_error("unsupported lib func " + funcName + " called!");
		executionFinished = true;
	}
	else{
		memory->topsp = memory->top;
		totalActuals = 0;
		(*this.*f)();
		if (!executionFinished)
			execute_funcexit(NULL);
	}
}
VirtualMachine::library_func_t VirtualMachine::avm_getlibraryfunc(string id){
	if (libfuncs.count(id) > 0)	return libfuncs[id];
	else return NULL;
}
userfunc* VirtualMachine::avm_getfuncinfo(uint address){
	for (int i = 0; i < memory->userFuncs.size(); i++)
	{
		if (memory->userFuncs[i].address == address) return &memory->userFuncs[i];
	}
	return NULL;
}
uint VirtualMachine::avm_totalactuals(){
	return avm_get_envvalue(memory->topsp + AVM_NUMACTUALS_OFFSET);
}
avm_memcell* VirtualMachine::avm_getactual(uint i){
	assert(i < avm_totalactuals());
	return &memory->stack[memory->topsp + AVM_STACKENV_SIZE + 1 + i];
}
void VirtualMachine::avm_registerlibfunc(string id, library_func_t addr){
	libfuncs[id] = addr;
}


string VirtualMachine::number_tostring(avm_memcell* m){
	ostringstream sstream;
	sstream << m->data.numVal;
	return sstream.str();
}
string VirtualMachine::string_tostring(avm_memcell* m){ return *m->data.strVal; }
string VirtualMachine::bool_tostring(avm_memcell* m){ return m->data.boolVal ? "TRUE" : "FALSE"; }
string VirtualMachine::table_tostring(avm_memcell*){
	return "table";
}
string VirtualMachine::userfunc_tostring(avm_memcell* m){ return avm_getfuncinfo(m->data.funcVal)->id; }
string VirtualMachine::libfunc_tostring(avm_memcell* m){ return *m->data.libfuncVal; }
string VirtualMachine::nil_tostring(avm_memcell*){ return "NILL"; }
string VirtualMachine::undef_tostring(avm_memcell*){ return "undef"; }

double VirtualMachine::add_impl(double x, double y){ return x + y; }
double VirtualMachine::sub_impl(double x, double y){ return x - y; }
double VirtualMachine::mul_impl(double x, double y){ return x*y; }
double VirtualMachine::div_impl(double x, double y){ 
	if (y == 0) {
		avm_error("trying to divide by 0!");
		executionFinished = true;
		return 0;
	}
	else return x / y; 
}
double VirtualMachine::mod_impl(double x, double y){
	if (y == 0) {
		avm_error("trying to divide by 0!");
		executionFinished = true;
		return 0;
	}
	else return (uint)x % (uint)y;
}

bool VirtualMachine::jle_impl(double x, double y){ return x <= y; }
bool VirtualMachine::jge_impl(double x, double y){ return x >= y; }
bool VirtualMachine::jlt_impl(double x, double y){ return x < y; }
bool VirtualMachine::jgt_impl(double x, double y){ return x > y; }

bool VirtualMachine::number_tobool(avm_memcell* a){ return a->data.numVal != 0; }
bool VirtualMachine::string_tobool(avm_memcell* a){ return (*a->data.strVal)[0] != 0; }
bool VirtualMachine::bool_tobool(avm_memcell* a){ return a->data.boolVal; }
bool VirtualMachine::table_tobool(avm_memcell*){ return true; }
bool VirtualMachine::userfunc_tobool(avm_memcell*){ return true; }
bool VirtualMachine::libfunc_tobool(avm_memcell*){ return true; }
bool VirtualMachine::nil_tobool(avm_memcell*){ return false; }
bool VirtualMachine::undef_tobool(avm_memcell*){ assert(0); return false; }

void VirtualMachine::execute_assign(instruction* a)
{
	avm_memcell* lv = avm_translate_operand(&a->result, NULL);
	avm_memcell* rv = avm_translate_operand(&a->arg1, &(memory->ax));

	//assert(lv && (&memory->stack[N-1] >= lv && lv > &memory->stack[top] || lv == &memory->retval));
	//TODO ~18
	avm_assign(lv, rv);
}
void VirtualMachine::execute_arithmetic(instruction* a)
{
	avm_memcell* lv = avm_translate_operand(&a->result, &(memory->cx));
	avm_memcell* rv1 = avm_translate_operand(&a->arg1, &(memory->ax));
	avm_memcell* rv2 = avm_translate_operand(&a->arg2, &(memory->bx));

	//assert(lv && (&memory->stack[N-1] >= lv && lv > &memory->stack[memory->top] || lv == &memory->retval));
	assert(rv1 && rv2);

	if (rv1->type != number_m || rv2->type != number_m){
		avm_error("not a number in arithmetic!");
		executionFinished = true;
	}
	else{
		arithmetic_func_t op = arithmeticFuncs[a->opcode - add_v];
		new avm_memcellclear(lv);
		lv->type = number_m;
		lv->data.numVal = (*this.*op)(rv1->data.numVal, rv2->data.numVal);
	}
}
void VirtualMachine::execute_jeq(instruction* a)
{
	assert(a->result.type == label_a);

	avm_memcell* rv1 = avm_translate_operand(&a->arg1, &(memory->ax));
	avm_memcell* rv2 = avm_translate_operand(&a->arg2, &(memory->bx));

	bool result = false;

	if (rv1->type == undef_m || rv2->type == undef_m)
		avm_error(" undef invlolved in equality ");
	else if (rv1->type == nil_m || rv2->type == nil_m)
		result = rv1->type == nil_m && rv2->type == nil_m;
	else if (rv1->type == bool_m || rv2->type == bool_m)
		result = (avm_tobool(rv1) == avm_tobool(rv2));
	else if (rv1->type != rv2->type)
		avm_error(typeStrings[rv1->type] + " == " + typeStrings[rv2->type] + " is Illegal");
	else if (rv1->type == number_m && rv2->type == number_m)
		result = rv1->data.numVal == rv2->data.numVal;
	else if (rv1->type == string_m && rv2->type == string_m)
		result = *rv1->data.strVal == *rv2->data.strVal;
	else if ((rv1->type == table_m && rv2->type == table_m) ||
			(rv1->type == userfunc_m && rv2->type == userfunc_m) ||
			(rv1->type == libfunc_m && rv2->type == libfunc_m))
			result = true;

	if (!executionFinished && result)
		pc = a->result.value;

}
void VirtualMachine::execute_jne(instruction* a)
{
	assert(a->result.type == label_a);

	avm_memcell* rv1 = avm_translate_operand(&a->arg1, &(memory->ax));
	avm_memcell* rv2 = avm_translate_operand(&a->arg2, &(memory->bx));

	bool result = false;

	if (rv1->type == undef_m || rv2->type == undef_m)
		avm_error(" undef invlolved in equality ");
	else if (rv1->type == nil_m || rv2->type == nil_m)
		result = rv1->type == nil_m && rv2->type == nil_m;
	else if (rv1->type == bool_m || rv2->type == bool_m)
		result = (avm_tobool(rv1) != avm_tobool(rv2));
	else if (rv1->type != rv2->type)
		avm_error(typeStrings[rv1->type] + " == " + typeStrings[rv2->type] + " is Illegal");
	else if (rv1->type == number_m && rv2->type == number_m)
		result = rv1->data.numVal != rv2->data.numVal;
	else if (rv1->type == string_m && rv2->type == string_m)
		result = *rv1->data.strVal != *rv2->data.strVal;
	else if ((rv1->type == table_m && rv2->type == table_m) ||
		(rv1->type == userfunc_m && rv2->type == userfunc_m) ||
		(rv1->type == libfunc_m && rv2->type == libfunc_m))
		result = true;

	if (!executionFinished && result)
		pc = a->result.value;
}
void VirtualMachine::execute_cmp(instruction* a)
{
	avm_memcell* rv1 = avm_translate_operand(&a->arg1, &(memory->ax));
	avm_memcell* rv2 = avm_translate_operand(&a->arg2, &(memory->bx));

	//assert(lv && (&memory->stack[N-1] >= lv && lv > &memory->stack[memory->top] || lv == &memory->retval));
	assert(rv1 && rv2);

	if (rv1->type != number_m || rv2->type != number_m){
		avm_error("not a number in compare!");
		executionFinished = true;
	}
	else{
		//TODO change lv?
		cmp_func_t op = cmpFuncs[a->opcode - jle_v];
		if ((*this.*op)(rv1->data.numVal, rv2->data.numVal))
			pc = a->result.value;
	}
}

void VirtualMachine::execute_jump(instruction* a)
{
	pc = a->result.value;
}

void VirtualMachine::execute_call(instruction* a)
{
	avm_memcell * func = avm_translate_operand(&a->arg1, &(memory->ax));
	assert(func);

	avm_callsaveenvironment();

	switch (func->type){
	case userfunc_m:
		pc = func->data.funcVal;
		assert(pc < AVM_ENDING_PC);
		//assert(code[pc].opcode == funcenter_v);
		break;
	case string_m:
		avm_calllibfunc(*func->data.strVal);
		break;
	case libfunc_m:
		avm_calllibfunc(*func->data.libfuncVal);
		break;
	default:
		string s = avm_tostring(func);
		avm_error("call: cannot bind " + s + " to function!");
		executionFinished = true;
		break;
	}

}
void VirtualMachine::execute_pusharg(instruction* a)
{
	avm_memcell* arg = avm_translate_operand(&a->arg1, &(memory->ax));
	assert(arg);

	avm_assign(&memory->stack[memory->top], arg);
	++totalActuals;
	avm_dec_top();
}
void VirtualMachine::execute_funcenter(instruction* a)
{
	avm_memcell* func = avm_translate_operand(&a->result, &(memory->ax));
	assert(func);
	assert(pc == func->data.funcVal);

	totalActuals = 0;
	userfunc* funcInfo = avm_getfuncinfo(func->data.funcVal);
	memory->topsp = memory->top;
	memory->top -= funcInfo->localSize;
}
void VirtualMachine::execute_funcexit(instruction* a)
{
	uint oldTop = memory->top;
	memory->top = avm_get_envvalue(memory->topsp + AVM_SAVEDTOP_OFFSET);
	pc = avm_get_envvalue(memory->topsp + AVM_SAVEDPC_OFFSET);
	memory->topsp = avm_get_envvalue(memory->topsp + AVM_SAVEDTOPSP_OFFSET);

	while (++oldTop <= memory->top){
		new avm_memcellclear(&memory->stack[oldTop]);
	}
}
void VirtualMachine::execute_newtable(instruction* a)
{
	avm_memcell* lv = avm_translate_operand(&a->result, NULL);
	
	new avm_memcellclear(lv);

	lv->type = table_m;
	lv->data.tableVal = new avm_table();
	lv->data.tableVal->avm_tableincrefcounter();
}
void VirtualMachine::execute_tablegetelem(instruction* a)
{
	avm_memcell* lv = avm_translate_operand(&a->result, NULL);
	avm_memcell* t = avm_translate_operand(&a->arg1, NULL);
	avm_memcell* i = avm_translate_operand(&a->arg2, &memory->ax);

	//
	//
	assert(i);

	new avm_memcellclear(lv);
	lv->type = nil_m;

	if (t->type != table_m){
		avm_error("illegal use of type " + typeStrings[t->type] + " as table!");
	}
	else{
		avm_memcell* content = t->data.tableVal->avm_tablegetelem(i);
		if (content){
			avm_assign(lv, content);
		}
		else{
			avm_warning(avm_tostring(t) + "[" + avm_tostring(i) + "] not found!");
		}

	}
}
void VirtualMachine::execute_tablesetelem(instruction* a)
{
	avm_memcell* t = avm_translate_operand(&a->arg1, &memory->cx);
	avm_memcell* i = avm_translate_operand(&a->arg2, &memory->ax);
	avm_memcell* c = avm_translate_operand(&a->result, &memory->bx);

	//
	assert(i && c);

	if (t->type != table_m){
		avm_error("illegal use of type " + typeStrings[t->type] + " as table!");
	}
	else
		t->data.tableVal->avm_tablesetelem(i, c);
}
void VirtualMachine::execute_nop(instruction* a){}

void VirtualMachine::libfunc_print(){
	uint n = avm_totalactuals();
	for (int i = n-1; i >= 0; i--){
		string s = avm_tostring(avm_getactual(i));
		cout << s.c_str();
	}
}
void VirtualMachine::libfunc_typeof(){
	uint n = avm_totalactuals();

	if (n != 1){
		ostringstream sstream;
		sstream << n;
		avm_error("one argument (not " + sstream.str() + ") expected in 'typeof'!");
	}
	else{
		new avm_memcellclear(&memory->retval);
		memory->retval.type = string_m;
		memory->retval.data.strVal = new string(typeStrings[avm_getactual(0)->type]);
	}
}
void VirtualMachine::libfunc_totalarguments(){
	uint p_topsp = avm_get_envvalue(memory->topsp + AVM_SAVEDTOPSP_OFFSET);
	new avm_memcellclear(&memory->retval);

	if (!p_topsp){
		avm_error("'total arguments' called outside of function!");
		memory->retval.type = nil_m;
	}
	else{
		memory->retval.type = number_m;
		memory->retval.data.numVal = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
	}
}
void VirtualMachine::libfunc_sqrt(){
    uint n = avm_totalactuals();
    if (n != 1){
        ostringstream sstream;
        sstream << n;
        avm_error("one argument (not " + sstream.str() + ") expected in 'typeof'!");
    }

    else{
        new avm_memcellclear(&memory->retval);
        memory->retval.type = number_m;
        if( avm_getactual(0)->type != number_m)
            avm_error(" Argument of SQRT is not a number ");
        else{
            if(sqrt( avm_getactual(0)->data.numVal < 0)){
                memory->retval.type = nil_m;
            }
            else{
                memory->retval.data.numVal = sqrt( avm_getactual(0)->data.numVal);

            }
        }
    }
}
void VirtualMachine::libfunc_argument(){
    uint n = avm_totalactuals();
    if (n != 1){
        ostringstream sstream;
        sstream << n;
        avm_error("one argument (not " + sstream.str() + ") expected in 'argument'!");
    }
    else{
        uint p_topsp = avm_get_envvalue(memory->topsp + AVM_SAVEDTOPSP_OFFSET);
        uint totalArgs = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
        new avm_memcellclear(&memory->retval);

        if(avm_getactual(0)->type != number_m){
            avm_error("Library Function argument(), argument is not number ");
            return;
        }

        double i = avm_getactual(0)->data.numVal;
        if(i > totalArgs || i == 0 ){
            avm_error("Function argument, cannot access this argument");
            memory->retval.type = nil_m;
        }
        else{
            avm_assign(&memory->retval,&memory->stack[p_topsp + AVM_STACKENV_SIZE + (int)i]);
        }
    }
}

void VirtualMachine::libfunc_sin(){
	uint n = avm_totalactuals();
	if (n != 1){
		ostringstream sstream;
		sstream << n;
		avm_error("one argument (not " + sstream.str() + ") expected in 'sin'!");
	}

	else{
		new avm_memcellclear(&memory->retval);
		memory->retval.type = number_m;
		if (avm_getactual(0)->type != number_m)
			avm_error(" Argument of sin is not a number ");
		else{
			memory->retval.data.numVal = sin(avm_getactual(0)->data.numVal);
		}
	}

}

void VirtualMachine::libfunc_cos(){
	uint n = avm_totalactuals();
	if (n != 1){
		ostringstream sstream;
		sstream << n;
		avm_error("one argument (not " + sstream.str() + ") expected in 'cos'!");
	}

	else{
		new avm_memcellclear(&memory->retval);
		memory->retval.type = number_m;
		if (avm_getactual(0)->type != number_m)
			avm_error(" Argument of cos is not a number ");
		else{
			memory->retval.data.numVal = cos(avm_getactual(0)->data.numVal);
		}
	}

}

void VirtualMachine::avm_push_envvalue(uint val){
	memory->stack[memory->top].type = number_m;
	memory->stack[memory->top].data.numVal = val;
	avm_dec_top();
}
void VirtualMachine::avm_dec_top(){
	if (!memory->top){
		avm_error("stack overflow");
		executionFinished = true;
	}
	else --memory->top;
}

avm_memcell* VirtualMachine::avm_translate_operand(vmarg* arg, avm_memcell* reg){
	switch (arg->type){
	case global_a:
		return &memory->stack[memory->AVM_STACKSIZE - 1 - arg->value];
	case local_a:
		return &memory->stack[memory->topsp - arg->value];
	case formal_a:
		return &memory->stack[memory->topsp + AVM_STACKENV_SIZE + 1 + arg->value];
	case retval_a:
		return &memory->retval;
	case number_a:
		reg->type = number_m;
		reg->data.numVal = memory->consts_getNumber(arg->value);
		return reg;
	case string_a:
		reg->type = string_m;
		reg->data.strVal = new string(memory->consts_getString(arg->value));
		return reg;
	case bool_a:
		reg->type = bool_m;
		reg->data.boolVal = arg->value != 0;
		return reg;
	case nil_a:
		reg->type = nil_m;
		return reg;
	case userfunc_a:
		reg->type = userfunc_m;
		reg->data.funcVal = arg->value;
		return reg;
	case libfunc_a:
		reg->type = libfunc_m;
		reg->data.libfuncVal = new string(memory->libfuncs_getUsed(arg->value));
		return reg;
	default:
		return reg;
	}
}

void VirtualMachine::execute_cycle(){
	if (executionFinished) return;
	else if (pc == AVM_ENDING_PC){
		executionFinished = true;
		return;
	}
	else{
		if (pc >= AVM_ENDING_PC){
			cout << "pc >= END" << endl;
		}
		instruction* instr = &memory->instructions[pc];
		if (instr->opcode < 0 || instr->opcode > AVM_MAX_INSTRUCTIONS){
			cout << "ERROR: Wrong opcode" << endl;
		}
		if (instr->srcLine) currLine = instr->srcLine;
		uint oldPc = pc;

		(this->*(this->executeFuncs[instr->opcode]))(instr);
		if (pc == oldPc) pc++;

	}
}