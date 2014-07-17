// Try using flex/bison, EG: http://research.engineering.wustl.edu/~beardj/FlexBisonC++.html


#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
 
#include "vm.h"
#include <stdio.h>
 
int yyparse(SStatements **stmts, yyscan_t scanner);
 
SStatements *getAST(const char *expr)
{
    SStatements *stmts;
    yyscan_t scanner;
    YY_BUFFER_STATE state;
 
    if (yylex_init(&scanner)) {
        // couldn't initialize
        return NULL;
    }
 
    state = yy_scan_string(expr, scanner);
 
    if (yyparse(&stmts, scanner)) {
        // error parsing
        return NULL;
    }
 
    yy_delete_buffer(state, scanner);
 
    yylex_destroy(scanner);
 
    return stmts;
}

int evaluate(SExpression *e, FILE *out)
{
  unsigned char instr;

  switch (e->type) {
    case eVALUE:
      printf("AST LITERAL %d\n", e->value);
      instr = INST_LITERAL;
      fwrite(&instr, sizeof(instr), 1, out);
      instr = (unsigned char)e->value;
      fwrite(&instr, sizeof(instr), 1, out);
      break;

    case eMULTIPLY:
      printf("AST MUL\n");
      instr = INST_MUL;
      evaluate(e->left, out);
      evaluate(e->right, out);
      fwrite(&instr, sizeof(instr), 1, out);
      break;

    case ePLUS:
      printf("AST ADD\n");
      instr = INST_ADD;
      evaluate(e->left, out);
      evaluate(e->right, out);
      fwrite(&instr, sizeof(instr), 1, out);
      break;

    default:
      // shouldn't be here
      return 0;
  }

  return 0;
}

int evaluateStmt(SStatement *s, FILE *out) {
  unsigned char instr;
  switch (s->type) {
    case sPRINT:
      printf("AST PRINT\n");
      instr = INST_ADD;
      instr = INST_IO;
      evaluate(s->expr, out);
      fwrite(&instr, sizeof(instr), 1, out);
      break;
  }

  return 0;
}
 
int evaluateStmts(SStatements *ss, FILE *out) {
  if (ss == NULL) {
    printf("evaluateStmts - ss is NULL\n");
    return 0;
  }

  for (SStatement *s = ss->head; s; s = s->next) {
    return evaluateStmt(s, out);
  }
}
 
int main(void)
{
    FILE *outf;
    SStatements *e = NULL;
    char test[]="PRINT 4 + 2*10 + 3*( 5 + 1 )";
    int result = 0;
 
    e = getAST(test);
 
    outf = fopen("out.bin", "wb"); // TODO: from cmd line
    result = evaluateStmts(e, outf);
    fclose(outf);
 
    printf("Result of '%s' is %d\n", test, result);
 
    deleteStmts(e);
 
    return 0;
}
