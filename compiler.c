// Try using flex/bison, EG: http://research.engineering.wustl.edu/~beardj/FlexBisonC++.html


#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
#include "vm.h"
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

/**
 * Allocate a new filename with the given extension,
 * or NULL if an error occurs
 */
char *fnameWithExt(const char *fname, const char *ext){
  int len = strlen(fname);

  if (len > 4 && fname[len - 4] == '.'){
    char *buf = malloc(len+1); 
    strncpy(buf, fname, len+1);
    strncpy(buf + (len - 3), ext, 3);
    return buf;
  }

  return NULL;
}

char *getFileContents(FILE *fp) {
  char *source = NULL;
  if (fp != NULL) {
      /* Go to the end of the file. */
      if (fseek(fp, 0L, SEEK_END) == 0) {
          /* Get the size of the file. */
          long bufsize = ftell(fp);
          if (bufsize == -1) { /* Error */ }
  
          /* Allocate our buffer to that size. */
          source = malloc(sizeof(char) * (bufsize + 1));
  
          /* Go back to the start of the file. */
          if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }
  
          /* Read the entire file into memory. */
          size_t newLen = fread(source, sizeof(char), bufsize, fp);
          if (newLen == 0) {
              fputs("Error reading file", stderr);
          } else {
              source[++newLen] = '\0'; /* Just to be safe. */
          }
      }
      //fclose(fp);
  }
  
  //free(source); /* Don't forget to call free() later! */
  return(source);
}

void process(FILE *input, FILE *output){
  char *src = getFileContents(input);
  SStatements *ast = getAST(src);
  int result = evaluateStmts(ast, output);
  //printf("Result of '%s' is %d\n", test, result);
 
  deleteStmts(ast);
  free(src);
}

//TODO: command line args to
// - specify code from cmd line (for convenience)
// - specify input (file or stdin -)
// - output filename (or just default to input.bin)
int main(int argc, char **argv) {
  int option = -1;
  FILE *input = NULL, *output = NULL;

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

      input = fopen(argv[optind], "r");
      output = fopen(outFname, "wb");
    }
  }

  if (input && output){
    process(input, output);
    fclose(input);
    fclose(output);
  }

  return 0;
}
