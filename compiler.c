// Try using flex/bison, EG: http://research.engineering.wustl.edu/~beardj/FlexBisonC++.html


#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
 
#include "vm.h"
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
 
int evaluate(SExpression *e, FILE *out)
{
    unsigned char instr;

    switch (e->type) {
        case eVALUE:
            instr = INST_LITERAL;
            fwrite(&instr, sizeof(instr), 1, out);
            instr = (unsigned char)e->value;
            fwrite(&instr, sizeof(instr), 1, out);
          // TODO: write an Object here?
          //printf("LITERAL %d\n", e->value);
            return e->value;
        case eMULTIPLY:
            instr = INST_MUL;
            fwrite(&instr, sizeof(instr), 1, out);
            return evaluate(e->left, out) * evaluate(e->right, out);
        case ePLUS:
            instr = INST_ADD;
            fwrite(&instr, sizeof(instr), 1, out);
            return evaluate(e->left, out) + evaluate(e->right, out);
        default:
            // shouldn't be here
            return 0;
    }
}
 
int main(void)
{
    FILE *outf;
    SExpression *e = NULL;
    char test[]="PRINT 4 + 2*10 + 3*( 5 + 1 )";
    int result = 0;
 
    e = getAST(test);
 
    outf = fopen("out.bin", "wb"); // TODO: from cmd line
    result = evaluate(e, outf);
    fclose(outf);
 
    printf("Result of '%s' is %d\n", test, result);
 
    deleteExpression(e);
 
    return 0;
}
