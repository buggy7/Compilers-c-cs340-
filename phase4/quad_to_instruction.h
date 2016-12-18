#ifndef _QUAD_TO_INSTRUCTION_H
#define _QUAD_TO_INSTRUCTION_H

#pragma once

#include "alpha.h"
#include "instruction.h"

struct incomplete_jump{
    uint instrNo;
    uint iaddress;
};

class convert{
private:
    uint currentInstruction = 0;
    uint currentProcessedQuad = 0;
    vector<quad*> quads;
    vector<vector<int>> funcstack;

    void generate_op(vmopcode op, quad *);
    void generate_relational(vmopcode op, quad *q);

    void generate_ADD (quad*);
    void generate_SUB (quad*);
    void generate_MUL (quad*);
    void generate_DIV (quad*);
    void generate_MOD (quad*);
    void generate_UMINUS (quad*);
    void generate_NEWTABLE (quad*);
    void generate_TABLEGETELEM (quad*);
    void generate_TABLESETELEM (quad*);
    void generate_ASSIGN (quad*);
    void generate_NOP (quad*);
    void generate_JUMP (quad*);
    void generate_IF_EQ (quad*);
    void generate_IF_NOTEQ (quad*);
    void generate_IF_GREATER (quad*);
    void generate_IF_GREATEREQ (quad*);
    void generate_IF_LESS (quad*);
    void generate_IF_LESSEQ (quad*);
    void generate_NOT (quad*);
    void generate_AND (quad*);
    void generate_OR (quad*);
    void generate_PARAM (quad*);
    void generate_CALL (quad*);
    void generate_GETRETVAL (quad*);
    void generate_FUNCSTART (quad*);
    void generate_RETURN (quad*);
    void generate_FUNCEND (quad*);

    uint consts_newstring(string s);
    uint consts_newnumber(double n);
    uint libfuncs_newused(string s);
    uint userfuncs_newfunc(SymbolEntry* sym);

    inline void emit(instruction* q){
        //t->srcLine = q->line;
        instructions.push_back(q);
        currentInstruction++;
    }

    inline uint nextInstructionLabel(){
        return currentInstruction;
    }

    void make_numberoperand(vmarg *arg, double val);

    void make_booloperand(vmarg *arg, bool val);

    void make_retvaloperand(vmarg *arg);

    inline void add_incomplete_jump(uint instrNo, uint iaddress){
        incomplete_jump i;
        i.iaddress = iaddress;
        i.instrNo = instrNo;
        ijs.push_back(i);
    }
    void patch_incomplete_jumps();

    void writeDouble(double num, ofstream& file);

    void writeNum(uint num, ofstream& file);

    void writeString(string s, ofstream& file);

    void backpatch_t(vector<int>& a);

public:
    vector<instruction*> instructions;
    vector<incomplete_jump> ijs;
    vector<string> const_strings;
    vector<double> const_nums;
    vector<string> const_libfuncs;
    vector<userfunc> const_userfuncs;
    int globalCount;


    typedef void (convert::*generator_func_t)(quad*);
    generator_func_t generators[27];

    convert();

    void make_operand(expr* e, vmarg* arg);

    void generate(vector<quad*>& quads, uint globalCount);



    void writeToFile();

};


#endif
