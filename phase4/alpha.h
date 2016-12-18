#ifndef _ALPHA_H
#define _ALPHA_H

#pragma once

#include "SymbolTable.h"
#include <fstream>
#include <sstream>

enum iopcode {
	OPCODE_ASSIGN, OPCODE_ADD, OPCODE_SUB, OPCODE_MUL, OPCODE_DIV, OPCODE_MOD, 
	OPCODE_UMINUS, OPCODE_AND, OPCODE_OR, OPCODE_NOT, OPCODE_IF_EQ, OPCODE_IF_NOTEQ,
	OPCODE_IF_LESSEQ, OPCODE_IF_GREATEREQ, OPCODE_IF_LESS, OPCODE_IF_GREATER, OPCODE_JUMP, 
	OPCODE_CALL, OPCODE_PARAM, OPCODE_RET, OPCODE_GETRETVAL, OPCODE_FUNCSTART, OPCODE_FUNCEND, 
	OPCODE_TABLECREATE, OPCODE_TABLEGETELEM, OPCODE_TABLESETELEM	
};

enum expr_e {
	var_e, tableitem_e, programfunc_e, libraryfunc_e, arithexpr_e, 
	boolexpr_e, assignexpr_e, newtable_e, constnum_e, constbool_e, 
	conststring_e, nil_e, special_e
};


// lvalue, member, primary, assignexpr, call, term, objectdef, const
class expr {
public:
	expr_e type;
	SymbolEntry* sym;
	expr* index;
	double numConst;
	string strConst;
	bool boolConst;
	expr* next;
	vector<int> breaklist;
	vector<int> contlist;
	bool true_false_added;
	vector<int> truelist;
	vector<int> falselist;

	void init(expr_e type, SymbolEntry* sym, expr* index, expr* next);

	expr();
	expr(expr_e type);
	expr(expr_e type, SymbolEntry* sym, expr* index);
	expr(expr_e type, SymbolEntry* sym, expr* index, expr* next);
	expr(double numConst);
	expr(string strConst);
	expr(bool boolConst);

	inline void newBreak(int n){
		breaklist.push_back(n);
	}
	inline void newCont(int n){
		contlist.push_back(n);
	}
	inline void insertBreaks(vector<int>& v){
		if (v.size() == 0) return;
		breaklist.insert(breaklist.end(),v.begin(),v.end());
	}
	inline void insertConts(vector<int>& v){
		if (v.size() == 0) return;
		contlist.insert(contlist.end(),v.begin(),v.end());
	}
	inline void insertTrue(vector<int>& v){
		if (v.size() == 0) return;
		truelist.insert(truelist.end(),v.begin(),v.end());
	}
	inline void insertFalse(vector<int>& v){
		if (v.size() == 0) return;
		falselist.insert(falselist.end(),v.begin(),v.end());
	}

};

class quad {
public:
	iopcode op;
	expr* result;
	expr* arg1;
	expr* arg2;
	int label;
	int line;
	uint taddress;

	quad(iopcode op, expr* result, expr* arg1, expr* arg2, int label, int line);
	quad(iopcode op, expr* result, expr* arg1, expr* arg2, int line);
	quad(iopcode op, expr* result, expr* arg1, int label, int line);
	quad(iopcode op, expr* result, expr* arg1, int line);
	quad(iopcode op, expr* result, int label, int line);
	quad(iopcode op, expr* result, int line);

	void print(ofstream& mFile, int i);
private:
	string printexpr(expr* e);
};

class TempVar{
public:
	int tempCounter = 0;
    int tempFCounter = 0;

	inline string newTempName(){
		return "_t" + to_string(tempCounter++);
	}

	inline string newTempFuncName(){
		return "_f" + to_string(tempFCounter++);
	}

	inline void resetTemp(){
		tempCounter = 0;
	}

	inline SymbolEntry* newTemp(SymbolMap* map){
		string name = newTempName();
		SymbolEntry* sym = map->lookup(name);
		if (sym == NULL) {
			sym = new SymbolEntry(name, map->scopeOffset->scopeSpaceCounter == GLOBALOFFSET ? GLOBAL_VAR : LOCAL_VAR, map->scopeOffset->scopeSpaceCounter, 0);
            sym->setSpace(map->scopeOffset->currScopeSpace());
            sym->setOffset(map->scopeOffset->currScopeOffset());
            map->scopeOffset->inCurrScopeOffset();
			map->insert(sym);
		}		
		return sym;
	}

	inline bool isTempName(string s){
		return (s[0] == '_');
	}

	inline bool isTempExpr(expr* e){
		return e->sym && isVAR(e->sym->getType()) && isTempName(e->sym->getName());
	}


};

class Stack{
private:
	vector<int> s;
public:
	int pop();
	void push(int n);

	Stack();
	Stack(vector<int>& v);
	int get();
	Stack operator=(int rhs);
	Stack& operator++();
	Stack& operator--();
	Stack operator++(int);
	Stack operator--(int);
};

class alpha{
public:
	SymbolMap* map;
	vector<quad*> quads;
	uint currQuad;
	int totalErrors;
	TempVar* tempVars;

	alpha();

	unsigned int nextQuadLabel();
	void patchLabel(unsigned int quadNo, unsigned int label);
	void patchLabels(vector<int>& quadsNo, unsigned int label);
	inline void emit(quad* q){
		quads.push_back(q);
		currQuad++;
	};
	expr* lvalue_expr(SymbolEntry* sym);
	expr* emit_iftableitem(expr* e, int yylineno);
	expr* make_call(expr* lvalue, vector<expr*>& args, int yylineno); // 10 - 27
	expr* member_item(expr* lvalue, string name, int yylineno);

	void backpatch(vector<int>& quadsNo, unsigned int quadNo);

	void checkuminus(expr* e);
	void checkarith(expr* e);
	void checkrel(expr* e);
	void checkrel(expr* e, expr* e2);
	bool checkreltab(expr* e);
	expr* convertBool(expr* e);
	expr* andop(iopcode op, expr* arg1, expr* arg2, int quadNo, int yylineno);
	expr* orop(iopcode op, expr* arg1, expr* arg2, int quadNo, int yylineno);
	expr* boolop(iopcode op, expr* arg1, expr* arg2, int quadNo, int yylineno);
	expr* notop(iopcode op, expr* arg1, int yylineno);
	expr* relop(iopcode op, expr* arg1, expr* arg2, int yylineno);
	expr* arithop(iopcode op, expr* arg1, expr* arg2, int yylineno);

	void writeToFile();

};

#endif
