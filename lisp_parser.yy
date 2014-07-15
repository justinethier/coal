%{
  #include <stdio.h>
  #include <stdlib.h>
  // stuff from flex that bison needs to know about:
  int yylex(void);
  void yyerror(char const *);
  int yyparse(void);
  FILE *yyin;
%}

/* Bison declarations */
%token NUM

%%
sexpr: atom                 {printf("matched sexpr\n");}
    | list
    ;
list: '(' members ')'       {printf("matched list\n");}
    | '('')'                {printf("matched empty list\n");}
    ;
members: sexpr              {printf("members 1\n");}
    | sexpr members         {printf("members 2\n");}
    ;
atom: ID                    {printf("ID\n");}
    | NUM                   {printf("NUM\n");}
    | STR                   {printf("STR\n");}
    ;
%%
// FROM http://aquamentus.com/flex_bison.html
// See also: http://en.wikipedia.org/wiki/GNU_bison
int main(int argc, char **argv) {
  FILE *fp = fopen("test.txt", "r");
  if (!fp) {
    return -1;
  }

  yyin = fp;
  do {
    yyparse();
  } while (!feof(yyin));

  return 0;
}

void yyerror(const char *s) {
  printf("Parse error: %s\n", s);
  exit(-1);
}
