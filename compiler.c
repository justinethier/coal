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

size_t codegen(int pass, SExpression *e, SymTbl *symTbl, FILE *out) {
  unsigned char instr;
  size_t numBytes = 0;

  switch (e->type) {
    case eVALUE:
      tracef("AST LITERAL %d\n", e->value);
      instr = INST_LITERAL;
      numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
      Object lit;
      lit.type = OBJ_INT;
      lit.value = e->value;
      numBytes += sizeof(Object) * fwrite(&lit, sizeof(Object), 1, out);
      break;

    case eIDENT:
      tracef("AST IDENT %s\n", e->ident);
      instr = INST_LOAD;
      numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
      instr = (unsigned char)getSymbolAddress(symTbl, e->ident);
      numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
      break;

    case eMULTIPLY:
      trace("AST MUL\n");
      instr = INST_MUL;
      numBytes += codegen(pass, e->left, symTbl, out);
      numBytes += codegen(pass, e->right, symTbl, out);
      numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
      break;

    case ePLUS:
      trace("AST ADD\n");
      instr = INST_ADD;
      numBytes += codegen(pass, e->left, symTbl, out);
      numBytes += codegen(pass, e->right, symTbl, out);
      numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
      break;

    default:
      // shouldn't be here
      return numBytes;
  }

  return numBytes;
}

size_t codegenStmt(int pass, SStatement *s, SymTbl *symTbl, FILE *out) {
  unsigned char instr;
  size_t numBytes = 0;

  switch (s->type) {
    case sPRINT:
      if (pass == 1) {
        trace("AST PRINT\n");
        instr = INST_IO;
        numBytes += codegen(pass, s->expr, symTbl, out);
        numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
      }
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
        numBytes += codegen(pass, s->expr, symTbl, out);
        instr = INST_STORE;
        numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
        instr = (unsigned char)getSymbolAddress(symTbl, s->identifier);
        numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
      }
      break;

    case sSUB:
// TODO: how to call into a function in the VM?  may need to switch code gen around to 
//       return number of bytes written out, and then compute function offsets. will this
//       require yet another pass to achieve?
      if (pass == 0) {
        AstSym *sym = (AstSym *)malloc(sizeof(AstSym));
        sym->type = sSUB;
        sym->addr = 0; // TODO
        htput(symTbl->syms, s->identifier, sym);
      }

      break;
  }

  return numBytes;
}
 
size_t codegenStmts(int pass, SStatements *ss, SymTbl *symTbl, FILE *out) {
  unsigned char instr;
  size_t numBytes = 0;

  if (ss == NULL) {
    trace("codegenStmts - ss is NULL\n");
    return numBytes;
  }

  // output vars
  // For now just reserve slots. not the most efficient scheme, but
  // should work
  if (pass == 1) {
      for (int i = 0; i < symTbl->numVars; i++) {
        instr = INST_LITERAL;
        numBytes += sizeof(instr) * fwrite(&instr, sizeof(instr), 1, out);
        Object lit;
        lit.type = OBJ_INT;
        lit.value = 0;
        numBytes += sizeof(Object) * fwrite(&lit, sizeof(Object), 1, out);
      }
  }

  for (SStatement *s = ss->head; s; s = s->next) {
    numBytes += codegenStmt(pass, s, symTbl, out);
  }

  return numBytes;
TODO: may not be good enough to record bytes when writing, because cannot record
location of functions like that. may need to make a pass that just returns the num
of bytes expected to write without writing them (possible?) then record that information.
could then make a final pass to write the acutal code, including exact function locations.
}

void process(const char *inputF, FILE *output){
  SymTbl *symTbl = newSymTbl();
  char *src = getFileContents(inputF, NULL);
  SStatements *ast = getAST(src);
  codegenStmts(0, ast, symTbl, output);
  size_t bytes = codegenStmts(1, ast, symTbl, output);
  tracef("Wrote %d bytes\n", bytes);
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
