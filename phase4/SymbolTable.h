#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;
typedef unsigned int uint;

const int TABLE_SIZE = 128;

#define GLOBALOFFSET 1
#define isVAR(x) (x == GLOBAL_VAR || x == LOCAL_VAR || x == FORMAL_VAR) 

enum SymbolType { GLOBAL_VAR, LOCAL_VAR, FORMAL_VAR, USERFUNC, LIBFUNC };
enum ScopeSpace { PROGRAM_VAR, FUNCTION_LOCAL, FORMAL_ARG };

class ScopesOffsets{
public:
 	unsigned int programVarOffset = 0;
 	unsigned int functionLocalOffset = 0;
 	unsigned int formalArgOffset = 0;
 	unsigned int scopeSpaceCounter = 1;

 	enum ScopeSpace currScopeSpace();
 	unsigned int currScopeOffset();
 	void inCurrScopeOffset();
 	void enterScopeSpace();
 	void exitScopeSpace();

	void resetFormalArgsOffset();
	void resetFunctionLocalOffset();
	void restoreCurrScopeOffset(unsigned int n);

	vector<int> functionLocalsStack;
	
	inline void push(){
		functionLocalsStack.push_back(functionLocalOffset);
	}

	inline void pop(){
		functionLocalOffset = functionLocalsStack.back();
		functionLocalsStack.pop_back();
	}

    inline void print(int id){
        cout << id << " - scopespacecounter: " << scopeSpaceCounter
        << " functionLocalOffset: " << functionLocalOffset
        << " formalArgOffset: " << formalArgOffset
        << " functionLocalsStackSize: " << functionLocalsStack.size()
        << endl;
    }
};


class SymbolEntry{
private:
	SymbolEntry *next;
	SymbolEntry *nextScope;
	bool isActive;
	string name;
	enum SymbolType type;
	enum ScopeSpace space;
	int offset;
	int scope;
	int line;
    int totalLocals;
	uint taddress;
	vector<string> arguments;

	void init(string name, enum SymbolType type, int scope, int line, vector<string> const &arguments);

public:
	SymbolEntry(string name);
	SymbolEntry(string name, enum SymbolType type, int scope, int line);
	SymbolEntry(string name, enum SymbolType type, int scope, int line, vector<string> const &arguments);

	void setNext(SymbolEntry* next);
	SymbolEntry* getNext();

	void setNextScope(SymbolEntry* Scope);
	SymbolEntry* getNextScope();

	bool getIsActive();
	void setIsActive(bool b);

	int getScope();
	void setScope(int scope);

	uint getAddress();
	void setAddress(uint address);

	int getLine();
	void setLine(int line);

	string getName();
	void setName(string name);

    int getTotalLocals();
    void setTotalLocals(int totalLocals);

	enum SymbolType getType();
	void setType(enum SymbolType type);

	enum ScopeSpace getSpace();
	void setSpace(enum ScopeSpace space);

	int getOffset();
	void setOffset(int offset);

	vector<string> getArguments();
	void setArguments(vector<string> const &arguments);

	void print();
};



class SymbolMap{
private:
	SymbolEntry **table;
	SymbolEntry **tableScope;
	int functionID;
	vector<string> lib;

	int getHash(string name);
	int getHash(int scope);
	void remove(string name);
	void insertLibrary(string name);
public:
	ScopesOffsets* scopeOffset;
	//int currentScope;
    uint globalCount;

	SymbolMap();
	~SymbolMap();
	void insert(SymbolEntry* entry);
	SymbolEntry* lookupAll(string name);
	SymbolEntry* lookup(string name);
	SymbolEntry* lookup(string name, int scope);
	void hide();
	void hide(int scope);

	bool isLibraryName(string name);
	void print();
};

#endif
