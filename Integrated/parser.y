%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "processor.h"

int number;
int yyerror(char *);
int yylex();
FILE *output;
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
%start solv
%start cfg
%%
solv:
|solv exprn EOI                                 {make_processor();exit(0);}
;
exprn: DATA                                     {instruction* instr = make_instruction($1); add_instruction(instr);}
| exprn DATA                                    {instruction* instr = make_instruction($2); add_instruction(instr);}                
;
cfg:
| cfg data EOF                                  { return 1; }
;
data:
| 

%%
int main(int argc, char **argv)
{
    number = 0;
    if (argc < 3)
    {   
        printf("Error: Too few arguments.\n");
        exit(0);
    }
    if (argc == 3)
    {
        printf("Only inputs for part 1.\n");
    }
    if (argc == 4)
        output_result_name = strdup(argv[3]);
    
    no_inputs = argc - 1;
    output_name = strdup(argv[2]);
    output = fopen(argv[2], "w");
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
