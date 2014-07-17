
/*[ \t\n]+      { } / * eat up whitespace */
%%

";"[^\r\n]*             { /* skip comments */ }

"\""([^"]|[\\].)*"\""    {printf("%s", yytext);}
.                        {printf("%s", yytext);}

%%
int main(int argc, char **argv)
{
    while(yylex() != 0);
    return 0;
}
