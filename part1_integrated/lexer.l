%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "processor.h"
#include "parser.tab.h"
extern YYSTYPE yylval ;
extern YYSTYPE yyltype;
#define YYSTYPE int
%}
%%
[ \t\n]                                                     {}
[a-z0-9]+                                                   {yylval.str = yytext; return DATA;}
.                                                           {}
<<EOF>>                                                     {return EOI; exit(0);}
%%
int yywrap()
{
    return 1;
}
