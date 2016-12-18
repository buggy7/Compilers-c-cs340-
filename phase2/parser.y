%{
	#include <stdio.h>
	#include <iostream>
	#include <string>
	#include <vector>
	#include "SymbolTable.h"

  #define DEBUG

  #ifdef DEBUG
  #define DBG( msg ) cout << msg 
  #else
  #define DBG( msg )
  #endif

	int yyerror(string yaccProvidedMessage);

	extern int yylex(void);
	extern int yylineno;
	extern char* yytext;
	extern FILE* yyin;
	extern FILE* yyout;

	using namespace std;

  SymbolMap* map;
  bool scopeFunctionInc = false;
  int insideFunc = 0;
  int insideLoop = 0;

  vector<vector<string>> tempArgs;

%}

%start program
%error-verbose

%union
{
	int intValue;
	string* stringValue;
	float floatValue;
}


%token <intValue> INTEGER
%token <floatValue> FLOAT
%token <stringValue> IDENTIFIER
%token <stringValue> STRING

%token <stringValue> IF ELSE WHILE AND NOT RETURN LOCAL TRUE FALSE NIL OR FOR FUNCTION CONTINUE BREAK 
%token <stringValue> ASSIGN PLUS ASTERISK MINUS SLASH PLUSPLUS MINUSMINUS MODULO EQUALS NEQUALS GREATER LESS GREATEREQ LESSEQ
%token <stringValue> CURLY_BRACKET_OPEN CURLY_BRACKET_CLOSE BRACKET_OPEN BRACKET_CLOSE PARENTHESIS_CLOSE PARENTHESIS_OPEN
%token <stringValue> SEMICOLON DOUBLE_COLON COLON DOT DOUBLE_DOT COMMA

%type <stringValue> program stmt expr ifstmt whilestmt forstmt returnstmt block funcdef assignexpr term primary lvalue member call callsuffix
%type <stringValue> normcall methodcall elist objectdef indexed indexedelem const idlist stmtloop

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
    }
    |ifstmt {
      DBG( "stmt -> ifstmt" << endl); 
    }
    |whilestmt {
      DBG( "stmt -> whilestmt" << endl); 
    }
    |forstmt {
      DBG( "stmt -> forstmt" << endl); 
    }
    |returnstmt {
      DBG( "stmt -> returnstmt" << endl); 
      if (!insideFunc) cout << "ERROR: return outside a function" << endl;
    }
    |BREAK SEMICOLON {
      DBG( "stmt -> break;" << endl); 
      if (!insideLoop) cout << "ERROR: break outside a loop" << endl;
    }
    |CONTINUE SEMICOLON {
      DBG( "stmt -> continue;" << endl); 
      if (!insideLoop) cout << "ERROR: continue outside a loop" << endl;
    }
    |block {
      DBG( "stmt -> block" << endl); 
    }
    |funcdef {
      DBG( "stmt -> funcdef" << endl); 
    }
    |SEMICOLON {
      DBG( "stmt -> ;" << endl); 
    }
    ;
    
    
expr: assignexpr {DBG("expr-> assignexpr"<< endl); }
    |expr PLUS expr {DBG( "expr -> expr + expr"<<endl);}
    |expr MINUS expr {DBG( "expr -> expr - expr"<<endl); }
    |expr ASTERISK expr  {DBG( "expr -> expr * expr"<<endl);}
    |expr SLASH expr {DBG( "expr -> expr / expr"<<endl); }
    |expr MODULO expr {DBG( "expr -> expr % expr"<<endl);}
    |expr GREATER expr {DBG( "expr -> expr > expr"<<endl); }
    |expr GREATEREQ expr {DBG( "expr -> expr >= expr"<<endl); }
    |expr LESS expr {DBG( "expr -> expr < expr"<<endl); }
    |expr LESSEQ expr {DBG( "expr -> expr <= expr"<<endl); }
    |expr EQUALS expr {DBG( "expr -> expr == expr"<<endl); }
    |expr NEQUALS expr {DBG( "expr -> expr != expr"<<endl); }
    |expr AND expr {DBG( "expr -> expr && expr"<<endl); }
    |expr OR expr {DBG( "expr -> expr || expr"<<endl);}
    |term  {DBG("expr-> term"<< endl);}
    ;
   
term: PARENTHESIS_OPEN expr PARENTHESIS_CLOSE{
        DBG( "term -> (expr)" << endl);
    }
    |UMINUS expr {DBG( "term -> -expr"<<endl);}
    |NOT expr	{DBG(  "term -> !expr"<<endl); }
    |PLUSPLUS lvalue2{ DBG(  "term -> ++lvalue2" <<endl); }
    |lvalue2 PLUSPLUS{DBG(  "term -> lvalue2++" <<endl);}
    |MINUSMINUS lvalue2 {DBG(  "term -> --lvalue2" << endl);}
    |lvalue2 MINUSMINUS {DBG(  "term -> lvalue2--"<< endl); }
    |primary { DBG( "term -> primary" << endl); }
    ;
    
assignexpr: lvalue ASSIGN expr { 
      DBG( "assignexpr -> lvalue = expr" << endl); 
      
      SymbolEntry* entry = NULL;
      if ($2 != NULL)  entry = map->lookupAll(*$2);
      //cout << "Looking up " << *$2 << " " << entry << endl; //TODO
      if (entry != NULL && (entry->getType() == USERFUNC || entry->getType() == LIBFUNC)){
        cout << "ERROR: can't have function as lvalue" << endl;
      }
    }
	  ;
	  
primary:   lvalue { DBG("primary-> lvalue"<< endl); }
	  |call {DBG("primary-> call"<< endl); }
	  |objectdef {DBG("primary-> objectdef"<< endl);}
	  |PARENTHESIS_OPEN funcdef PARENTHESIS_CLOSE { DBG( "( funcdef )" << endl); }
	  |const {DBG("primary-> const"<< endl); }
	  ;
	  
lvalue: lvalue2{
      DBG( "lvalue2 -> lvalue2" << endl);
    }
	  |member {
      DBG( "lvalue -> member" << endl);
    }
	  ;

lvalue2: IDENTIFIER { 
      DBG( "lvalue2 -> IDENTIFIER" << endl); 
      SymbolEntry* entry = map->lookupAll(*$1);
      if (entry == NULL && !map->isLibraryName(*$1)) map->insert(new SymbolEntry(*$1, map->currentScope == 0 ? GLOBAL_VAR : LOCAL_VAR, map->currentScope, yylineno));
      else if (entry != NULL && insideFunc && entry->getScope() != 0 && entry->getScope() != map->currentScope 
        && entry->getType() != USERFUNC && entry->getType() != LIBFUNC) cout << "ERROR: variable " << *$1 << " not visible inside function" << endl;
      //else if (map->isLibraryName(*$1)) cout << "ERROR: can't assign library name" << endl;//TODO
      //else if (entry->getType() == USERFUNC) cout << "ERROR: function already defined" << endl;//TODO
    }
    |LOCAL IDENTIFIER { 
      DBG( "lvalue2 -> LOCAL IDENTIFIER" << endl); 
      /*if (map->lookup(*$2) != NULL) cout << "ERROR: ?" << endl; //TODO No error. Just refer to the Variable or Function
      else */ if (map->isLibraryName(*$2)) cout << "ERROR: can't have variable with library name" << endl;
      else map->insert(new SymbolEntry(*$2, LOCAL_VAR, map->currentScope, yylineno));

    }
    |DOUBLE_COLON IDENTIFIER { 
      DBG( "lvalue2 -> ::IDENTIFIER" << endl); 
      if (map->isLibraryName(*$2)) cout << "ERROR: can't have global variable with library name" << endl;
      else if (map->lookup(*$2, 0) == NULL){
          if (map->currentScope != 0) { cout << "ERROR: global id not found" << endl; }
          //else if (map->currentScope == 0) { map->insert(new SymbolEntry(*$2, GLOBAL_VAR, 0, yylineno)); }//TODO
      }
      
    }
    ;
	
member: lvalue DOT IDENTIFIER {
            DBG( "member -> lvalue.IDENTIFIER" << endl);
        }
       |lvalue BRACKET_OPEN expr BRACKET_CLOSE {
            DBG( "member -> lvalue[expr]" << endl);
       }
       |call DOT IDENTIFIER {
            DBG( "member -> call.IDENTIFIER" << endl);
       }
       |call BRACKET_OPEN expr BRACKET_CLOSE {
            DBG( "member -> call[expr]" << endl);
       }
       ;
       
call: call PARENTHESIS_OPEN elist PARENTHESIS_CLOSE {
          DBG( "call -> call(elist)" << endl);
      }
     | lvalue callsuffix {
          DBG( "call -> lvalue callsuffix" << endl);
     }
     |PARENTHESIS_OPEN funcdef PARENTHESIS_CLOSE PARENTHESIS_OPEN elist PARENTHESIS_CLOSE {
          DBG( "call -> (funcdef) (elist)" << endl);
     }
      ;
      
callsuffix: normcall {
        DBG( "callsuffix -> normcall" << endl);
      }
	   |methodcall {
        DBG( "callsuffix -> methodcall" << endl);
     }
	   ;

normcall: PARENTHESIS_OPEN elist PARENTHESIS_CLOSE {
          DBG( "normcall -> (elist)" << endl);
      }
	 ;

methodcall: DOUBLE_DOT IDENTIFIER PARENTHESIS_OPEN elist PARENTHESIS_CLOSE {
          DBG( "methodcall -> ..IDENTIFIER (elist)" << endl);
      }
	    ;

elist: expr {
        DBG( "elist -> expr" << endl);
      }
      |expr COMMA elist{
        DBG( "elist -> expr, elist" << endl);
      }
      | {
        DBG( "elist -> empty");
      }
      ;
      
objectdef: BRACKET_OPEN elist BRACKET_CLOSE {
      DBG( "objectdef -> [elist]" << endl);
    }
	  |BRACKET_OPEN indexed BRACKET_CLOSE {
      DBG( "objectdef -> [indexed]" << endl);
    }
	  ;

indexed: indexedelem {
    DBG( "indexed -> indexedelem" << endl);
  }
	|indexed COMMA indexedelem {
    DBG( "indexed -> indexed, indexedelem" << endl);
  }

	;
	
indexedelem: CURLY_BRACKET_OPEN expr COLON expr CURLY_BRACKET_CLOSE {
      DBG( "indexedelem -> { expr : expr }" << endl);
  }
	    ;

stmtloop:stmt stmtloop {
    DBG( "stmtloop -> stmt stmtloop" << endl);
  }
	|stmt {
    DBG( "stmtloop -> stmt" << endl);
  }
	;

block: CURLY_BRACKET_OPEN { if (!scopeFunctionInc) { map->currentScope++; } scopeFunctionInc = false; } stmtloop CURLY_BRACKET_CLOSE{
        DBG( "block -> { stmtloop }" << endl);
        map->hide(map->currentScope--); 
      }
      |CURLY_BRACKET_OPEN { if (!scopeFunctionInc) map->currentScope++; } CURLY_BRACKET_CLOSE{
        DBG( "block -> {}" << endl);
        map->hide(map->currentScope--);
      }
      ;
      
funcdef: FUNCTION PARENTHESIS_OPEN { map->currentScope++; scopeFunctionInc = true; insideFunc++; } idlist PARENTHESIS_CLOSE
      {
        SymbolEntry* entry = new SymbolEntry(" ", USERFUNC, map->currentScope - 1, yylineno);
        entry->setArguments(tempArgs[tempArgs.size() - 1]);
        tempArgs.pop_back();
        map->insert(entry);

      } block { 
        DBG( "funcdef -> function (idlist) block" << endl); 
        insideFunc--;
      }
	|	FUNCTION IDENTIFIER PARENTHESIS_OPEN { map->currentScope++; scopeFunctionInc = true; insideFunc++; } idlist PARENTHESIS_CLOSE 
      {
        if (map->isLibraryName(*$2)) cout << "ERROR: can't declare function with a library name" << endl; 
        else if (map->lookup(*$2, map->currentScope - 1) != NULL) {
          cout << "ERROR: name already defined in the same scope" << endl;
        }
        else {
          SymbolEntry* entry = new SymbolEntry(*$2, USERFUNC, map->currentScope - 1, yylineno);
          entry->setArguments(tempArgs[tempArgs.size() - 1]);
          tempArgs.pop_back();
          map->insert(entry);
        }      
      } block   { 
        DBG( "funcdef -> function IDENTIFIER (idlist) block" << endl); 
        insideFunc--;        
      }
	;
	
const: INTEGER {
        DBG( "const -> INT" << endl);
        $$ = new string(to_string($1));
      }
	   |FLOAT {
        DBG( "const -> FLOAT" << endl);
        $$ = new string(to_string($1));
      }
     |STRING {
        DBG( "const -> STRING" << endl);
      }
     |NIL {
        DBG( "const -> NIL" << endl);
      }
     |TRUE {
        DBG( "const -> TRUE" << endl);
      }
     |FALSE {
        DBG( "const -> FALSE" << endl);
      }
     ;
      
idlist2: IDENTIFIER { 
        DBG( "idlist2 -> IDENTIFIER" << endl); 
        if (map->isLibraryName(*$1)){
          tempArgs.push_back(vector<string>());
          cout << "ERROR: function variable can't have the same name as a library function" << endl;
        }
        else {
          map->insert(new SymbolEntry(*$1, FORMAL_VAR, map->currentScope, yylineno));
          tempArgs.push_back({*$1});
        }
      }
      |idlist2 COMMA IDENTIFIER
      { 
        DBG( "idlist2 -> idlist2, IDENTIFIER" << endl); 
        if (map->lookup(*$3) != NULL) cout << "ERROR: variable already defined" << endl;
        else if (map->isLibraryName(*$3)){
          cout << "ERROR: function variable can't have the same name as a library function" << endl;
        }
        else {        
          map->insert(new SymbolEntry(*$3, FORMAL_VAR, map->currentScope, yylineno));
          (tempArgs[tempArgs.size() - 1]).push_back(*$3);
        }
      }
      ;

idlist: idlist2 {
          DBG( "idlist -> idlist2" << endl); 
        }
        | {
          DBG( "idlist -> empty" << endl); 
          tempArgs.push_back(vector<string>());        
        }
        ;
      
ifstmt: IF PARENTHESIS_OPEN expr PARENTHESIS_CLOSE stmt {
          DBG( "ifstmt -> if (expr) stmt" << endl);
      }
      | IF PARENTHESIS_OPEN expr PARENTHESIS_CLOSE stmt ELSE stmt{
        DBG( "ifstmt -> if (expr) stmt else stmt" << endl);
      }
      ;

whilestmt: WHILE PARENTHESIS_OPEN expr PARENTHESIS_CLOSE { insideLoop++; } stmt
      {
        DBG( "whilestmt -> while (expr) stmt" << endl);
        insideLoop--;
      }
	  ;

forstmt: FOR PARENTHESIS_OPEN elist SEMICOLON expr SEMICOLON elist PARENTHESIS_CLOSE { insideLoop++; } stmt
    {
      DBG( "forstmt -> for (elist;expr;elist) stmt" << endl);
      insideLoop--;
    }
	;

returnstmt: RETURN SEMICOLON {
        DBG( "returnstmt -> return; :" << endl);
    }
	  |RETURN expr SEMICOLON {
        DBG( "returnstmt -> return expr;" << endl);
    }
	  ;
%%

int yyerror(string yaccProvidedMessage){
	cout << yaccProvidedMessage << ": at line " << yylineno << ",before token: " << yytext << endl;
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
     
  map = new SymbolMap();

	yyparse();

  map->print();

	return 0;
}
