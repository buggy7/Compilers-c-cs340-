%{
#include <iostream>
#include <string>
using namespace std;
#include "parser.hpp"

string ourString;
int counterBegin=0;

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
%x STRINGS

%%


"\"" {
 BEGIN (STRINGS);
 ourString = "";
}
<STRINGS><<EOF>>   { 
                    cout << endl << "ERROR, string not closed before EOF" << endl;
                    yyterminate();
                  } 
<STRINGS>"\\\"" { ourString += "\""; }
<STRINGS>"\\n" { ourString += "\n"; }
<STRINGS>"\\t" { ourString += "\t"; }
<STRINGS>"\\\\" { ourString += "\\"; }
<STRINGS>"\\"[^nt\\\"] { cout << endl << "WARNING at line " << yylineno << ", uknown escape sequence character " << yytext << endl; ourString += yytext; }
<STRINGS>[^\\\"]* { ourString += yytext; }

<STRINGS>"\""      {
                      //insert((list<alpha_token_t*>*) yylval, STRINGS, ourString,yylineno);
                      yylval.stringValue = new string(ourString);
                      BEGIN(INITIAL);
                      return STRING;
                  }
<STRINGS>.        { cout << endl << "WARNING at line " << yylineno <<", unrecognized rule " << yytext << endl;}



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
		                    //insert((list<alpha_token_t*>*) yylval, MULTI_LINE_COMMENT,"",yylineno);
				                BEGIN(INITIAL);
                      }
                      //else insert((list<alpha_token_t*>*) yylval, EMBEDDED_COMMMENT,"",yylineno);
                  }

<COMMENT><<EOF>>  { 
                    cout << endl << "ERROR around line " << yylineno <<", comment not closed before EOF" << endl;
                    yyterminate();
                  }	


{keyword_if}	    { return IF; }
{keyword_else}		{ return ELSE; }
{keyword_while}		{ return WHILE; }
{keyword_for}		  { return FOR; }
{keyword_function}	{ return FUNCTION; }
{keyword_return}	{ return RETURN; }
{keyword_continue}	{ return CONTINUE; }
{keyword_break}		{ return BREAK; }
{keyword_and}		  { return AND; }
{keyword_local}		{ return LOCAL; }
{keyword_true}		{ return TRUE; }
{keyword_false}		{ return FALSE; }
{keyword_nil}		  { return NIL; }
{keyword_not}		  { return NOT; }
{keyword_or}		  { return OR; }

{int}             { yylval.intValue = stoi(yytext); return INTEGER; }
{float}	          {	yylval.floatValue = stof(yytext); return FLOAT; }
{id}              { yylval.stringValue = new string(yytext); return IDENTIFIER; }
{comment1}        { }
{newline}         { }

{assign}          { return ASSIGN; }
{plus}            { return PLUS; }
{asterisk}        { return ASTERISK; }
{minus}           { return MINUS; }
{slash}           { return SLASH; }
{plusplus}        { return PLUSPLUS; }
{minusminus}      { return MINUSMINUS; }
{modulo}          { return MODULO; }
{equals}          { return EQUALS; }
{nequals}         { return NEQUALS; }
{greater}         { return GREATER; }
{less}            { return LESS; }
{greatereq}       { return GREATEREQ; }
{lesseq}          { return LESSEQ; }

{curlybracketOpen}  { return CURLY_BRACKET_OPEN; }
{curlybracketClose} { return CURLY_BRACKET_CLOSE; }
{bracketOpen}       { return BRACKET_OPEN; }
{bracketClose}      { return BRACKET_CLOSE; }
{parenthesisOpen}   { return PARENTHESIS_OPEN; }
{parenthesisClose}  { return PARENTHESIS_CLOSE; }
{semicolon}         { return SEMICOLON; }
{comma}             { return COMMA; }
{colon}             { return COLON; }
{doubleColon}       { return DOUBLE_COLON; }
{dot}               { return DOT; }
{doubleDot}         { return DOUBLE_DOT; }
{rest} 		          { cout << endl << "WARNING at line " << yylineno <<", unrecognized rule " << yytext << endl;}

%%
