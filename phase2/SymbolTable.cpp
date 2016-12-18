#include "SymbolTable.h"
string SYMBOL_TYPE_NAMES[] = { "GLOBAL VARIABLE", "LOCAL VARIABLE", "FORMAL VARIABLE", "USER DEFINED FUNCTION", "LIBRARY FUNCTION" };

SymbolEntry::SymbolEntry(string name, enum SymbolType type, int scope, int line){
	vector<string> arguments;
	init(name, type, scope, line, arguments);
}
SymbolEntry::SymbolEntry(string name, enum SymbolType type, int scope, int line, vector<string> const &arguments){
	init(name, type, scope, line, arguments);
}

void SymbolEntry::init(string name, enum SymbolType type, int scope, int line, vector<string> const &arguments){
	this->name = name;
	this->type = type;
	this->scope = scope;
	this->line = line;
	this->arguments = arguments;
	this->next = NULL;
	this->nextScope = NULL;
	this->isActive = true;
}

bool SymbolEntry::getIsActive(){ return this->isActive; }
void SymbolEntry::setIsActive(bool b) { this->isActive = b; }

int SymbolEntry::getScope() { return this->scope; }
void SymbolEntry::setScope(int scope){ this->scope = scope; }

int SymbolEntry::getLine() { return this->line; }
void SymbolEntry::setLine(int line) { this->line = line; }

string SymbolEntry::getName() { return this->name; }
void SymbolEntry::setName(string name){ this->name = name; }

enum SymbolType SymbolEntry::getType() { return this->type; }
void SymbolEntry::setType(enum SymbolType type) { this->type = type; }

vector<string> SymbolEntry::getArguments() { return this->arguments; }
void SymbolEntry::setArguments(vector<string> const &arguments) { this->arguments = arguments; }
 
SymbolEntry *SymbolEntry::getNext() { return next; }
void SymbolEntry::setNext(SymbolEntry *next) { this->next = next; }

SymbolEntry *SymbolEntry::getNextScope() { return nextScope; }
void SymbolEntry::setNextScope(SymbolEntry *nextScope) { this->nextScope = nextScope; }

void SymbolEntry::print(){
	cout << "name: " << setw(20) << left << name << " type: " << setw(21) << left << SYMBOL_TYPE_NAMES[type] << " line: " << line << "\tscope: " << scope << "\tactive: ";
	if (isActive) cout << "True";
	else cout << "False";
	if (type == USERFUNC && arguments.size() > 0)
	{
		cout << "\targuments: ";
		for (int i = 0; i < arguments.size(); i++) cout << arguments[i] << ",";
	}
	cout << endl;

}

 SymbolMap::SymbolMap() {
 	functionID = 0;
 	currentScope = 0;
 	table = new SymbolEntry*[TABLE_SIZE];
 	tableScope = new SymbolEntry*[TABLE_SIZE];
    for (int i = 0; i < TABLE_SIZE; i++) {
    	table[i] = NULL;
    	tableScope[i] = NULL;
    }

    lib = { "print", "input", "objectmemberkeys", "objecttotalmembers", "objectcopy", "totalarguments", "argument", "typeof", "strtonum", "sqrt", "cos", "sin" };
	for (int i = 0; i < lib.size(); i++) insertLibrary(lib[i]);
}

void SymbolMap::insertLibrary(string name) {
	SymbolEntry* nentry = new SymbolEntry(name, LIBFUNC, 0, 0);

	int Symbol = getHash(name);
	if (table[Symbol] == NULL)
		table[Symbol] = nentry;
	else {
		SymbolEntry *entry = table[Symbol];
		nentry->setNext(entry);
		table[Symbol] = nentry;
	}

	int s = getHash(0);
	if (tableScope[s] == NULL)
		tableScope[s] = nentry;
	else {
		SymbolEntry *entry = tableScope[s];
		nentry->setNextScope(entry);
		tableScope[s] = nentry;
	}
}

bool SymbolMap::isLibraryName(string name){
	for (int i = 0; i < lib.size(); i++) if (name == lib[i]) return true;
	return false;
}
 
int SymbolMap::getHash(string name){
	int sum=0;
    for(int i=0; i < name.length(); i++)
    {
        sum += name[i];
    }
    return sum % TABLE_SIZE;
}

int SymbolMap::getHash(int scope){
    return scope % TABLE_SIZE;
}

SymbolEntry* SymbolMap::lookupAll(string name) {
	int scope = currentScope;
	SymbolEntry* entry = NULL;
	while ((entry = lookup(name, scope)) == NULL && scope >= 0){
		scope--;
	}
	return entry;
}

SymbolEntry* SymbolMap::lookup(string name) {
	return lookup(name, currentScope);
}

SymbolEntry* SymbolMap::lookup(string name, int scope) {
	int Symbol = getHash(name);
	if (table[Symbol] == NULL)
		return NULL;
	else {
		SymbolEntry *entry = table[Symbol];
		while (entry != NULL){
			if (entry->getName() == name 
				&& entry->getScope() == scope
				 && entry->getIsActive()) return entry;
			entry = entry->getNext();
		}
	}
	return NULL;
}

void SymbolMap::hide(){
	hide(currentScope);
}

void SymbolMap::hide(int scope){
	/*for (int i = 0; i < TABLE_SIZE; i++)
    	if (table[i] != NULL) {
            SymbolEntry *entry = table[i];
            while (entry != NULL) {
            	if (entry->getScope() == scope){
            		entry->setIsActive(false);
            	}
                entry = entry->getNext();
			}
	}*/
	int s = getHash(scope);
	if (tableScope[s] != NULL) {
		SymbolEntry *entry = tableScope[s];
        while (entry != NULL) {
           	entry->setIsActive(false);
            entry = entry->getNextScope();
		}
	}
}
 
void SymbolMap::insert(SymbolEntry* nentry) {
	if (nentry->getName() == " " && nentry->getType() == USERFUNC) {
		nentry->setName("$f" + to_string(++functionID));
	}

	int Symbol = getHash(nentry->getName());
	if (table[Symbol] == NULL)
		table[Symbol] = nentry;
	else {
		SymbolEntry *entry = table[Symbol];
		nentry->setNext(entry);
		table[Symbol] = nentry;
	}

	int s = getHash(nentry->getScope());
	if (tableScope[s] == NULL)
		tableScope[s] = nentry;
	else {
		SymbolEntry *entry = tableScope[s];
		nentry->setNextScope(entry);
		tableScope[s] = nentry;
	}
}
 
void SymbolMap::remove(string name) {
	int Symbol = getHash(name);
	if (table[Symbol] != NULL) {
    	SymbolEntry *prevEntry = NULL;
		SymbolEntry *entry = table[Symbol];
		while (entry->getNext() != NULL && entry->getName() != name) {
        	prevEntry = entry;
            entry = entry->getNext();
		}
        if (entry->getName() == name) {
        	if (prevEntry == NULL) {
            	SymbolEntry *nextEntry = entry->getNext();
                delete entry;
                table[Symbol] = nextEntry;
            } else {
            	SymbolEntry *next = entry->getNext();
                delete entry;
                prevEntry->setNext(next);
            }
    	}
	}
}

void SymbolMap::print(){
	for (int i = 0; i < TABLE_SIZE; i++)
    	if (tableScope[i] != NULL) {
    		cout << endl << "Scope: " << i << endl << endl;
            SymbolEntry *entry = tableScope[i];
            while (entry != NULL) {
            	entry->print();
                entry = entry->getNextScope();
			}
	}
	cout << endl;
}


SymbolMap::~SymbolMap() {
    for (int i = 0; i < TABLE_SIZE; i++){
    	if (table[i] != NULL) {
        	SymbolEntry *prevEntry = NULL;
            SymbolEntry *entry = table[i];
            while (entry != NULL) {
            	prevEntry = entry;
                entry = entry->getNext();
                delete prevEntry;
			}
		}
	}
	delete[] table;
	delete[] tableScope;
}