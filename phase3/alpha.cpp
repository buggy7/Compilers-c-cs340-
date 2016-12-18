#include "alpha.h"

string OPCODE_NAMES[] = { "ASSIGN" , "ADD" , "SUB" , "MUL" , "DIV" , "MOD" , "UMINUS" , "AND" , 
"OR" , "NOT" , "IF_EQ","IF_NOTEQ" , "IF_LESSEQ", "IF_GREATEREQ","IF_LESS","IF_GREATER", "JUMP", "CALL","PARAM", 
"RETURN","GETRETVAL","FUNCSTART","FUNCEND","TABLECREATE","TABLEGETELEM", "TABLESETELEM" };

quad::quad(iopcode op, expr* result, int line){
	this->op = op;
	this->result = result;
	this->arg2 = NULL;
	this->arg1 = NULL;
	this->line = line;
	this->label = 0;
}


quad::quad(iopcode op, expr* result, int label, int line){
	this->op = op;
	this->result = result;
	this->arg2 = NULL;
	this->arg1 = NULL;
	this->label = label;
	this->line = line;
}

quad::quad(iopcode op, expr* result, expr* arg1, int line){
	this->op = op;
	this->result = result;
	this->arg2 = NULL;
	this->arg1 = arg1;
	this->line = line;
	this->label = 0;
}

quad::quad(iopcode op, expr* result, expr* arg1, int label, int line){
	this->op = op;
	this->result = result;
	this->arg2 = NULL;
	this->arg1 = arg1;
	this->label = label;
	this->line = line;
}

quad::quad(iopcode op, expr* result, expr* arg1, expr* arg2, int line)  {
	this->op = op;
	this->arg1 = arg1;
	this->arg2 = arg2;
	this->result = result;
	this->line = line;
	this->label = 0;
}

quad::quad(iopcode op, expr* result, expr* arg1, expr* arg2, int label, int line)  {
	this->op = op;
	this->arg1 = arg1;
	this->arg2 = arg2;
	this->result = result;
	this->label = label;
	this->line = line;
}

void quad::print(ofstream& mFile, int num){
	mFile << setiosflags( ios::left )
		<< setw( 4 ) << to_string(num) << " |"
		<< setiosflags( ios::right ) 
		<< setw( 12 ) << OPCODE_NAMES[op] << setw(7) << "|"
		<< setw( 11 ) << printexpr(result) << setw(6) << "|"
		<< setw( 15 ) << printexpr(arg1) << setw(6) << "|"
		<< setw( 15 ) << printexpr(arg2) << setw(6) << "|"
		<< setw( 10 ) << label << setw(6) << "|"
		<< setw( 9 ) << line << setw(6) << "|"
		<< endl;
}

string quad::printexpr(expr* e){
	if (e == NULL) return "(not set)";
	if (e->type == conststring_e) return "\"" + e->strConst + "\"";
	else if (e->type == nil_e) return "NULL";
	else if (e->type == constbool_e) return e->boolConst ? "TRUE" : "FALSE";
	else if (e->type == constnum_e) {
		ostringstream strs;
		strs << e->numConst;
		return strs.str();
	}
	else if (e->type == libraryfunc_e || e->type == arithexpr_e || e->type == var_e
		|| e->type == boolexpr_e || e->type == assignexpr_e) return e->sym->getName();
	else if (e->type == programfunc_e) return e->sym->getName();
	else if (e->type == tableitem_e) return e->sym->getName();
	else if (e->type == newtable_e) return e->sym->getName();
	return "BAD"; 
}


void expr::init(expr_e type, SymbolEntry* sym, expr* index, expr* next){
	this->type = type;
	this->sym = sym;
	this->index = index;
	this->next = next;
}
expr::expr(){
	this->next = NULL;
}
expr::expr(expr_e type){
	init(type, NULL, NULL, NULL);
}
expr::expr(expr_e type, SymbolEntry* sym, expr* index){
	init(type, sym, index, NULL);
}
expr::expr(expr_e type, SymbolEntry* sym, expr* index, expr* next){
	init(type, sym, index, next);
}
expr::expr(double numConst){
	init(constnum_e, NULL, NULL, NULL);
	this->numConst = numConst;

}
expr::expr(string strConst){
	init(conststring_e, NULL, NULL, NULL);
	this->strConst = strConst;
}
expr::expr(bool boolConst){
	init(constbool_e, NULL, NULL, NULL);
	this->boolConst = boolConst;
}

int Stack::pop(){
	int t = s.back();
	s.pop_back();
	return t;
}
void Stack::push(int n){
	s.push_back(n);
}
Stack::Stack(){
	s.push_back(0);
}
Stack::Stack(vector<int>& v){
	s = v;
}
int Stack::get(){
	return s.back();
}
Stack Stack::operator=(int rhs){
	s.back() = rhs;
	return *this;
}
Stack& Stack::operator++(){
	++s.back();
	return *this;
}
Stack& Stack::operator--(){
	--s.back();
	return *this;
}
Stack Stack::operator++(int){
	Stack r(s);
	++(*this);
	return r;
}
Stack Stack::operator--(int){
	Stack r(s);
	--(*this);
	return r;
}

alpha::alpha(){
	currQuad = 0;
	totalErrors = 0;
	tempVars = new TempVar();
	map = new SymbolMap();
}

unsigned int alpha::nextQuadLabel(){
	return currQuad;
}

void alpha::patchLabel(unsigned int quadNo, unsigned int label){
	quads[quadNo]->label = label;
}

void alpha::patchLabels(vector<int>& quadsNo, unsigned int label){
	if (quadsNo.size() == 0) return;
	for(int i = 0; i < quadsNo.size(); i++){
		quads[quadsNo[i]]->label = label;
	}

}

expr* alpha::lvalue_expr(SymbolEntry* sym){
	if (sym == NULL) return NULL;
	expr* e = new expr();
	e->next = NULL;
	e->sym = sym;

	switch(sym->getType()){
		case GLOBAL_VAR:
		case LOCAL_VAR:
		case FORMAL_VAR:
			e->type = var_e;
			break;
		case USERFUNC:
			e->type = programfunc_e;
			break;
		case LIBFUNC:
			e->type = libraryfunc_e;
			break;
		default:
			cout << "Something has gone bad lvalue_expr" << endl;
			break;
	}

	return e;
}

expr* alpha::emit_iftableitem(expr* e, int yylineno){
	if (e->type != tableitem_e){
		return e;
	}
	else{
		expr* result = new expr();
		result->type = var_e;
		result->sym = tempVars->newTemp(map);
		emit(new quad(OPCODE_TABLEGETELEM, result, e, e->index, yylineno));
		return result;
	}
}

expr* alpha::make_call(expr* lvalue, vector<expr*>& args, int yylineno){
	expr* func = emit_iftableitem(lvalue, yylineno);
	for(int i = 0; i < args.size(); i++){
		emit(new quad(OPCODE_PARAM, NULL, args[i], yylineno));
	}

	emit(new quad(OPCODE_CALL, NULL, func, yylineno));
	expr* result = new expr();
	result->type = var_e;
	result->sym = tempVars->newTemp(map);
	emit(new quad(OPCODE_GETRETVAL, result, yylineno));
	return result;
}

expr* alpha::member_item(expr* lvalue, string name, int yylineno){
	lvalue = emit_iftableitem(lvalue, yylineno);
	expr* item = new expr(tableitem_e, lvalue->sym, new expr(name));
	return item;
}

void alpha::writeToFile(){
	ofstream mFile;
	mFile.open("quads.txt");

	mFile << setiosflags( ios::left )
		<< setw( 6 ) << "#    |"  
		<< setiosflags( ios::right )
		<< setw( 12 ) << "OPCODE" << setw(7) << "|"
		<< setw( 11 ) << "Result" << setw(6) << "|"
		<< setw( 15 ) << "Argument_1" << setw(6) << "|"
		<< setw( 15 ) << "Argument_2" << setw(6) << "|"
		<< setw( 10 ) << "Label" << setw(6) << "|"
		<< setw( 9 ) << "Line" << setw(6) << "|"
		<< endl;

	for(int i = 0; i < quads.size(); i++){
		quads[i]->print(mFile, i);
	}


	mFile.close();
}