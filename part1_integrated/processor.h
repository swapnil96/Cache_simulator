
// Arithmetic and Logical Instructions

// 1)  Addition (with overflow)                         add rd, rs, rt
// 2)  Addition immediate (with overflow)               addi rt, rs, imm
// 3)  AND                                              and rd, rs, rt
// 4)  Multiply                                         mult rs, rt
// 5)  Multiply add                                     madd rs, rt
// 6)  NOR                                              nor rd, rs, rt
// 7)  OR                                               or rd, rs, rt
// 8)  OR immediate                                     ori rt, rs, imm
// 9)  Shift left logical                               sll rd, rt, shamt
// 10) Shift left logical variable                      sllv rd, rt, rs
// 11) Subtract (with overflow)                         sub rd, rs, rt

// Constant-Manipulating Instructions

// 12) Load upper immediate                             lui rt, imm

// Comparison Instructions

// 13) Set less than unsigned                           sltu rd, rs, rt
// 14) Set less than immediate                          slti rt, rs, imm

// Branch Instructions

// 15) Branch on equal                                  beq rs, rt, label
// 16) Branch on greater than equal zero                bgez rs, label
// 17) Branch on greater than zero                      bgtz rs, label
// 18) Branch on less than equal zero                   blez rs, label
// 19) Branch on less than zero                         bltz rs, label

// Load/Store Instructions

// 20) Load byte                                        lb rt, address
// 21) Load word                                        lw rt, address
// 22) Store byte                                       sb rt, address
// 23) Store word                                       sw rt, address

// Jump Instructions

// 24) Jump                                             j target
// 25) Jump and link                                    jal target
// 26) Jump and link register                           jalr rs, rd
// 27) Jump register                                    jr rs

// Data Movement

// 28) Move from lo                                     mflo rd
// 29) Move                                             move rdest(=Rd), rsrc (=Rt)

// Extra Instructions

// 30) Set less than                                    slt rd, rs, rt
// 31) Addition Immediate (without overflow)            addiu rt, rs, imm
// 32) Branch on not equal                              bne rs, rt, label

#ifndef PROCESSOR_H
#define PROCESSOR_H

extern FILE* yyin;
extern int yylineno;

extern char* output_name;
extern char* output_result_name;
extern char* cfg;

typedef struct registers
{
    char* value;
    char* name;
    char* address;
}registers ;

typedef struct instruction
{
    int type;
    char* binary;
    char* Rd;
    char* Rs;
    char* Rt;
    char* shamt;
    char* offset;
    char* target;
}instruction;

typedef struct pipeline_register_1
{
    char* name;
    int pc;
    instruction* ins;
}pipeline_register_1;


typedef struct pipeline_register_2
{
    char* name;
    char* op1;
    char* op2;
    char* sign;
    char* Rs;
    char* Rt;
    char* Rd;
    char* shamt;
    char* target;
    int branch;
    int ALU_op;

    int REG_WRITE;
    int MEM_TO_REG;
    int MEM_WRITE;
    int MEM_READ;
    int ALU_src_1;
    int ALU_src_2;
    int zero_extend;
    int REG_DEST;
}pipeline_register_2;



typedef struct pipeline_register_3
{
    char* name;
    char* result;
    char* write_data;
    char* write_data_register;
    char* dest;
    double mult_data;
    int REG_WRITE;
    int MEM_TO_REG;
    int MEM_WRITE;
    int MEM_READ;
}pipeline_register_3;


typedef struct pipeline_register_4
{
    char* name;
    char* read_data;
    char* dest;
    char* ans;
    double mult_data;
    int REG_WRITE;
    int MEM_TO_REG;
}pipeline_register_4;


void add_instruction(instruction* data);
instruction* make_instruction(char* data);
void make_processor();
void start();
char* slice(char* data, int length);
char* bin_to_hex(char* str, int to);
char* hex_to_bin(char* str);
double bin_to_dec(char* bin);
char* dec_to_bin(double d);
char* sign_extend(char* data, int start);
char* alu(char* x, char* y,int ALU_op);
char* zero_extend_func(char* data, int start);
void control_function();
instruction* copy_instruction(instruction* ins);
void print_instruction(instruction* ins);
void print_p_1(pipeline_register_1* p1);
void print_p_2(pipeline_register_2* p2);
void print_p_3(pipeline_register_3* p3);
void print_p_4(pipeline_register_4* p4);
void write_svg(char* name);
void build_svg(int iff,int id,int ex,int mem,int wb, char* I1, char* I2, char* I3, char* I4, char* I5, int T1, int T2, int T3, int T4, int T5);
char* print_ins(instruction* ins);
void print_result_file(int cycles, int ins, int idle_cycles);

void read_cfg();

char* concat(char* s1, char* s2);


registers* find_register(char* add);
registers* faltu_register;


extern FILE* output;
extern FILE* svg;

extern int number;
extern int pc_branch;
extern instruction* instruction_array[20000];
extern registers* register_array[34];
extern char* memory[67108864];

extern instruction* noop;


extern pipeline_register_1* IF_ID_write;
extern pipeline_register_1* IF_ID_read;

extern pipeline_register_2* ID_EX_write;
extern pipeline_register_2* ID_EX_read;

extern pipeline_register_3* EX_MEM_write;
extern pipeline_register_3* EX_MEM_read;

extern pipeline_register_4* MEM_WB_write;
extern pipeline_register_4* MEM_WB_read;
#endif