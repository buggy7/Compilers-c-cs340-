#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

const int TABLE_SIZE = 128;

enum SymbolType { GLOBAL_VAR, LOCAL_VAR, FORMAL_VAR, USERFUNC, LIBFUNC };

class SymbolEntry{
private:
	SymbolEntry *next;
	SymbolEntry *nextScope;
	bool isActive;
	string name;
	enum SymbolType type;
	int scope;
	int line;
	vector<string> arguments;

	void init(string name, enum SymbolType type, int scope, int line, vector<string> const &arguments);

public:
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

	int getLine();
	void setLine(int line);

	string getName();
	void setName(string name);

	enum SymbolType getType();
	void setType(enum SymbolType type);

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
	int currentScope;

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