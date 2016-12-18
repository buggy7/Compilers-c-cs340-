%{

#include <stdio.h>
#include <iostream>
#include <string>
#include <list>

#define YY_DECL int alpha_yylex(void* yylval)

using namespace std;

enum CATEGORIES {
  KEYWORD, OPERATOR, INTEGER, FLOAT, STRINGS, PUNCTUATION, IDENTIFIER, COMMENTS, MULTI_LINE_COMMENT, EMBEDDED_COMMMENT
};
string CATEGORIES_NAMES[] = { "KEYWORD", "OPERATOR", "INTEGER", "FLOAT", "STRING", "PUNCTUATION", "IDENTIFIER", "COMMENT", "MULTILINE COMMENT", "EMBEDDED COMMMENT" };

union Content{
    float f;
    int i;
};

typedef struct alpha_token_t {
  string content;
  Content value;
  unsigned int  numline;
  unsigned int  numToken;
  int    type;
  struct alpha_token_t *alpha_yylex;
}alpha_token_t;

string ourString;
int counterBegin=0;
int tokenNum=0;

void insert(list<alpha_token_t*> *ourList, int type, string content, int line);

%}


%option yylineno
%option noyywrap


id          		[a-zA-Z][a-zA-Z0-9_]*
int         		[0-9]+
comment1    		"//".*
newline     		[\n\t ]
float	      		[0-9]+\.[0-9]+
keyword_if	  	("if")
keyword_else 		("else") 
keyword_while 	("while")
keyword_for 		("for")
keyword_function 	("function")
keyword_return 	("return")
keyword_continue 	("continue")
keyword_break 	("break")
keyword_and 		("and")
keyword_not 		("not")
keyword_local 	("local")
keyword_true 		("true")
keyword_false 	("false")
keyword_nil 		("nil")
keyword_or			("or")
assign      "="
plus        "+"
asterisk    "*"
minus       "-"
slash       "/"
plusplus    "++"
minusminus  "--"
modulo      "%"
equals      "=="
nequals     "!="
greater     ">"
less        "<"
greatereq   ">="
lesseq      "<="
curlybracketOpen  "{"
curlybracketClose "}"
bracketOpen       "["
bracketClose      "]"
parenthesisOpen   "("
parenthesisClose  ")"
semicolon         ";"
comma             ","
colon             ":"
doubleColon       "::"
dot               "."
doubleDot         ".."
rest              .


%x COMMENT
%x STRING

%%


"\"" {
 BEGIN (STRING);
 ourString = "";
}
<STRING><<EOF>>   { 
                    cout << endl << "ERROR, string not closed before EOF" << endl;
                    yyterminate();
                  } 
<STRING>"\\\"" { ourString += "\""; }
<STRING>"\\n" { ourString += "\n"; }
<STRING>"\\t" { ourString += "\t"; }
<STRING>"\\\\" { ourString += "\\"; }
<STRING>"\\"[^nt\\\"] { cout << endl << "WARNING at line " << yylineno << ", uknown escape sequence character " << yytext << endl; ourString += yytext; }
<STRING>[^\\\"]* { ourString += yytext; }

<STRING>"\""      {
                      insert((list<alpha_token_t*>*) yylval, STRINGS, ourString,yylineno);
                      BEGIN(INITIAL);
                  }
<STRING>.        { cout << endl << "WARNING at line " << yylineno <<", unrecognized rule " << yytext << endl;}



"/*" {
	counterBegin++;   
	BEGIN(COMMENT);
}
<COMMENT>[^*/]*
<COMMENT>"/"+"*"  { counterBegin++; }  
<COMMENT>"*"+[^*/]*
<COMMENT>"/"+[^*/]*
<COMMENT>"*"+"/"  {    
                      counterBegin--;
                      if (counterBegin == 0){
		                    insert((list<alpha_token_t*>*) yylval, MULTI_LINE_COMMENT,"",yylineno);
				                BEGIN(INITIAL);
                      }
                      else insert((list<alpha_token_t*>*) yylval, EMBEDDED_COMMMENT,"",yylineno);
                  }

<COMMENT><<EOF>>  { 
                    cout << endl << "ERROR around line " << yylineno <<", comment not closed before EOF" << endl;
                    yyterminate();
                  }	


{keyword_if}	    { insert((list<alpha_token_t*>*) yylval, KEYWORD, "IF", yylineno); }
{keyword_else}		{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "ELSE", yylineno); }
{keyword_while}		{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "WHILE", yylineno); }
{keyword_for}		  { insert((list<alpha_token_t*>*) yylval, KEYWORD, "FOR", yylineno); }
{keyword_function}	{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "FUNCTION", yylineno); }
{keyword_return}	{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "RETURN", yylineno); }
{keyword_continue}	{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "CONTINUE", yylineno); }
{keyword_break}		{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "BREAK", yylineno); }
{keyword_and}		  { insert((list<alpha_token_t*>*) yylval, KEYWORD, "AND", yylineno); }
{keyword_local}		{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "LOCAL", yylineno); }
{keyword_true}		{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "TRUE", yylineno); }
{keyword_false}		{ insert((list<alpha_token_t*>*) yylval, KEYWORD, "FALSE", yylineno); }
{keyword_nil}		  { insert((list<alpha_token_t*>*) yylval, KEYWORD, "NIL", yylineno); }
{keyword_not}		  { insert((list<alpha_token_t*>*) yylval, KEYWORD, "NOT", yylineno); }
{keyword_or}		  { insert((list<alpha_token_t*>*) yylval, KEYWORD, "OR", yylineno); }

{int}             { insert((list<alpha_token_t*>*) yylval, INTEGER, yytext, yylineno); }
{float}	          {	insert((list<alpha_token_t*>*) yylval, FLOAT, yytext, yylineno); }
{id}              { insert((list<alpha_token_t*>*) yylval, IDENTIFIER, yytext, yylineno); }
{comment1}        { insert((list<alpha_token_t*>*) yylval, COMMENTS, "", yylineno); }
{newline}         { }

{assign}          { insert((list<alpha_token_t*>*) yylval, OPERATOR, "ASSIGN", yylineno); }
{plus}            { insert((list<alpha_token_t*>*) yylval, OPERATOR, "PLUS", yylineno); }
{asterisk}        { insert((list<alpha_token_t*>*) yylval, OPERATOR, "ASTERISK", yylineno); }
{minus}           { insert((list<alpha_token_t*>*) yylval, OPERATOR, "MINUS", yylineno); }
{slash}           { insert((list<alpha_token_t*>*) yylval, OPERATOR, "SLASH", yylineno); }
{plusplus}        { insert((list<alpha_token_t*>*) yylval, OPERATOR, "INCREMENT", yylineno); }
{minusminus}      { insert((list<alpha_token_t*>*) yylval, OPERATOR, "DECREMENT", yylineno); }
{modulo}          { insert((list<alpha_token_t*>*) yylval, OPERATOR, "MODULO", yylineno); }
{equals}          { insert((list<alpha_token_t*>*) yylval, OPERATOR, "EQUALS", yylineno); }
{nequals}         { insert((list<alpha_token_t*>*) yylval, OPERATOR, "NOT EQUALS", yylineno); }
{greater}         { insert((list<alpha_token_t*>*) yylval, OPERATOR, "GREATER THAN", yylineno); }
{less}            { insert((list<alpha_token_t*>*) yylval, OPERATOR, "LESS THAN", yylineno); }
{greatereq}       { insert((list<alpha_token_t*>*) yylval, OPERATOR, "GREATER THAN OR EQUAL", yylineno); }
{lesseq}          { insert((list<alpha_token_t*>*) yylval, OPERATOR, "LESS THAN OR EQUAL", yylineno); }

{curlybracketOpen}  { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "CURLY BRACKET OPEN", yylineno); }
{curlybracketClose} { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "CURLY BRACKET CLOSE", yylineno); }
{bracketOpen}       { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "BRACKET OPEN", yylineno); }
{bracketClose}      { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "BRACKET CLOSE", yylineno); }
{parenthesisOpen}   { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "PARENTHESIS OPEN", yylineno); }
{parenthesisClose}  { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "PARENTHESIS CLOSE", yylineno); }
{semicolon}         { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "SEMICOLON", yylineno); }
{comma}             { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "COMMA", yylineno); }
{colon}             { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "COLON", yylineno); }
{doubleColon}       { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "DOUBLE COLON", yylineno); }
{dot}               { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "DOT", yylineno); }
{doubleDot}         { insert((list<alpha_token_t*>*) yylval, PUNCTUATION, "DOUBLE DOT", yylineno); }
{rest} 		          { cout << endl << "WARNING at line " << yylineno <<", unrecognized rule " << yytext << endl;}

%%
void insert(list<alpha_token_t*> *ourList, int type, string content, int line){
	  alpha_token_t *token = new alpha_token_t();
	  token->numline=line;
    if (type == INTEGER){
        token->value.i = atoi(content.c_str());
    }
    else if (type == FLOAT){
        token->value.f = atof(content.c_str());
    }
    token->content= content;
	  token->type=type;
	  token->numToken = tokenNum++;
	  (*ourList).push_back(token);
}

void print_list(list<alpha_token_t*> *ourList){
  list<alpha_token_t*> ::iterator iIt;
  for (iIt = (*ourList).begin(); iIt != (*ourList).end(); ++iIt){
    cout << (*iIt)->numline << ":\t#" << (*iIt)->numToken << "\t" << CATEGORIES_NAMES[(*iIt)->type] << "\t";
    if ((*iIt)->type == INTEGER){
        cout << (*iIt)->value.i << endl;
    }
    else if ((*iIt)->type == FLOAT){
        cout << (*iIt)->value.f << endl;
    }
    else
        cout << (*iIt)->content << endl;
  }
}

int main(int argc, char** argv)
{

  if (argc > 1){
      yyin = fopen (argv[1] , "r");
  }
  else yyin = stdin;

  if (argc > 2){
      yyout = fopen (argv[2] , "w");
  }
  else yyout = stdout;
      
  list<alpha_token_t*> ourList;
  alpha_yylex(&ourList);

  print_list(&ourList);

  return 0;
}