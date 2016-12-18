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

};

class quad {
public:
	iopcode op;
	expr* result;
	expr* arg1;
	expr* arg2;
	int label;
	int line;

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

	inline string newTempName(){
		return "_t" + to_string(tempCounter++);
	}

	inline string newTempFuncName(){
		return "_f" + to_string(tempCounter++);
	}

	inline void resetTemp(){
		tempCounter = 0;
	}

	inline SymbolEntry* newTemp(SymbolMap* map){
		string name = newTempName();
		SymbolEntry* sym = map->lookup(name);
		if (sym == NULL) {
			sym = new SymbolEntry(name);
		}
		map->insert(sym);
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
	int currQuad;
	int totalErrors;
	TempVar* tempVars;

	alpha();

	unsigned int nextQuadLabel();
	void patchLabel(unsigned int quadNo, unsigned int label);
	void patchLabels(vector<int>& quadsNo, unsigned int label);
	inline void emit(quad* q){
		quads.push_back(q);
		currQuad++;
	}
	expr* lvalue_expr(SymbolEntry* sym);
	expr* emit_iftableitem(expr* e, int yylineno);
	expr* make_call(expr* lvalue, vector<expr*>& args, int yylineno); // 10 - 27
	expr* member_item(expr* lvalue, string name, int yylineno);

	inline void checkuminus(expr* e){
		if (e->type != var_e && e->type != tableitem_e &&
			e->type != arithexpr_e && e->type != assignexpr_e &&
			e->type != constnum_e) {
				cout << "ERROR: ILLEGAL EXPR TO UNARY -" << endl;
				totalErrors++;
		}
	}
	inline void checkarith(expr* e){
		if(e->type == programfunc_e || e->type == libraryfunc_e ||
			e->type == boolexpr_e || e->type == newtable_e ||
			e->type == constbool_e || e->type == conststring_e ||
			e->type == nil_e) {
				cout << "ERROR: ILLEGAL ARITH EXPR" << endl;
				totalErrors++;
		}
	}
	
	inline void checkrel(expr* e){
		if(e->type == programfunc_e || e->type == libraryfunc_e ||
			e->type == boolexpr_e || e->type == newtable_e ||
			e->type == constbool_e || e->type == conststring_e ||
			e->type == nil_e) {
				cout << "ERROR: ILLEGAL REL EXPR" << endl;
				totalErrors++;
		}
	}
	inline void checkrel(expr* e , expr* e2){
		if(e->type != e2->type) {
			cout << "ERROR: ILLEGAL REL EXPR" << endl;
			totalErrors++;
		}
	}
	inline bool checkreltab(expr* e){
		if(e->type == newtable_e){
			if( e->type == nil_e) return false;
		}
		return true;
	}
	inline expr* convertBool(expr* e){
		expr* r = e;
		if(e->type == programfunc_e || e->type == libraryfunc_e || e->type == newtable_e ) r = new expr(true); 

		if(e->type == constnum_e){
			r = new expr(e->numConst != 0);
		}
		if(e->type == conststring_e){
			r = new expr(e->strConst != "");
		}
		if(e->type == boolexpr_e) return e;
		if(e->type == constbool_e) return e;
		
		return r;
	}
	
	inline expr* boolop(iopcode op, expr* arg1, expr* arg2, int yylineno)
	{
	   //checkbool(arg1);
	   //checkbool(arg2);
	   expr* result = new expr(boolexpr_e);
	   result->sym = tempVars->newTemp(map);
	   emit(new quad(op, result, convertBool(arg1), convertBool(arg2), yylineno));
	   return result;
	  
	}
	
	inline expr* boolop(iopcode op, expr* arg1, int yylineno)
	{
	   //checkbool(arg1);
	   expr* result = new expr(boolexpr_e);
	   result->sym = tempVars->newTemp(map);
	   emit(new quad(op, result, convertBool(arg1), yylineno));
	   return result;
	  
	}

	inline expr* relop(iopcode op, expr* arg1, expr* arg2, int yylineno)
	{
	  if( op == OPCODE_IF_EQ && op == OPCODE_IF_NOTEQ){
		checkrel(arg1, arg2);
		checkreltab(arg1);
		checkreltab(arg2);
	  }
	  else{
		checkrel(arg1);
		checkrel(arg2);
	  }
	  expr* result = new expr(boolexpr_e);
	  result->sym = tempVars->newTemp(map);

	  emit(new quad(op, NULL, arg1, arg2, nextQuadLabel() + 3, yylineno));
	  emit(new quad(OPCODE_ASSIGN, result, new expr(false), yylineno));
	  emit(new quad(OPCODE_JUMP, NULL, nextQuadLabel() + 2, yylineno));
	  emit(new quad(OPCODE_ASSIGN, result, new expr(true), yylineno));
	  return result;
	}

	inline expr* arithop(iopcode op, expr* arg1, expr* arg2, int yylineno){
	  checkarith(arg1);
	  checkarith(arg2);
	  expr* result = new expr((arg1)->type == constnum_e && (arg2)->type == constnum_e ? constnum_e : arithexpr_e);
	  result->sym = tempVars->newTemp(map);
	  emit(new quad(op, result, arg1, arg2, yylineno));
	  return result;
	}

	void writeToFile();

};

#endif