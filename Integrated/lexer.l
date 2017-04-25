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
[ \t\n]                                                    {}
[a-z0-9]+                                                   {if(strlen(yytext) == 8)
                                                                {yylval.str = yytext; return DATA;}
                                                             else
                                                                printf("Error\n");
                                                            }
<<EOF>>                                                     {return EOI; exit(0);}
%%
int yywrap()
{
    return 1;
}
