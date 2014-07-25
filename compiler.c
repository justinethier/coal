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
 
SStatements *getAST(const char *expr);
int getSymbolAddress(SymTbl *symTbl, char *sym);
size_t fwritep(int pass, const void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t codegen(int pass, SExpression *e, SymTbl *symTbl, FILE *out, size_t numBytes);
size_t codegenStmt(int pass, SStatement *s, SymTbl *symTbl, FILE *out, size_t numBytes);
size_t codegenStmts(int pass, SStatements *ss, SymTbl *symTbl, FILE *out, size_t numBytes);

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
 
    return ParserAstRoot; //stmts;
}

int getSymbolAddress(SymTbl *symTbl, char *sym) {
  struct nlist *htdata = htlookup(symTbl->syms, sym);
  if (htdata == NULL || htdata->defn == NULL) {
    printf("ERROR: no symbol table entry found for identifier %s\n", sym);
    return -1;
  }

  AstSym *symData = (AstSym *)htdata->defn;
  return symData->addr;
}

size_t fwritep(int pass, const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (pass == 0){
    return size * nmemb; // Expected num bytes
  }

  return fwrite(ptr, size, nmemb, stream);
}

size_t codegen(int pass, SExpression *e, SymTbl *symTbl, FILE *out, size_t numBytes) {
  unsigned char instr;

  switch (e->type) {
    case eVALUE:
      tracef("AST LITERAL %d\n", e->value);
      instr = INST_LITERAL;
      numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
      Object lit;
      lit.type = OBJ_INT;
      lit.value = e->value;
      numBytes += sizeof(Object) * fwritep(pass, &lit, sizeof(Object), 1, out);
      break;

    case eIDENT:
      tracef("AST IDENT %s\n", e->ident);
      instr = INST_LOAD;
      numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
      instr = (unsigned char)getSymbolAddress(symTbl, e->ident);
      numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
      break;

    case eMULTIPLY:
      trace("AST MUL\n");
      instr = INST_MUL;
      numBytes = codegen(pass, e->left, symTbl, out, numBytes);
      numBytes = codegen(pass, e->right, symTbl, out, numBytes);
      numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
      break;

    case ePLUS:
      trace("AST ADD\n");
      instr = INST_ADD;
      numBytes = codegen(pass, e->left, symTbl, out, numBytes);
      numBytes = codegen(pass, e->right, symTbl, out, numBytes);
      numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
      break;

    default:
      // shouldn't be here
      return numBytes;
  }

  return numBytes;
}

size_t codegenStmt(int pass, SStatement *s, SymTbl *symTbl, FILE *out, size_t numBytes) {
  unsigned char instr;

  switch (s->type) {
    case sPRINT:
      if (pass == 1) {
        trace("AST PRINT\n");
      }
      instr = INST_IO;
      numBytes = codegen(pass, s->expr, symTbl, out, numBytes);
      numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
      break;

    case sLET:
      if (pass == 0) {
        AstSym *sym = (AstSym *)malloc(sizeof(AstSym));
        sym->type = sLET;
        sym->addr = symTbl->numVars++;
        htput(symTbl->syms, s->identifier, sym);
      }
      // TODO: need to compute index into activation frame, and write it here
      // TODO: do we need to make a first pass over AST for variables?

      if (pass == 1) {
        tracef("AST LET %s\n", s->identifier);
      }
      numBytes = codegen(pass, s->expr, symTbl, out, numBytes);
      instr = INST_STORE;
      numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
      instr = (unsigned char)getSymbolAddress(symTbl, s->identifier);
      numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
      break;

    case sSUB:
      if (pass == 0) {
        AstSym *sym = (AstSym *)malloc(sizeof(AstSym));
        sym->type = sSUB;
        sym->addr = numBytes;
        htput(symTbl->syms, s->func->name, sym);
      }

      // TODO: allocate new symbol table?
      //       then how do we do two passes? need to store it in function AST?
      numBytes = codegenStmts(pass, (SStatements *)s->func->body, symTbl, out, numBytes);
      break;
  }

  return numBytes;
}
 
size_t codegenStmts(int pass, SStatements *ss, SymTbl *symTbl, FILE *out, size_t numBytes) {
  unsigned char instr;

  if (ss == NULL) {
    trace("codegenStmts - ss is NULL\n");
    return numBytes;
  }

  // output vars
  // For now just reserve slots. not the most efficient scheme, but
  // should work
  for (int i = 0; i < symTbl->numVars; i++) {
    instr = INST_LITERAL;
    numBytes += sizeof(instr) * fwritep(pass, &instr, sizeof(instr), 1, out);
    Object lit;
    lit.type = OBJ_INT;
    lit.value = 0;
    numBytes += sizeof(Object) * fwritep(pass, &lit, sizeof(Object), 1, out);
  }

  for (SStatement *s = ss->head; s; s = s->next) {
    numBytes = codegenStmt(pass, s, symTbl, out, numBytes);
  }

  return numBytes;
}

void process(const char *inputF, FILE *output){
  SymTbl *symTbl = newSymTbl();
  char *src = getFileContents(inputF, NULL);
  size_t numBytes = 0;
  SStatements *ast = getAST(src);
  numBytes = codegenStmts(0, ast, symTbl, output, 0);
  numBytes = codegenStmts(1, ast, symTbl, output, 0);
  tracef("Wrote %d bytes\n", numBytes);
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

  while ((option = getopt(argc, argv, "")) != -1) {
    switch(option) {
      default:
        break;
    }
  }

  if (argv[optind] != NULL) {
    char *outFname = fnameWithExt(argv[optind], "bin");
    if (outFname) {
      tracef("%s\n", outFname);

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
