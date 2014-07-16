// Try using flex/bison, EG: http://research.engineering.wustl.edu/~beardj/FlexBisonC++.html


#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
 
#include <stdio.h>
 
int yyparse(SExpression **expression, yyscan_t scanner);
 
SExpression *getAST(const char *expr)
{
    SExpression *expression;
    yyscan_t scanner;
    YY_BUFFER_STATE state;
 
    if (yylex_init(&scanner)) {
        // couldn't initialize
        return NULL;
    }
 
    state = yy_scan_string(expr, scanner);
 
    if (yyparse(&expression, scanner)) {
        // error parsing
        return NULL;
    }
 
    yy_delete_buffer(state, scanner);
 
    yylex_destroy(scanner);
 
    return expression;
}
 
int evaluate(SExpression *e)
{
    switch (e->type) {
        case eVALUE:
            printf("LITERAL %d\n", e->value);
            return e->value;
        case eMULTIPLY:
            printf("MUL \n");
            return evaluate(e->left) * evaluate(e->right);
        case ePLUS:
            printf("ADD \n");
            return evaluate(e->left) + evaluate(e->right);
        default:
            // shouldn't be here
            return 0;
    }
}
 
int main(void)
{
    SExpression *e = NULL;
    char test[]=" 4 + 2*10 + 3*( 5 + 1 )";
    int result = 0;
 
    e = getAST(test);
 
    result = evaluate(e);
 
    printf("Result of '%s' is %d\n", test, result);
 
    deleteExpression(e);
 
    return 0;
}
