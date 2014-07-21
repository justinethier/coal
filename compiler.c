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
 
typedef struct {
    hashtbl *syms;
    int numVars;
} SymTbl;
SymTbl *newSymTbl(){
  SymTbl *st = (SymTbl *)malloc(sizeof(SymTbl));
  st->syms = htinit();
  st->numVars = 0;
}

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

void evaluate(int pass, SExpression *e, SymTbl *symTbl, FILE *out) {
  unsigned char instr;

  switch (e->type) {
    case eVALUE:
      printf("AST LITERAL %d\n", e->value);
      instr = INST_LITERAL;
      fwrite(&instr, sizeof(instr), 1, out);
      instr = (unsigned char)e->value;
      fwrite(&instr, sizeof(instr), 1, out);
      break;

    case eIDENT:
      printf("AST IDENT %s\n", e->ident);

// TODO: combine with other common code for loading addr from symTbl
      struct nlist *htdata = htlookup(symTbl->syms, (char *)e->ident);
      if (htdata == NULL || htdata->defn == NULL) {
        printf("ERROR: no symbol table entry found for identifier %s\n", 
               e->ident);
      } else {
        int *addr = (int *)htdata->defn;
        instr = INST_LOAD;
        fwrite(&instr, sizeof(instr), 1, out);
        instr = (unsigned char)(*addr);
        fwrite(&instr, sizeof(instr), 1, out);
      }
      break;

    case eMULTIPLY:
      printf("AST MUL\n");
      instr = INST_MUL;
      evaluate(pass, e->left, symTbl, out);
      evaluate(pass, e->right, symTbl, out);
      fwrite(&instr, sizeof(instr), 1, out);
      break;

    case ePLUS:
      printf("AST ADD\n");
      instr = INST_ADD;
      evaluate(pass, e->left, symTbl, out);
      evaluate(pass, e->right, symTbl, out);
      fwrite(&instr, sizeof(instr), 1, out);
      break;

    default:
      // shouldn't be here
      return;
  }

  return;
}

void evaluateStmt(int pass, SStatement *s, SymTbl *symTbl, FILE *out) {
  unsigned char instr;
  switch (s->type) {
    case sPRINT:
      if (pass == 1) {
        printf("AST PRINT\n");
        instr = INST_IO;
        evaluate(pass, s->expr, symTbl, out);
        fwrite(&instr, sizeof(instr), 1, out);
      }
      break;

    case sLET:
      if (pass == 0) {
        int *num = (int *)malloc(sizeof(int));
        *num = symTbl->numVars++;
        htput(symTbl->syms, s->identifier, num);
      }
      // TODO: need to compute index into activation frame, and write it here
      // TODO: do we need to make a first pass over AST for variables?

      if (pass == 1) {
        printf("AST LET %s\n", s->identifier);
        struct nlist *htdata = htlookup(symTbl->syms, s->identifier);
        if (htdata == NULL || htdata->defn == NULL) {
          printf("ERROR: no symbol table entry found for identifier %s\n", 
                 s->identifier);
        } else {
          int *addr = (int *)htdata->defn;
          evaluate(pass, s->expr, symTbl, out);
          instr = INST_STORE;
          fwrite(&instr, sizeof(instr), 1, out);
          instr = (unsigned char)(*addr);
          fwrite(&instr, sizeof(instr), 1, out);
        }
      }
      break;
  }

  return;
}
 
void evaluateStmts(int pass, SStatements *ss, SymTbl *symTbl, FILE *out) {
  unsigned char instr;

  if (ss == NULL) {
    printf("evaluateStmts - ss is NULL\n");
    return;
  }

  // output vars
  // For now just reserve slots. not the most efficient scheme, but
  // should work
  if (pass == 1) {
      for (int i = 0; i < symTbl->numVars; i++) {
        instr = INST_LITERAL;
        fwrite(&instr, sizeof(instr), 1, out);
        instr = (unsigned char)0;
        fwrite(&instr, sizeof(instr), 1, out);
      }
  }

  for (SStatement *s = ss->head; s; s = s->next) {
    evaluateStmt(pass, s, symTbl, out);
  }
}

void process(const char *inputF, FILE *output){
  SymTbl *symTbl = newSymTbl();
  char *src = getFileContents(inputF, NULL);
  SStatements *ast = getAST(src);
  evaluateStmts(0, ast, symTbl, output);
  evaluateStmts(1, ast, symTbl, output);
  deleteStmts(ast);
  free(src);
  htfree(symTbl->syms);
}

//TODO: command line args to
// - specify code from cmd line (for convenience)
// - specify input (file or stdin -)
// - output filename (or just default to input.bin)
int main(int argc, char **argv) {
  int option = -1;
  char *inputF = NULL;
  FILE *output = NULL;

//char key[] = "key";
//char testStr[] = "my test string";
//struct nlist **ht = htinit();
//if (htlookup(ht, "test")) printf("found test in ht\n");
////htput(ht, key, NULL);
//htput(ht, key, testStr);
//if (htlookup(ht, key)) printf("found test in ht\n");
//struct nlist *htnode = htlookup(ht, key);
//if (htnode) printf("htnode = %s\n", htnode->defn);
//htfree(ht);

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
