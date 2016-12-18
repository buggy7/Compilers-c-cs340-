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
	true_false_added = false;
}
expr::expr(){
	this->next = NULL;
	true_false_added = false;
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

void alpha::backpatch(vector<int>& quadsNo, unsigned int label){
	if (quadsNo.size() == 0) return;
	for(int i = 0; i < quadsNo.size(); i++){
		quads[quadsNo[i]]->label = label;
	}
}

void alpha::checkuminus(expr* e){
	if (e->type != var_e && e->type != tableitem_e &&
		e->type != arithexpr_e && e->type != assignexpr_e &&
		e->type != constnum_e) {
		cout << "ERROR: ILLEGAL EXPR TO UNARY -" << endl;
		totalErrors++;
	}
}

void alpha::checkarith(expr* e){
	if(e->type == programfunc_e || e->type == libraryfunc_e ||
		e->type == boolexpr_e || e->type == newtable_e ||
		e->type == constbool_e || e->type == conststring_e ||
		e->type == nil_e) {
		cout << "ERROR: ILLEGAL ARITH EXPR" << endl;
		totalErrors++;
	}
}

void alpha::checkrel(expr* e){
	if(e->type == programfunc_e || e->type == libraryfunc_e ||
		e->type == boolexpr_e || e->type == newtable_e ||
		e->type == constbool_e || e->type == conststring_e ||
		e->type == nil_e) {
		cout << "ERROR: ILLEGAL REL EXPR" << endl;
		totalErrors++;
	}
}

expr* alpha::convertBool(expr* e){
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

expr* alpha::andop(iopcode op, expr* arg1, expr* arg2, int quadNo, int yylineno)
{
	expr* result = new expr(boolexpr_e);
	result->sym = tempVars->newTemp(map);

	expr* temp1 = convertBool(arg1);
	expr* temp2 = convertBool(arg2);
	
	//int q = nextQuadLabel();

	if (arg1->type == boolexpr_e) {
		backpatch(arg1->truelist, quadNo);
		arg1->truelist.clear();
	}

	if (arg1->type != boolexpr_e) {
		arg1->truelist.push_back(nextQuadLabel());
		arg1->falselist.push_back(nextQuadLabel() + 1);
		emit(new quad(OPCODE_IF_EQ, NULL, arg1, new expr(true), 0, yylineno));						
		emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
		quadNo += 2;
	}
	//q = nextQuadLabel();
	if (arg2->type != boolexpr_e){
		arg2->falselist.push_back(nextQuadLabel() + 1);
		arg2->truelist.push_back(nextQuadLabel());
		emit(new quad(OPCODE_IF_EQ, NULL, arg2, new expr(true), 0, yylineno));						
		emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
	}

	result->true_false_added = true;

	backpatch(arg1->truelist, quadNo);
	result->truelist = arg2->truelist;
	result->falselist = arg1->falselist;
	result->insertFalse(arg2->falselist);

	return result;
}

expr* alpha::orop(iopcode op, expr* arg1, expr* arg2, int quadNo, int yylineno)
{
	expr* result = new expr(boolexpr_e);
	result->sym = tempVars->newTemp(map);

	//expr* temp1 = convertBool(arg1);
	//expr* temp2 = convertBool(arg2);

	int q = nextQuadLabel();

	if (arg1->type == boolexpr_e) {
		backpatch(arg1->falselist, quadNo);
		arg1->falselist.clear();
	}


	if (arg1->type != boolexpr_e) {
		arg1->truelist.push_back(nextQuadLabel());
		arg1->falselist.push_back(nextQuadLabel() + 1);
		emit(new quad(OPCODE_IF_EQ, NULL, arg1, new expr(true), 0, yylineno));						
		emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
	}

	if (arg2->type != boolexpr_e) {
		arg2->falselist.push_back(nextQuadLabel() + 1);
		arg2->truelist.push_back(nextQuadLabel());
		emit(new quad(OPCODE_IF_EQ, NULL, arg2, new expr(true), 0, yylineno));						
		emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
	}
	else if (arg1->type != boolexpr_e){
		backpatch(arg2->falselist, q);
		arg2->falselist.clear();
		arg2->falselist.push_back(q + 1);
	}
	

	result->true_false_added = true;

	backpatch(arg1->falselist, q + 2);
	result->truelist = arg1->truelist;
	result->insertTrue(arg2->truelist);
	result->falselist = arg2->falselist;

	return result;
}


expr* alpha::boolop(iopcode op, expr* arg1, expr* arg2, int quadNo, int yylineno)
{
	//checkbool(arg1);
	//checkbool(arg2);

	/*expr* result = new expr(boolexpr_e);
	result->sym = tempVars->newTemp(map);

	expr* temp1 = convertBool(arg1);
	expr* temp2 = convertBool(arg2);


	emit(new quad(OPCODE_IF_EQ, NULL, arg1, new expr(true), 0, yylineno));						
	emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));

	emit(new quad(OPCODE_IF_EQ, NULL, arg2, new expr(true), 0, yylineno));						
	emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));*/
	if (op == OPCODE_OR){
		return orop(op, arg1, arg2, quadNo, yylineno);
	}
	else{
		return andop(op, arg1, arg2, quadNo, yylineno);
	}

	//emit(new quad(op, result, convertBool(arg1), convertBool(arg2), yylineno));
	
	/*if (op == OPCODE_OR){
		backpatch(arg1->falselist, quadNo);
		result->truelist = arg1->truelist;
		result->insertTrue(arg2->truelist);
		result->falselist = arg2->falselist;
		result->truelist.push_back(nextQuadLabel());
	}
	else if (op == OPCODE_AND){
		backpatch(arg1->truelist, quadNo);
		result->truelist = arg2->truelist;
		result->falselist = arg1->falselist;
		result->insertFalse(arg2->falselist);
		result->falselist.push_back(nextQuadLabel() + 1);
	}
	result->true_false_added = true;

	

	//result->truelist.push_back(nextQuadLabel());
	//result->falselist.push_back(nextQuadLabel() + 1);
	emit(new quad(OPCODE_IF_EQ, NULL, result, new expr(true), 0, yylineno));						
	emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));*/

	//return result;

}

expr* alpha::notop(iopcode op, expr* arg1, int yylineno)
{
	   //checkbool(arg1);
	expr* result = new expr(boolexpr_e);
	result->sym = tempVars->newTemp(map);

	result->truelist = arg1->falselist;
	result->falselist = arg1->truelist;

	//result->true_false_added = true;

	//emit(new quad(op, result, convertBool(arg1), yylineno));

	if (arg1->type != boolexpr_e) {
		result->truelist.push_back(nextQuadLabel() + 1);
		result->falselist.push_back(nextQuadLabel());
		emit(new quad(OPCODE_IF_EQ, NULL, arg1, new expr(true), 0, yylineno));
		emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
		result->true_false_added = true;
	}

	return result;

}

expr* alpha::relop(iopcode op, expr* arg1, expr* arg2, int yylineno)
{
	checkrel(arg1);
	checkrel(arg2);

	expr* result = new expr(boolexpr_e);
	result->sym = tempVars->newTemp(map);
	result->truelist.push_back(nextQuadLabel());
	result->falselist.push_back(nextQuadLabel() + 1);

	result->true_false_added = true;

	emit(new quad(op, NULL, arg1, arg2, 0, yylineno));
	//emit(new quad(OPCODE_ASSIGN, result, new expr(false), yylineno));
	emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
	//emit(new quad(OPCODE_ASSIGN, result, new expr(true), yylineno));
	return result;
}

expr* alpha::arithop(iopcode op, expr* arg1, expr* arg2, int yylineno){
	checkarith(arg1);
	checkarith(arg2);
	expr* result = new expr((arg1)->type == constnum_e && (arg2)->type == constnum_e ? constnum_e : arithexpr_e);
	result->sym = tempVars->newTemp(map);
	emit(new quad(op, result, arg1, arg2, yylineno));
	return result;
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