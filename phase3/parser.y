%{
	#include <stdio.h>
	#include <iostream>
	#include <string>
	#include <vector>
	#include "alpha.h"

  #define DEBUG

  #ifdef DEBUG
  #define DBG( msg ) cout << msg 
  #else
  #define DBG( msg )
  #endif

  #define scopeOffset map->scopeOffset

	int yyerror(string yaccProvidedMessage);

	extern int yylex(void);
	extern int yylineno;
	extern char* yytext;
	extern FILE* yyin;
	extern FILE* yyout;

	using namespace std;

  alpha* al;
  SymbolMap* map;
  Stack loopcounter;
  bool scopeFunctionInc = false;
  int insideFunc = 0;
  vector<vector<string>> tempArgs;

  struct functioncall{
    vector<expr*>* elist;
    bool method;
    string* name;
  };

  struct indexedElemExprs{
    expr* arg1;
    expr* arg2;
  };

  struct sForPrefix{
    unsigned int enter;
    unsigned int test;
  };

%}

%start program
%error-verbose

%union
{
	int intValue;
	string* stringValue;
	float floatValue;
  void* exprValue;  // *(expr*)
  void* symValue;   // *(SymbolEntry*)
  void* funValue;   // *(functioncall*)
  void* exprsValue; // vector<expr*>*
  void* indexElemValue; // *(indexedElemExprs*)
  void* indexValue; // vector<indexedElemExprs*>*
  void* sForPrefixValue; // *(sForPrefix*)
}


%token <intValue> INTEGER
%token <floatValue> FLOAT
%token <stringValue> IDENTIFIER
%token <stringValue> STRING

%token <stringValue> IF ELSE WHILE AND NOT RETURN LOCAL TRUE FALSE NIL OR FOR FUNCTION CONTINUE BREAK 
%token <stringValue> ASSIGN PLUS ASTERISK MINUS SLASH PLUSPLUS MINUSMINUS MODULO EQUALS NEQUALS GREATER LESS GREATEREQ LESSEQ
%token <stringValue> CURLY_BRACKET_OPEN CURLY_BRACKET_CLOSE BRACKET_OPEN BRACKET_CLOSE PARENTHESIS_CLOSE PARENTHESIS_OPEN
%token <stringValue> SEMICOLON DOUBLE_COLON COLON DOT DOUBLE_DOT COMMA

%type <intValue> ifprefix elseprefix whilestart whilecond N M
%type <exprValue> lvalue expr lvalue2 tableitem assignexpr call objectdef primary term stmt loopstmt stmtloop block
%type <exprValue> ifstmt whilestmt forstmt returnstmt const member funcdef
%type <stringValue> idlist funcname
%type <symValue> funcprefix
%type <funValue> normcall methodcall callsuffix
%type <exprsValue> elist
%type <indexElemValue> indexedelem
%type <indexValue> indexed
%type <sForPrefixValue> forprefix

%right ASSIGN
%left OR
%left AND
%nonassoc EQUALS NEQUALS
%nonassoc GREATER GREATEREQ LESS LESSEQ
%left PLUS MINUS
%left ASTERISK SLASH MODULO
%right NOT PLUSPLUS MINUSMINUS UMINUS
%left DOT DOUBLE_DOT
%left BRACKET_OPEN BRACKET_CLOSE
%left PARENTHESIS_OPEN PARENTHESIS_CLOSE

%nonassoc ELSE

%%

program: stmt program {
        DBG("program -> stmt program" << endl);
    }
	  |	{ DBG( "program -> empty " << endl); }
        ;

stmt: expr SEMICOLON {
      DBG( "stmt -> expr;" << endl);
      $$ = $1;
    }
    |ifstmt {
      DBG( "stmt -> ifstmt" << endl); 
      $$ = $1;
    }
    |whilestmt {
      DBG( "stmt -> whilestmt" << endl); 
      $$ = $1;
    }
    |forstmt {
      DBG( "stmt -> forstmt" << endl); 
      $$ = $1;
    }
    |returnstmt {
      DBG( "stmt -> returnstmt" << endl); 
      if (!insideFunc) cout << "ERROR: return outside a function" << endl;
      $$ = $1;
    }
    |BREAK SEMICOLON {
      DBG( "stmt -> break;" << endl); 
      if (!loopcounter.get()) cout << "ERROR: break outside a loop" << endl;
      $$ = new expr(special_e);
      ((expr*)$$)->newBreak(al->nextQuadLabel());
      al->emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
    }
    |CONTINUE SEMICOLON {
      DBG( "stmt -> continue;" << endl); 
      if (!loopcounter.get()) cout << "ERROR: continue outside a loop" << endl;
      $$ = new expr(special_e);
      ((expr*)$$)->newCont(al->nextQuadLabel());
      al->emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
    }
    |block {
      DBG( "stmt -> block" << endl); 
      $$ = $block;
    }
    |funcdef {
      DBG( "stmt -> funcdef" << endl); 
      $$ = $1;
    }
    |SEMICOLON {
      DBG( "stmt -> ;" << endl); 
      $$ = new expr(special_e);
    }
    ;

expr: assignexpr {DBG("expr-> assignexpr"<< endl); $$ = $1; }
    |expr PLUS expr {
      DBG( "expr -> expr + expr"<<endl);
      $$ = al->arithop(OPCODE_ADD, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr MINUS expr {
      DBG( "expr -> expr - expr"<<endl);
      $$ = al->arithop(OPCODE_SUB, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr ASTERISK expr {
      DBG( "expr -> expr * expr"<<endl);
      $$ = al->arithop(OPCODE_MUL, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr SLASH expr {
      DBG( "expr -> expr / expr"<<endl);
      $$ = al->arithop(OPCODE_DIV, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr MODULO expr {
      DBG( "expr -> expr % expr"<<endl);
      $$ = al->arithop(OPCODE_MOD, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr GREATER expr {
      DBG( "expr -> expr > expr"<<endl);
       $$ = al->relop(OPCODE_IF_GREATER, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr GREATEREQ expr {
      DBG( "expr -> expr >= expr"<<endl);
       $$ = al->relop(OPCODE_IF_GREATEREQ, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr LESS expr {
      DBG( "expr -> expr < expr"<<endl);
       $$ = al->relop(OPCODE_IF_LESS, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr LESSEQ expr {
      DBG( "expr -> expr <= expr"<<endl);
       $$ = al->relop(OPCODE_IF_LESSEQ, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr EQUALS expr {
      DBG( "expr -> expr == expr"<<endl);
       $$ = al->relop(OPCODE_IF_EQ, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr NEQUALS expr {
      DBG( "expr -> expr != expr"<<endl);
       $$ = al->relop(OPCODE_IF_NOTEQ, (expr*)$1, (expr*)$3, yylineno);
    }
    |expr AND expr {
        DBG( "expr -> expr && expr"<<endl);
        $$ = al->boolop(OPCODE_AND,(expr*)$1,(expr*)$3, yylineno);	     
      }
    |expr OR expr {
        DBG( "expr -> expr || expr"<<endl);
        $$ = al->boolop(OPCODE_OR,(expr*)$1,(expr*)$3, yylineno);   
      }
    |term  {
      DBG("expr-> term"<< endl);
      $$ = $1;
    }
    ;
   
term: PARENTHESIS_OPEN expr PARENTHESIS_CLOSE{
        DBG( "term -> (expr)" << endl);
        $term = $expr;
    }
    |MINUS expr %prec UMINUS {
      DBG( "term -> -expr"<<endl);
      al->checkuminus((expr*)$expr);
      $term = new expr(arithexpr_e);
      ((expr*)$term)->sym = al->tempVars->newTemp(map);
      al->emit(new quad(OPCODE_UMINUS, (expr*)$term, (expr*)$expr, yylineno));
    }
    |NOT expr	{
      DBG(  "term -> !expr"<<endl);
      $$ = al->boolop(OPCODE_NOT,(expr*)$2,yylineno);  
    }
    |PLUSPLUS lvalue2{ 
      DBG(  "term -> ++lvalue2" <<endl); 
      if (((expr*)$lvalue2)->type == tableitem_e){
          $term = al->emit_iftableitem((expr*)$lvalue2, yylineno);
          al->emit(new quad(OPCODE_ADD, (expr*)$term, new expr((double)1), (expr*)$term, yylineno));
          al->emit(new quad(OPCODE_TABLESETELEM, (expr*)$lvalue2, ((expr*)$lvalue2)->index, (expr*)$term, yylineno));
      }
      else{
          al->emit(new quad(OPCODE_ADD, (expr*)$lvalue2, new expr((double)1), (expr*)$lvalue2, yylineno));
          $term = new expr(arithexpr_e);
          ((expr*)$term)->sym = al->tempVars->newTemp(map);
          al->emit(new quad(OPCODE_ASSIGN, (expr*)$term, (expr*)$lvalue2, yylineno));
      }

    }
    |lvalue2 PLUSPLUS{
      DBG(  "term -> lvalue2++" <<endl);
      $term = new expr(var_e);
      ((expr*)$term)->sym = al->tempVars->newTemp(map);

      if (((expr*)$lvalue2)->type == tableitem_e){
          expr* value = al->emit_iftableitem((expr*)$lvalue2, yylineno);
          al->emit(new quad(OPCODE_ASSIGN, value, (expr*)$term, yylineno));
          al->emit(new quad(OPCODE_ADD, value, new expr((double)1), value, yylineno));
          al->emit(new quad(OPCODE_TABLESETELEM, (expr*)$lvalue2, ((expr*)$lvalue2)->index, value, yylineno));
      }
      else{
          al->emit(new quad(OPCODE_ASSIGN, (expr*)$lvalue2, (expr*)$term, yylineno));
          al->emit(new quad(OPCODE_ADD, (expr*)$lvalue2, new expr((double)1), (expr*)$lvalue2, yylineno));
      }

    }
    |MINUSMINUS lvalue2 {
      DBG(  "term -> --lvalue2" << endl);
      if (((expr*)$lvalue2)->type == tableitem_e){
          $term = al->emit_iftableitem((expr*)$lvalue2, yylineno);
          al->emit(new quad(OPCODE_SUB, (expr*)$term, new expr((double)1), (expr*)$term, yylineno));
          al->emit(new quad(OPCODE_TABLESETELEM, (expr*)$lvalue2, ((expr*)$lvalue2)->index, (expr*)$term, yylineno));
      }
      else{
          al->emit(new quad(OPCODE_SUB, (expr*)$lvalue2, new expr((double)1), (expr*)$lvalue2, yylineno));
          $term = new expr(arithexpr_e);
          ((expr*)$term)->sym = al->tempVars->newTemp(map);
          al->emit(new quad(OPCODE_ASSIGN, (expr*)$lvalue2, (expr*)$term, yylineno));
      }
    }
    |lvalue2 MINUSMINUS {
      DBG(  "term -> lvalue2--"<< endl); 
      $term = new expr(var_e);
      ((expr*)$term)->sym = al->tempVars->newTemp(map);

      if (((expr*)$lvalue2)->type == tableitem_e){
          expr* value = al->emit_iftableitem((expr*)$lvalue2, yylineno);
          al->emit(new quad(OPCODE_ASSIGN, value, (expr*)$term, yylineno));
          al->emit(new quad(OPCODE_SUB, value, new expr((double)1), value, yylineno));
          al->emit(new quad(OPCODE_TABLESETELEM, (expr*)$lvalue2, ((expr*)$lvalue2)->index, value, yylineno));
      }
      else{
          al->emit(new quad(OPCODE_ASSIGN, (expr*)$lvalue2, (expr*)$term, yylineno));
          al->emit(new quad(OPCODE_SUB, (expr*)$lvalue2, new expr((double)1), (expr*)$lvalue2, yylineno));
      }
    }
    |primary { 
      DBG( "term -> primary" << endl); 
      $term = $primary;
    }
    ;
    
assignexpr: lvalue ASSIGN expr { 
      DBG( "assignexpr -> lvalue = expr" << endl); 
      
      SymbolEntry* entry = NULL;
      if ($1 != NULL)  entry = map->lookupAll(((expr*)$1)->sym->getName());
      //cout << "Looking up " << *$2 << " " << entry << endl; 
      if (entry != NULL && (entry->getType() == USERFUNC || entry->getType() == LIBFUNC)){
        cout << "ERROR: can't have function as lvalue" << endl;
        al->totalErrors++;
      }
      else{
        if (((expr*)$lvalue)->type == tableitem_e){
              al->emit(new quad(OPCODE_TABLESETELEM, (expr*)$lvalue, ((expr*)$lvalue)->index, (expr*)$expr, yylineno));
              $assignexpr = al->emit_iftableitem((expr*)$lvalue, yylineno);
              ((expr*)$assignexpr)->type = assignexpr_e;
        } 
        else{
            al->emit(new quad(OPCODE_ASSIGN, (expr*)$lvalue, (expr*)$expr, yylineno));
            $assignexpr = new expr(assignexpr_e);
            ((expr*)$assignexpr)->sym = al->tempVars->newTemp(map);
            al->emit(new quad(OPCODE_ASSIGN, (expr*)$assignexpr, (expr*)$lvalue, yylineno));
        }
      }
      //TODO
    }
	  ;
	  
primary:   lvalue {
       DBG("primary-> lvalue"<< endl); 
      $primary = (expr*)al->emit_iftableitem((expr*)$lvalue, yylineno);
     }
	  |call {
      DBG("primary-> call"<< endl); 
      $$ = $1;
    }
	  |objectdef {
      DBG("primary-> objectdef"<< endl);
      $$ = $1;
    }
	  |PARENTHESIS_OPEN funcdef PARENTHESIS_CLOSE 
      { 
        DBG( "( funcdef )" << endl); 
        $$ = $funcdef;
      }
	  |const {
      DBG("primary-> const"<< endl); 
      $$ = $1;
    }
	  ;
	  
lvalue: lvalue2{
      DBG( "lvalue2 -> lvalue2" << endl);
      $$ = $1;
    }
	  |member {
      DBG( "lvalue -> member" << endl);
      //D
      $$ = $1;
    }
	  ;

lvalue2: IDENTIFIER { 
      DBG( "lvalue2 -> IDENTIFIER" << endl); 
      SymbolEntry* entry = map->lookupAll(*$1);
      if (entry == NULL && !map->isLibraryName(*$1)) {
        entry = new SymbolEntry(*$1, map->currentScope == 0 ? GLOBAL_VAR : LOCAL_VAR, map->currentScope, yylineno);
        entry->setSpace(scopeOffset->currScopeSpace());
        entry->setOffset(scopeOffset->currScopeOffset());
        scopeOffset->inCurrScopeOffset();
        map->insert(entry);

      }
      else if (entry != NULL && insideFunc && entry->getScope() != 0 && entry->getScope() != map->currentScope 
        && entry->getType() != USERFUNC && entry->getType() != LIBFUNC) {
          cout << "ERROR: variable " << *$1 << " not visible inside function" << endl;
          al->totalErrors++;
      }
      //else if (map->isLibraryName(*$1)) cout << "ERROR: can't assign library name" << endl;
      //else if (entry->getType() == USERFUNC) cout << "ERROR: function already defined" << endl;

        $$ = al->lvalue_expr(entry);

        // 9 - 48
        // 10 - 18

    }
    |LOCAL IDENTIFIER { 
      DBG( "lvalue2 -> LOCAL IDENTIFIER" << endl); 
      SymbolEntry* entry = map->lookup(*$2);
      if (entry == NULL) {
          if (map->isLibraryName(*$2)) {
            cout << "ERROR: can't have variable with library name" << endl;
            al->totalErrors++;
          }
          else{

                entry = new SymbolEntry(*$2, LOCAL_VAR, map->currentScope, yylineno);
                entry->setSpace(scopeOffset->currScopeSpace());
                entry->setOffset(scopeOffset->currScopeOffset());
                scopeOffset->inCurrScopeOffset();
                map->insert(entry);
          }
      }
      else if (entry->getType() == USERFUNC || entry->getType() == LIBFUNC){
            cout << "ERROR: its a function";
            al->totalErrors++;
      }

      $$ = al->lvalue_expr(entry);

        // 9 - 48
    }
    |DOUBLE_COLON IDENTIFIER { 
      DBG( "lvalue2 -> ::IDENTIFIER" << endl); 
      SymbolEntry* entry;
      if (map->isLibraryName(*$2)) {
        cout << "ERROR: can't have global variable with library name" << endl;
        al->totalErrors++;
      }
      else if ((entry = map->lookup(*$2, 0)) == NULL){
          if (map->currentScope != 0) { cout << "ERROR: global id not found" << endl; al->totalErrors++; }
          //else if (map->currentScope == 0) { map->insert(new SymbolEntry(*$2, GLOBAL_VAR, 0, yylineno)); }
      }
      else{
            $$ = al->lvalue_expr(entry);
      }
      
    }
    ;
	
tableitem: lvalue DOT IDENTIFIER{
          DBG( "member -> lvalue.IDENTIFIER" << endl);
          $tableitem = al->member_item((expr*)$lvalue, *$IDENTIFIER, yylineno);
      }
      |
      lvalue BRACKET_OPEN expr BRACKET_CLOSE {
          DBG( "member -> lvalue[expr]" << endl);
          $lvalue = (expr*)al->emit_iftableitem((expr*)$lvalue, yylineno);
          $tableitem = new expr(tableitem_e, ((expr*)$lvalue)->sym, (expr*)$expr, NULL);
      }
      ;

member: tableitem
       |call DOT IDENTIFIER {
            DBG( "member -> call.IDENTIFIER" << endl);
            //D
            $$ = $1;
            ((expr*)$$)->index = new expr(*$IDENTIFIER);
       }
       |call BRACKET_OPEN expr BRACKET_CLOSE {
            DBG( "member -> call[expr]" << endl);
            //D
            $$ = $1;
            ((expr*)$$)->index = (expr*)$expr;
       }
       ;
       
call: call PARENTHESIS_OPEN elist PARENTHESIS_CLOSE {
          DBG( "call -> call(elist)" << endl);
          $$ = al->make_call((expr*)$1, *((vector<expr*>*)$elist), yylineno);
      }
     | lvalue callsuffix {
          DBG( "call -> lvalue callsuffix" << endl);
          if (((functioncall*)$callsuffix)->method){
            expr* self = (expr*)$lvalue;
            $lvalue = al->emit_iftableitem(al->member_item(self, *((functioncall*)$callsuffix)->name, yylineno), yylineno);
            ((functioncall*)$callsuffix)->elist->push_back(self);
          }
          $$ = al->make_call((expr*)$lvalue, *((vector<expr*>*)((functioncall*)$callsuffix)->elist), yylineno);
     }
     |PARENTHESIS_OPEN funcdef PARENTHESIS_CLOSE PARENTHESIS_OPEN elist PARENTHESIS_CLOSE {
          DBG( "call -> (funcdef) (elist)" << endl);
          expr* func = new expr(programfunc_e);
          func->sym = (SymbolEntry*)$funcdef;
          $call = al->make_call(func, *((vector<expr*>*)$elist), yylineno);
     }
      ;
   
callsuffix: normcall {
        DBG( "callsuffix -> normcall" << endl);
        $callsuffix = $normcall;
      }
	   |methodcall {
        DBG( "callsuffix -> methodcall" << endl);
        $callsuffix = $methodcall;
     }
	   ;

normcall: PARENTHESIS_OPEN elist PARENTHESIS_CLOSE {
          DBG( "normcall -> (elist)" << endl);
          $$ = new functioncall();
          ((functioncall*)$normcall)->elist = (vector<expr*>*)$elist;
          ((functioncall*)$normcall)->method = false;
          ((functioncall*)$normcall)->name = new string("");
      }
	 ;

methodcall: DOUBLE_DOT IDENTIFIER PARENTHESIS_OPEN elist PARENTHESIS_CLOSE {
          DBG( "methodcall -> ..IDENTIFIER (elist)" << endl);
          $$ = new functioncall();
          ((functioncall*)$methodcall)->elist = (vector<expr*>*)$elist;
          ((functioncall*)$methodcall)->method = true;
          ((functioncall*)$methodcall)->name = new string(*$IDENTIFIER);
      }
	    ;

elist: expr {
        DBG( "elist -> expr" << endl);
        $$ = new vector<expr*>;
        ((vector<expr*>*)$$)->push_back((expr*)$expr);
      }
      |expr COMMA elist{
        DBG( "elist -> expr, elist" << endl);
        $$ = $3;
        ((vector<expr*>*)$$)->insert(((vector<expr*>*)$$)->begin(), (expr*)$expr);
      }
      | {
        DBG( "elist -> empty");
        $$ = new vector<expr*>;
      }
      ;
      
objectdef: BRACKET_OPEN elist BRACKET_CLOSE {
      DBG( "objectdef -> [elist]" << endl);
      expr* t = new expr(newtable_e);
      t->sym = al->tempVars->newTemp(map);
      al->emit(new quad(OPCODE_TABLECREATE, t, yylineno));

      for (int i = 0; i < ((vector<expr*>*)$elist)->size();i++){
          al->emit(new quad(OPCODE_TABLESETELEM, (*((vector<expr*>*)$elist))[i], t, new expr((double)i), yylineno));
      }
      $objectdef = t;

    }
	  |BRACKET_OPEN indexed BRACKET_CLOSE {
      DBG( "objectdef -> [indexed]" << endl);
      expr* t = new expr(newtable_e);
      t->sym = al->tempVars->newTemp(map);
      al->emit(new quad(OPCODE_TABLECREATE, t, yylineno));

      for (int i = 0; i < ((vector<indexedElemExprs*>*)$indexed)->size();i++){
          al->emit(new quad(OPCODE_TABLESETELEM, t, (*(vector<indexedElemExprs*>*)$indexed)[i]->arg2, (*(vector<indexedElemExprs*>*)$indexed)[i]->arg1, yylineno));
          //TODO
      }

      $objectdef = t;
    }
	  ;

indexed: indexedelem {
    DBG( "indexed -> indexedelem" << endl);
    //D
    $$ = new vector<indexedElemExprs*>();
    ((vector<indexedElemExprs*>*)$$)->push_back((indexedElemExprs*)$1);
  }
	|indexed COMMA indexedelem {
    DBG( "indexed -> indexed, indexedelem" << endl);
    //D
    $$ = $1;
    ((vector<indexedElemExprs*>*)$$)->insert(((vector<indexedElemExprs*>*)$$)->begin(), (indexedElemExprs*)$3);
  }

	;
	
indexedelem: CURLY_BRACKET_OPEN expr COLON expr CURLY_BRACKET_CLOSE {
      DBG( "indexedelem -> { expr : expr }" << endl);
      //D
      $$ = new indexedElemExprs();
      ((indexedElemExprs*)$$)->arg1 = (expr*)$2;
      ((indexedElemExprs*)$$)->arg2 = (expr*)$4;
  }
	    ;

stmtloop: stmtloop stmt {
    DBG( "stmtloop -> stmtloop stmt" << endl);
    $$ = $1;
    ((expr*)$$)->insertBreaks(((expr*)$stmt)->breaklist);
    //((expr*)$$)->insertBreaks(((expr*)$1)->breaklist);
    ((expr*)$$)->insertConts(((expr*)$stmt)->contlist);
    //((expr*)$$)->insertConts(((expr*)$1)->contlist);
  }
	|stmt {
    DBG( "stmtloop -> stmt" << endl);
    $$ = $1;
  }
	;

block: CURLY_BRACKET_OPEN { if (!scopeFunctionInc) { map->currentScope++; } scopeFunctionInc = false; } stmtloop CURLY_BRACKET_CLOSE{
        DBG( "block -> { stmtloop }" << endl);
        map->hide(map->currentScope--); 
        $$ = $stmtloop;
      }
      |CURLY_BRACKET_OPEN { if (!scopeFunctionInc) map->currentScope++; } CURLY_BRACKET_CLOSE{
        DBG( "block -> {}" << endl);
        map->hide(map->currentScope--);
        $$ = new expr(special_e);
      }
      ;

funcname: IDENTIFIER {
      $funcname = $IDENTIFIER;
    }
    | {
      $funcname = new string();
      *$funcname = al->tempVars->newTempFuncName();
    };


funcprefix: FUNCTION funcname{
      if (map->isLibraryName(*$2)) {
        cout << "ERROR: can't declare function with a library name" << endl; 
        al->totalErrors++;
      }
      
      if (($$ = map->lookup(*$2, map->currentScope - 1)) != NULL) {
           cout << "ERROR: name already defined in the same scope" << endl;
           al->totalErrors++;
      }
      else {
          SymbolEntry* entry = new SymbolEntry(*$funcname, USERFUNC, map->currentScope - 1, yylineno);
          al->emit(new quad(OPCODE_FUNCSTART, al->lvalue_expr(entry), al->nextQuadLabel(), yylineno));
          map->insert(entry);       
        
          //entry->setArguments(tempArgs[tempArgs.size() - 1]);
          //tempArgs.pop_back();
          $$ = entry;
        }      
        scopeOffset->push();
        scopeOffset->enterScopeSpace();
        scopeOffset->resetFormalArgsOffset();
  }

funcargs: PARENTHESIS_OPEN idlist PARENTHESIS_CLOSE{
        scopeOffset->enterScopeSpace();
        scopeOffset->resetFunctionLocalOffset();
};

funcbody: block 
  {
        scopeOffset->exitScopeSpace();
  };

funcblockstart: { loopcounter.push(0); } // 11-22

funcblockend: { loopcounter.pop(); } // 11-22

      
funcdef: funcprefix funcargs funcblockstart funcbody funcblockend
      {
           scopeOffset->exitScopeSpace();
           //totallocas - functionLocalsOffset;
            scopeOffset->pop();
            $funcdef = al->lvalue_expr((SymbolEntry*)$funcprefix);
           al->emit(new quad(OPCODE_FUNCEND, (expr*)$funcdef, yylineno));
      }
	;
	
const: INTEGER {
        DBG( "const -> INT" << endl);
        $$ = new expr((double)$1);
      }
	   |FLOAT {
        DBG( "const -> FLOAT" << endl);
        $$ = new expr((double)$1);
      }
     |STRING {
        DBG( "const -> STRING" << endl);
        $$ = new expr(*$1);
      }
     |NIL {
        DBG( "const -> NIL" << endl);
        $$ = new expr(nil_e);
      }
     |TRUE {
        DBG( "const -> TRUE" << endl);
        $$ = new expr(true);
      }
     |FALSE {
        DBG( "const -> FALSE" << endl);
        $$ = new expr(false);
      }
     ;
      
idlist2: IDENTIFIER { 
        DBG( "idlist2 -> IDENTIFIER" << endl); 
        if (map->isLibraryName(*$1)){
          tempArgs.push_back(vector<string>());
          cout << "ERROR: function variable can't have the same name as a library function" << endl;
          al->totalErrors++;
        }
        else {
          map->insert(new SymbolEntry(*$1, FORMAL_VAR, map->currentScope, yylineno));
          tempArgs.push_back({*$1});
        }
        //TODO
      }
      |idlist2 COMMA IDENTIFIER
      { 
        DBG( "idlist2 -> idlist2, IDENTIFIER" << endl); 
        if (map->lookup(*$3) != NULL) {
          cout << "ERROR: variable already defined" << endl;
          al->totalErrors++;
        }
        else if (map->isLibraryName(*$3)){
          cout << "ERROR: function variable can't have the same name as a library function" << endl;
          al->totalErrors++;
        }
        else {        
          map->insert(new SymbolEntry(*$3, FORMAL_VAR, map->currentScope, yylineno));
          (tempArgs[tempArgs.size() - 1]).push_back(*$3);
        }
        //TODO
      }
      ;

idlist: idlist2 {
          DBG( "idlist -> idlist2" << endl); 
          //TODO
        }
        | {
          DBG( "idlist -> empty" << endl); 
          tempArgs.push_back(vector<string>());        
          //TODO
        }
        ;
     
ifprefix: IF PARENTHESIS_OPEN expr PARENTHESIS_CLOSE{
        al->emit(new quad(OPCODE_IF_EQ, NULL, (expr*)$expr, new expr(true), al->nextQuadLabel() + 2, yylineno));
        $ifprefix = al->nextQuadLabel();
        al->emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
    }

elseprefix: ELSE {
        $elseprefix = al->nextQuadLabel();
        al->emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
    }

ifstmt: ifprefix stmt {
          DBG( "ifstmt -> if (expr) stmt" << endl);
          al->patchLabel($ifprefix, al->nextQuadLabel());
          //D
          $$ = $stmt;
      }
      | ifprefix stmt elseprefix stmt {
        DBG( "ifstmt -> if (expr) stmt else stmt" << endl);
        al->patchLabel($ifprefix, $elseprefix + 1);
        al->patchLabel($elseprefix, al->nextQuadLabel());
        //D
        $$ = $2;
        ((expr*)$$)->insertBreaks(((expr*)$4)->breaklist);
        ((expr*)$$)->insertConts(((expr*)$4)->contlist);
      }
      ;

whilestart: WHILE{
        $whilestart = al->nextQuadLabel();
    }

whilecond: PARENTHESIS_OPEN expr PARENTHESIS_CLOSE{
        DBG( "whilecond -> (expr)" << endl);
        al->emit(new quad(OPCODE_IF_EQ, NULL, (expr*)$expr, new expr(true), al->nextQuadLabel() + 2, yylineno));
        $whilecond = al->nextQuadLabel();
        al->emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));
    }

loopstart: { ++loopcounter; }
loopend: { --loopcounter; }
loopstmt: loopstart stmt loopend { $loopstmt = $stmt; }

whilestmt: whilestart whilecond loopstmt
      {
        DBG( "whilestmt -> whilestart whilecond loopstmt" << endl);
        al->emit(new quad(OPCODE_JUMP, NULL, $whilestart, yylineno));
        al->patchLabel($whilecond, al->nextQuadLabel());
        al->patchLabels(((expr*)$loopstmt)->breaklist, al->nextQuadLabel());
        al->patchLabels(((expr*)$loopstmt)->contlist, $whilestart);
        //D
        $$ = $loopstmt;
      }
	  ;

N: {$N = al->nextQuadLabel(); al->emit(new quad(OPCODE_JUMP, NULL, 0, yylineno));}

M: {$M = al->nextQuadLabel();}

forprefix: FOR PARENTHESIS_OPEN elist SEMICOLON M expr SEMICOLON{
        $$ = new sForPrefix();
        ((sForPrefix*)$forprefix)->test = $M;
        ((sForPrefix*)$forprefix)->enter = al->nextQuadLabel();
        al->emit(new quad(OPCODE_IF_EQ, (expr*)$expr, new expr(true), 0, yylineno));
    }

forstmt: forprefix N elist PARENTHESIS_CLOSE N loopstmt N
    {
      DBG( "forstmt -> for (elist;expr;elist) stmt" << endl);
      al->patchLabel(((sForPrefix*)$forprefix)->enter, $5 + 1);
      al->patchLabel($2, al->nextQuadLabel());
      al->patchLabel($5, ((sForPrefix*)$forprefix)->test);
      al->patchLabel($7, $2 + 1);
      al->patchLabels(((expr*)$loopstmt)->breaklist, al->nextQuadLabel());
      al->patchLabels(((expr*)$loopstmt)->contlist, $2 + 1);
      //D
      $$ = $loopstmt;
    }
	;

returnstmt: RETURN SEMICOLON {
        DBG( "returnstmt -> return; :" << endl);
        al->emit(new quad(OPCODE_RET, NULL, yylineno));
        //D
        $$ = new expr(special_e);
    }
	  |RETURN expr SEMICOLON {
        DBG( "returnstmt -> return expr;" << endl);
        al->emit(new quad(OPCODE_RET, (expr*)$expr, yylineno));
        //D
        $$ = $expr;
    }
	  ;
%%

int yyerror(string yaccProvidedMessage){
	cout << yaccProvidedMessage << ": at line " << yylineno << ",before token: " << yytext << endl;
  return 0;
}

int main(int argc, char** argv){
	if (argc > 1){
    yyin = fopen (argv[1] , "r");
  }
  else yyin = stdin;

  if (argc > 2){
    yyout = fopen (argv[2] , "w");
  }
  else yyout = stdout;
     
  al = new alpha();
  map = al->map;

	yyparse();

  map->print();
  al->writeToFile();

	return 0;
}
