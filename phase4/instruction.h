#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

typedef unsigned int uint;
using namespace std;

enum vmopcode {
	assign_v, add_v, sub_v, mul_v, div_v, mod_v, //uminus_v, and_v, or_v, not_v, 
	jeq_v, jne_v, jle_v, jge_v, jlt_v, jgt_v, jump_v, call_v, pusharg_v,
	funcenter_v, funcexit_v, newtable_v, tablegetelem_v, tablesetelem_v, nop_v
};

enum vmarg_t{
	label_a, global_a, formal_a, local_a, number_a, string_a, bool_a, nil_a, userfunc_a, libfunc_a, retval_a,undef_a
};

const static string vmarg_names[] = { "label_a", "global_a", "formal_a", "local_a", "number_a",
                                      "string_a", "bool_a", "nil_a", "userfunc_a", "libfunc_a", "retval_a","UNDEF" };

const static string vmopcode_names[] = { "assign_v", "add_v", "sub_v", "mul_v", "div_v", "mod_v", //uminus_v, and_v, or_v, not_v,
                                         "jeq_v", "jne_v", "jle_v", "jge_v", "jlt_v", "jgt_v", "jump_v", "call_v", "pusharg_v",
                                         "funcenter_v", "funcexit_v", "newtable_v", "tablegetelem_v", "tablesetelem_v", "nop_v" };

struct vmarg{
	vmarg_t type;
	uint value;
};

class instruction{
public:
    vmopcode opcode;
    vmarg arg1;
    vmarg arg2;
    vmarg result;
    uint srcLine;

    instruction(){
        arg1.type = undef_a;
        arg1.value = 0;
        arg2.value=0;
        result.value=0;
        arg2.type = undef_a;
        result.type = undef_a;
    }

};

struct userfunc{
	uint address;
	uint localSize;
	string id;
};

#endif
