// Try using flex/bison, EG: http://research.engineering.wustl.edu/~beardj/FlexBisonC++.html


#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
#include "hashtbl.h"
#include "vm.h"
#include "util.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
 
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

void evaluate(SExpression *e, FILE *out) {
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
      return;
  }

  return;
}

void evaluateStmt(SStatement *s, FILE *out) {
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

  return;
}
 
void evaluateStmts(SStatements *ss, FILE *out) {
  if (ss == NULL) {
    printf("evaluateStmts - ss is NULL\n");
    return;
  }

  for (SStatement *s = ss->head; s; s = s->next) {
    evaluateStmt(s, out);
  }
}

void process(const char *inputF, FILE *output){
  char *src = getFileContents(inputF, NULL);
  SStatements *ast = getAST(src);
  evaluateStmts(ast, output);
  deleteStmts(ast);
  free(src);
}

//TODO: command line args to
// - specify code from cmd line (for convenience)
// - specify input (file or stdin -)
// - output filename (or just default to input.bin)
int main(int argc, char **argv) {
  int option = -1;
  char *inputF = NULL;
  FILE *output = NULL;

char key[] = "key";
char testStr[] = "my test string";
struct nlist **ht = htinit();
if (htlookup(ht, "test")) printf("found test in ht\n");
//htput(ht, key, NULL);
htput(ht, key, testStr);
if (htlookup(ht, key)) printf("found test in ht\n");
struct nlist *htnode = htlookup(ht, key);
if (htnode) printf("htnode = %s\n", htnode->defn);



  while ((option = getopt(argc, argv, "")) != -1) {
    switch(option) {
      default:
        break;
    }
  }

  if (argv[optind] != NULL) {
    char *outFname = fnameWithExt(argv[optind], "bin");
    if (outFname) {
      printf("%s\n", outFname);

      inputF = argv[optind];
      output = fopen(outFname, "wb");
    }
    free(outFname);
  }

  if (inputF && output){
    process(inputF, output);
    fclose(output);
  }

  return 0;
}
