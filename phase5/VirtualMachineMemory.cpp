#include "VirtualMachineMemory.h"

avm_memcellclear::avm_memcellclear(avm_memcell* m){
	memclearFuncs[0] = NULL;
	memclearFuncs[1] = &avm_memcellclear::memclear_string;
	memclearFuncs[2] = NULL;
	memclearFuncs[3] = &avm_memcellclear::memclear_table;
	memclearFuncs[4] = NULL;
	memclearFuncs[5] = NULL;
	memclearFuncs[6] = NULL;
	memclearFuncs[7] = NULL;


	if (m->type != undef_m){
		memclear_func_t f = memclearFuncs[m->type];
		if (f) (this->*f)(m);
		m->type = undef_m;
	}
}

void avm_memcellclear::memclear_string(avm_memcell* m){
	if (m->data.strVal) delete m->data.strVal;
}

void avm_memcellclear::memclear_table(avm_memcell* m){
	if (m->data.tableVal) m->data.tableVal->avm_tabledecrefcounter();
}


avm_table::avm_table(){
	refCounter = 0;
	totalNum = totalStr = 0;
	avm_tablebucketsinit(numIndexed);
	avm_tablebucketsinit(strIndexed);

    int i = 0;
    compareFuncs[i++]= &avm_table::number_compareimpl;
    compareFuncs[i++]= &avm_table::string_compareimpl;
    compareFuncs[i++]= &avm_table::bool_compareimpl;
    compareFuncs[i++]= &avm_table::table_compareimpl;
    compareFuncs[i++]= &avm_table::userfunc_compareimpl;
    compareFuncs[i++]= &avm_table::libfunc_compareimpl;
    compareFuncs[i++]= &avm_table::nil_compareimpl;
    compareFuncs[i++]= &avm_table::undef_compareimpl;

    i = 0;
    hashFuncs[i++] = &avm_table::number_hashimpl;
    hashFuncs[i++] = &avm_table::string_hashimpl;
    hashFuncs[i++] = &avm_table::bool_hashimpl;
    hashFuncs[i++] = &avm_table::table_hashimpl;
    hashFuncs[i++] = &avm_table::userfunc_hashimpl;
    hashFuncs[i++] = &avm_table::libfunc_hashimpl;
    hashFuncs[i++] = &avm_table::nil_hashimpl;
    hashFuncs[i++] = &avm_table::undef_hashimpl;
}

avm_table::~avm_table(){
	//avm_tablebucketsdestroy(numIndexed);
	//avm_tablebucketsdestroy(strIndexed);
}

avm_memcell* avm_table::avm_tablegetelem(avm_memcell* key){
	int hash = 0;
    avm_table_bucket* temp = NULL;

    hash = (this->*(this->hashFuncs[key->type]))(key);
    switch (key->type){
        case number_m:
		case table_m:
            temp = numIndexed[hash];
            break;
        case string_m:
            temp = strIndexed[hash];
            break;
        default:
            cout << "Type isn't supported." << endl;
            return NULL;
    }

    while (temp){
        if ((this->*(this->compareFuncs[key->type]))(&temp->key, key))
            return &temp->value;
        temp = temp->next;
    }
    return NULL;
}

void avm_table::avm_tablesetelem(avm_memcell* key, avm_memcell* value){
    int hash = 0;
    avm_table_bucket* temp = NULL;

    hash = (this->*(this->hashFuncs[key->type]))(key);
    switch (key->type){
        case number_m:
		case table_m:
            temp = numIndexed[hash];
			totalNum++;
            break;
        case string_m:
            temp = strIndexed[hash];
			totalStr++;
            break;
        case nil_m:
            delete_elem(key, hash);
            return;
        default:
            cout << "Type isn't supported." << endl;
            return;
    }
	totalActuals++;

    avm_table_bucket* n = new avm_table_bucket();
    n->next = NULL;
    n->key = *key;
    n->value = *value;

    while (temp != NULL && temp->next != NULL) temp = temp->next;
    if (temp == NULL){
        switch (key->type){
            case number_m:
			case table_m:
                numIndexed[hash] = n;
                break;
            case string_m:
                strIndexed[hash] = n;
                break;
            default:
                cout << "Type isn't supported." << endl;
                return;
        }
    }
    else{
		if ((this->*(this->compareFuncs[key->type]))(&temp->key, key)){
			temp->value = *value;
		}
		else temp->next = n;
    }

    return;
}

void avm_table::delete_elem(avm_memcell* key, int hash){
    avm_table_bucket* temp = NULL;
    avm_table_bucket* prev = NULL;
    switch (key->type){
        case number_m:
		case table_m:
            temp = numIndexed[hash];
			totalNum--;
			totalActuals--;
            break;
        case string_m:
            temp = strIndexed[hash];
			totalStr--;
			totalActuals--;
            break;
        default:
            cout << "Type isn't supported." << endl;
            return;
    }

    while (temp){
        if ((this->*(this->compareFuncs[key->type]))(&temp->key, key)) {
            if (prev) prev->next = temp->next;
            else {
                switch (key->type){
                    case number_m:
					case table_m:
                        numIndexed[hash] = NULL;
                        break;
                    case string_m:
                        strIndexed[hash] = NULL;
                        break;
                    default:
                        cout << "Type isn't supported." << endl;
                        return;
                }
            }
            delete temp;
            return;
        }
        prev = temp;
        temp = temp->next;
    }

}

void avm_table::avm_tablebucketsinit(avm_table_bucket** p){
	for (uint i = 0; i < AVM_TABLE_HASHSIZE; i++){
		p[i] = NULL;
	}
}

void avm_table::avm_tablebucketsdestroy(avm_table_bucket** p){
	for (uint i = 0; i < AVM_TABLE_HASHSIZE; i++, p++){
		for (avm_table_bucket* b = *p; b;){
			avm_table_bucket* del = b;
			b = b->next;
			new avm_memcellclear(&del->key);
			new avm_memcellclear(&del->value);
			delete del;
		}
		p[i] = NULL;
	}
}

int avm_table::number_hashimpl(avm_memcell *memcell) {
    return (int)(memcell->data.numVal) % AVM_TABLE_HASHSIZE;
}

int avm_table::string_hashimpl(avm_memcell *memcell) {
    uint h = 31 /* also prime */;
    string s = *memcell->data.strVal;
    for (int i =0; i < s.size(); i++){
        h = (h * 54059) ^ (s[i] * 76963);
    }
    return h % AVM_TABLE_HASHSIZE; // or return h % C;
}

int avm_table::bool_hashimpl(avm_memcell *memcell) {
    return 0;
}

int avm_table::table_hashimpl(avm_memcell *memcell) {
	return (int)((size_t)memcell->data.tableVal % AVM_TABLE_HASHSIZE);
}

int avm_table::userfunc_hashimpl(avm_memcell *memcell) {
    return 0;
}

int avm_table::libfunc_hashimpl(avm_memcell *memcell) {
    return 0;
}

int avm_table::nil_hashimpl(avm_memcell *memcell) {
    return 0;
}

int avm_table::undef_hashimpl(avm_memcell *memcell) {
    return 0;
}

bool avm_table::number_compareimpl(avm_memcell *memcell, avm_memcell *avmMemcell) {
    return memcell->data.numVal == avmMemcell->data.numVal;
}

bool avm_table::string_compareimpl(avm_memcell *memcell, avm_memcell *avmMemcell) {
    return *memcell->data.strVal == *avmMemcell->data.strVal;
}

bool avm_table::bool_compareimpl(avm_memcell *memcell, avm_memcell *avmMemcell) {
    return false;
}

bool avm_table::table_compareimpl(avm_memcell *memcell, avm_memcell *avmMemcell) {
	return memcell->data.tableVal == avmMemcell->data.tableVal;
}

bool avm_table::userfunc_compareimpl(avm_memcell *memcell, avm_memcell *avmMemcell) {
    return false;
}

bool avm_table::libfunc_compareimpl(avm_memcell *memcell, avm_memcell *avmMemcell) {
    return false;
}

bool avm_table::nil_compareimpl(avm_memcell *memcell, avm_memcell *avmMemcell) {
    return false;
}

bool avm_table::undef_compareimpl(avm_memcell *memcell, avm_memcell *avmMemcell) {
    return false;
}


void VirtualMachineMemory::loadFile(string f){

	file.open(f, ios::in|ios::binary);
  	if (file.is_open())
  	{
	    if (readNum() == 340200501){
	    	int total = 0;
	    	// read strings
	    	total = readNum();
	    	for(int i = 0; i < total; i++){
	    		stringConsts.push_back(readString());
	    	}

	    	// read numbers
	    	total = readNum();
	    	for(int i = 0; i < total; i++){
				numConsts.push_back(readDouble());				
	    	}

			// read libfunctions
			total = readNum();
			for (int i = 0; i < total; i++){
				namedLibFuncs.push_back(readString());
			}

	    	// read user functions
	    	total = readNum();
	    	for(int i = 0; i < total; i++){
				userfunc u = userfunc();
	    		u.address = readNum();
	    		u.localSize = readNum();
	    		u.id = readString();
				userFuncs.push_back(u);
	    	}

	    	// read code
	    	total = readNum();
	    	for (int i = 0; i < total; i++){
	    		char* r = new char[16];
	    		file.read(r, 16); 

	    		instruction s = instruction();
				s.opcode = (vmopcode)r[0];

				//s.operand1 = operand();
				s.result.type = (vmarg_t)r[1];
				s.result.value = *(uint*)(r + 2);

				//s.operand2 = operand();
				s.arg1.type = (vmarg_t)r[6];
				s.arg1.value = *(uint*)(r + 7);

				//s.result = operand();
				s.arg2.type = (vmarg_t)r[11];
				s.arg2.value = *(uint*)(r + 12);

				instructions.push_back(s);
	    	}

			globalCount = readNum();
	    }
	    else cout << "Wrong magic number!" << endl;


	    file.close();
  	}
  	else cout << "Unable to open file" << endl;
}

double VirtualMachineMemory::readDouble(){
	char* n = new char[8];

	file.read(n, 8);
	double num = *(double*)n;

	delete[] n;
	return num;
}

uint VirtualMachineMemory::readNum(){
	char* n = new char[4];

	file.read(n, 4);
	uint num = *(uint*)n;

	delete[] n;
	return num;
}

string VirtualMachineMemory::readString(){
	int size = readNum();
	char* n = new char[size + 1];

	file.read(n, size);
	n[size] = '\0';
	string str(n);
	delete[] n;

	return str;
}

void VirtualMachineMemory::avm_wipeout(avm_memcell* t){
	memset(t, 0, sizeof(avm_memcell));
}

void VirtualMachineMemory::avm_initstack(){
	for (uint i = 0; i < AVM_STACKSIZE; i++){
		avm_wipeout(&stack[i]);
		stack[i].type = undef_m;
	}
}

VirtualMachineMemory::VirtualMachineMemory(string f){
	loadFile(f);
	top = topsp = AVM_STACKSIZE - 1 - globalCount;
    avm_initstack();
}

VirtualMachineMemory::~VirtualMachineMemory(){

}