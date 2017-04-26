%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "processor.h"

int number;
int yyerror(char *);
int yylex();
FILE *output;
char* cfg;
char* output_name;
char* output_result_name;
int no_inputs;
%}
%union
{
    char* str;
}
%token EOI
%token <str> DATA
%type <str> exprn
%%
solv:
|solv exprn EOI                                 {make_processor();exit(0);}
;
exprn: DATA                                     {instruction* instr = make_instruction($1); add_instruction(instr);}
| exprn DATA                                    {instruction* instr = make_instruction($2); add_instruction(instr);}                
;
%%
int main(int argc, char **argv)
{
    number = 0;
    if (argc < 5)
    {   
        printf("Error: Too few arguments.\n");
        exit(0);
    }
    cfg = strdup(argv[2]);
    read_cfg();
    output_result_name = strdup(argv[4]);

    no_inputs = argc - 1;
    output_name = strdup(argv[3]);
    output = fopen(argv[3], "w");
    yyin = fopen(argv[1], "r");
    yyparse();
    fclose(yyin);
    fclose(output);
    return 0;
}
int yyerror(char *s)
{
    printf("%s\n", "SynErr");
    return 1;
}
