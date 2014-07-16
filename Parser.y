%{
 
/*
 * Parser.y file
 * To generate the parser run: "bison Parser.y"
 */
 
#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
 
int yyerror(SExpression **expression, yyscan_t scanner, const char *msg) {
    // Add error handling routine as needed
    fprintf(stderr,"Error:%s\n",msg); return 0;
}
 
%}
 
%code requires {
 
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif
 
}
 
%output  "Parser.c"
%defines "Parser.h"
 
%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { SExpression **expression }
%parse-param { yyscan_t scanner }
 
%union {
    int value;
    SExpression *expression;
    SStatement *statement;
    SStatements *statements;
}
 
%left '+' TOKEN_PLUS
%left '*' TOKEN_MULTIPLY
 
%token TOKEN_PRINT
%token TOKEN_LPAREN
%token TOKEN_RPAREN
%token TOKEN_PLUS
%token TOKEN_MULTIPLY
%token <value> TOKEN_NUMBER
 
%type <expression> expr
%type <statement> stmt
%type <statements> stmts
 
%%

// See http://www.gnu.org/software/bison/manual/bison.html#Rpcalc-Rules
//input
//    : expr { *expression = $1; }
input : stmts
    ;

// See http://stackoverflow.com/questions/1655166/using-bison-to-parse-list-of-elements
stmts: /* empty */ { $$ = NULL; }
  | stmts stmt { if ($1 == NULL) 
                   $$ = initStmts($2); 
                 else
                   addStmt((SStatements *)$1, $2); }
  ;

//stmt: TOKEN_PRINT expr { *expression = $1; }
stmt: TOKEN_PRINT expr { $$ = newStmt(sPRINT, $2); }
  ;

expr
    : expr[L] TOKEN_PLUS expr[R] { $$ = createOperation( ePLUS, $L, $R ); }
    | expr[L] TOKEN_MULTIPLY expr[R] { $$ = createOperation( eMULTIPLY, $L, $R ); }
    | TOKEN_LPAREN expr[E] TOKEN_RPAREN { $$ = $E; }
    | TOKEN_NUMBER { $$ = createNumber($1); }
    ;
 
%%
