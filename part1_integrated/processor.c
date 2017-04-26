#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "processor.h"
#include "cache.h"
// Global variables
int number;
instruction* instruction_array[20000];
registers* register_array[34];
pipeline_register_1* IF_ID_write;
pipeline_register_1* IF_ID_read;
pipeline_register_2* ID_EX_write;
pipeline_register_2* ID_EX_read;
pipeline_register_3* EX_MEM_write;
pipeline_register_3* EX_MEM_read;
pipeline_register_4* MEM_WB_write;

instruction* noop;

FILE* output;
FILE* alpha;
char* output_name;
char* output_result_name;

// File variables
char* bin_codes[] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011",
                     "1100", "1101", "1110", "1111"};

char* hex_codes[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"};


// Functions
void write_svg(char* name)
{
    FILE* to = fopen(output_name,"ab");
    FILE* from = fopen(name,"rb");

    int c;
    while((c=getc(from))!=EOF)
        fputc(c, to);

    fclose(to);
    fclose(from);
}

void build_svg(int iff,int id,int ex,int mem,int wb, char* I1, char* I2, char* I3, char* I4, char* I5, int T1, int T2, int T3, int T4, int T5)
{
    remove(output_name);
    write_svg("SVG/start");
    if(iff){
        write_svg("SVG/IF_a");
    }
    else{
        write_svg("SVG/IF");
    }
    if(id){
        write_svg("SVG/ID_a");
    }
    else{
        write_svg("SVG/ID");
    }
    if(ex){
        write_svg("SVG/EX_a");
    }
    else{
        write_svg("SVG/EX");
    }
    if(mem){
        write_svg("SVG/MEM_a");
    }
    else{
        write_svg("SVG/MEM");
    }
    if(wb){
        write_svg("SVG/WB_a");
    }
    else{
        write_svg("SVG/WB");
    }
    write_svg("SVG/base");
    write_svg("SVG/text");
    
    write_svg("SVG/I_1");
    FILE* alpha = fopen(output_name,"a");    
    fprintf(alpha, "%s", I1);
    fclose(alpha);

    write_svg("SVG/I_2");
    alpha = fopen(output_name,"a");    
    fprintf(alpha, "%s", I2);
    fclose(alpha);
    
    write_svg("SVG/I_3");
    alpha = fopen(output_name,"a");    
    fprintf(alpha, "%s", I3);
    fclose(alpha);
    
    write_svg("SVG/I_4");
    alpha = fopen(output_name,"a");    
    fprintf(alpha, "%s", I4);
    fclose(alpha);
    
    write_svg("SVG/I_5");
    alpha = fopen(output_name,"a");    
    fprintf(alpha, "%s", I5);
    fprintf(alpha, "</tspan></text></g>\n");
    fclose(alpha);

    write_svg("SVG/T_1");
    alpha = fopen(output_name,"a");
    if(T1 == 0)
        fprintf(alpha, "Active");
    else
        fprintf(alpha, "Inactive");
    fclose(alpha);

    write_svg("SVG/T_2");
    alpha = fopen(output_name,"a");
    if(T2 == 0)
        fprintf(alpha, "Active");
    else
        fprintf(alpha, "Inactive");
    fclose(alpha);

    write_svg("SVG/T_3");
    alpha = fopen(output_name,"a");
    if(T3 == 0)
        fprintf(alpha, "Active");
    else
        fprintf(alpha, "Inactive");
    fclose(alpha);

    write_svg("SVG/T_4");
    alpha = fopen(output_name,"a");
    if(T4 == 0)
        fprintf(alpha, "Active");
    else
        fprintf(alpha, "Inactive");
    fclose(alpha);

    write_svg("SVG/T_5");
    alpha = fopen(output_name,"a");
    if(T5 == 0)
        fprintf(alpha, "Active");
    else
        fprintf(alpha, "Inactive");
    
    fprintf(alpha, "</tspan></text></g></g></svg>\n");

    fclose(alpha);

}

void print_result_file(int cycles, int ins,int idle_cycles)
{
    remove(output_result_name);
    FILE* write = fopen(output_name,"w");
    fprintf(write, "Instructions, %d\n", ins);
    fprintf(write, "IPC, %d\n", cycles);
    fprintf(write, "Time (ns), %lf\n", ins/2.0);
    fprintf(write, "Idle time (ns), %d\n", idle_cycles);
    fprintf(write, "Idle time (%%), %d\n", ins);
    fprintf(write, "Cache Summary\n%s\n", "Cache L1-I");
    fprintf(write, "num cache accesses, %d\n Cache L1-D\n", ins);
    fprintf(write, "num cache accesses, %d\n", ins);
    fclose(write);    
}

void print_instruction(instruction* ins)
{
    printf("Type - %d\n", ins->type);
    //printf("Binary - %s\n", ins->binary);
    printf("Rd - %s\n", ins->Rd);
    printf("Rs - %s\n", ins->Rs);
    printf("Rt - %s\n", ins->Rt);
    printf("Shamt - %s\n", ins->shamt);
    printf("Offset - %s\n", ins->offset);
    printf("Target - %s\n", ins->target);
}

void print_p_1(pipeline_register_1* p1)
{
    printf("PC - %d\n", p1->pc);
    printf("INSTRUCTION - \n");
    print_instruction(p1->ins);
}

void print_p_2(pipeline_register_2* p2)
{
    printf("OP1 - %s\n", p2->op1);
    printf("OP2 - %s\n", p2->op2);
    printf("Sign - %s\n", p2->sign);
    printf("Rs - %s\n", p2->Rs);
    printf("Rt - %s\n", p2->Rt);
    printf("Rd - %s\n", p2->Rd);
    printf("Shamt - %s\n", p2->shamt);
    printf("Target - %s\n", p2->target);
    printf("REG_WRITE - %d\n", p2->REG_WRITE);
    printf("MEM_TO_REG - %d\n", p2->MEM_TO_REG);
    printf("MEM_WRITE - %d\n", p2->MEM_WRITE);
    printf("MEM_READ - %d\n", p2->MEM_READ);
    printf("ALU_src_1 - %d\n", p2->ALU_src_1);
    printf("ALU_src_2 - %d\n", p2->ALU_src_2);
    printf("zero_extend - %d\n", p2->zero_extend);
    printf("branch - %d\n", p2->branch);
    printf("REG_DEST - %d\n", p2->REG_DEST);
    printf("ALU_op - %d\n", p2->ALU_op);
}

void print_p_3(pipeline_register_3* p3)
{
    printf("Result - %s\n", p3->result);
    printf("Write_data - %s\n", p3->write_data);
    printf("Write_data_Register - %s\n", p3->write_data_register);
    printf("Dest - %s\n", p3->dest);
    printf("REG_WRITE - %d\n", p3->REG_WRITE);
    printf("MEM_TO_REG - %d\n", p3->MEM_TO_REG);
    printf("MEM_WRITE - %d\n", p3->MEM_WRITE);
    printf("MEM_READ - %d\n", p3->MEM_READ);
}

void print_p_4(pipeline_register_4* p4)
{
    printf("Read_data - %s\n", p4->read_data);
    printf("Dest - %s\n", p4->dest);
    printf("Ans - %s\n", p4->ans);
    printf("REG_WRITE - %d\n", p4->REG_WRITE);
    printf("MEM_TO_REG - %d\n", p4->MEM_TO_REG);
}

char* concat(char* s1, char* s2)
{
    char* s3 = (char *) calloc(1 + strlen(s1)+ strlen(s2), sizeof(char));
    strcpy(s3, s1);
    strncat(s3, s2, strlen(s2));
    return s3;
}

char* hex_to_bin(char* str)
{
    char* ans = "";
    for(int i = 0; i < 8; i++)
    {
        if('0' == str[i])
            ans = concat(ans, bin_codes[0]);

        if('1' == str[i])
            ans = concat(ans, bin_codes[1]);

        if('2' == str[i])
            ans = concat(ans, bin_codes[2]);

        if('3' == str[i])
            ans = concat(ans, bin_codes[3]);

        if('4' == str[i])
            ans = concat(ans, bin_codes[4]);

        if('5' == str[i])
            ans = concat(ans, bin_codes[5]);

        if('6' == str[i])
            ans = concat(ans, bin_codes[6]);

        if('7' == str[i])
            ans = concat(ans, bin_codes[7]);

        if('8' == str[i])
            ans = concat(ans, bin_codes[8]);

        if('9' == str[i])
            ans = concat(ans, bin_codes[9]);

        if('a' == str[i])
            ans = concat(ans, bin_codes[10]);

        if('b' == str[i])
            ans = concat(ans, bin_codes[11]);

        if('c' == str[i])
            ans = concat(ans, bin_codes[12]);

        if('d' == str[i])
            ans = concat(ans, bin_codes[13]);

        if('e' == str[i])
            ans = concat(ans, bin_codes[14]);

        if('f' == str[i])
            ans = concat(ans, bin_codes[15]);
    }
    return ans;
}

char* bin_to_hex(char* str, int to)
{
    char* ans = "";
    for(int i = 0; i < to; i += 4)
    {
        if(strcmp(slice(str + i, 4), bin_codes[0]) == 0)
            ans = concat(ans, hex_codes[0]);

        else if(strcmp(slice(str + i, 4), bin_codes[1]) == 0)
            ans = concat(ans, hex_codes[1]);

        else if(strcmp(slice(str + i, 4), bin_codes[2]) == 0)
            ans = concat(ans, hex_codes[2]);

        else if(strcmp(slice(str + i, 4), bin_codes[3]) == 0)
            ans = concat(ans, hex_codes[3]);

        else if(strcmp(slice(str + i, 4), bin_codes[4]) == 0)
            ans = concat(ans, hex_codes[4]);

        else if(strcmp(slice(str + i, 4), bin_codes[5]) == 0)
            ans = concat(ans, hex_codes[5]);

        else if(strcmp(slice(str + i, 4), bin_codes[6]) == 0)
            ans = concat(ans, hex_codes[6]);

        else if(strcmp(slice(str + i, 4), bin_codes[7]) == 0)
            ans = concat(ans, hex_codes[7]);

        else if(strcmp(slice(str + i, 4), bin_codes[8]) == 0)
            ans = concat(ans, hex_codes[8]);

        else if(strcmp(slice(str + i, 4), bin_codes[9]) == 0)
            ans = concat(ans, hex_codes[9]);

        else if(strcmp(slice(str + i, 4), bin_codes[10]) == 0)
            ans = concat(ans, hex_codes[10]);

        else if(strcmp(slice(str + i, 4), bin_codes[11]) == 0)
            ans = concat(ans, hex_codes[11]);

        else if(strcmp(slice(str + i, 4), bin_codes[12]) == 0)
            ans = concat(ans, hex_codes[12]);

        else if(strcmp(slice(str + i, 4), bin_codes[13]) == 0)
            ans = concat(ans, hex_codes[13]);

        else if(strcmp(slice(str + i, 4), bin_codes[14]) == 0)
            ans = concat(ans, hex_codes[14]);

        else if(strcmp(slice(str + i, 4), bin_codes[15]) == 0)
            ans = concat(ans, hex_codes[15]);
    }
    return ans;
}

double bin_to_dec(char* bin)
{
    double ans = 0;
    for(int i = 31; i >= 0; i--)
        ans += (bin[i] - '0') * pow(2,31-i);

    ans = bin[0] == '0' ? ans : -1*((double)pow(2, 32) - ans);
    return ans;
}

char* dec_to_bin(double d)
{
    long int dec = (long int) d;
    if (dec < 0)
        dec = (long int)pow(2, 32) + dec;

    int* ans = calloc(32, sizeof(int));
    int idx = 31;
    while(dec > 0)
    {
        ans[idx] = dec % 2;;
        dec = dec / 2;
        idx--;
    }
    char* a = calloc(32, sizeof(char));
    for(int i = 0; i < 32; i++)
    {
        char y = ans[i] + '0';
        strncat(a, &y, 1);
    }
    return a;    
}

char* slice(char* data, int length)
{
    char* ans = calloc(length+1, sizeof(char));
    memmove(ans, data, length);
    return ans;
}

char* zero_extend_func(char* data, int start)
{
    char* ans = calloc(32, sizeof(char));    
    char y = '0';
    for(int i = 0; i < 32 - start; i++)
        strncat(ans+i, &y, 1);

    strncat(ans+32-start, data, start);
    return ans;
}

char* sign_extend(char* data, int start)
{
    int* a = calloc(32, sizeof(int));
    for(int i = 0; i < start; i++)
        a[32 - start + i] = data[i] - '0';

    char* ans = calloc(32, sizeof(char));    
    int t = data[0] == '1' ? 1 : 0;
    while(start < 32)
    {
        a[31 - start] = t;
        start++;
    }
    for(int i = 0; i < 32; i++)
    {
        char y = a[i] + '0';
        strncat(ans, &y, 1);
    }        
    return ans;
}

registers* faltu_register;

registers* find_register(char* add)
{
    if (strcmp(add, "12345") == 0)
        return faltu_register;

    if (strcmp(add, "00000") == 0)
        return register_array[0];

    if (strcmp(add, "00001") == 0)
        return register_array[1];

    if (strcmp(add, "00010") == 0)
        return register_array[2];

    if (strcmp(add, "00011") == 0)
        return register_array[3];

    if (strcmp(add, "00100") == 0)
        return register_array[4];

    if (strcmp(add, "00101") == 0)
        return register_array[5];

    if (strcmp(add, "00110") == 0)
        return register_array[6];

    if (strcmp(add, "00111") == 0)
        return register_array[7];

    if (strcmp(add, "01000") == 0)
        return register_array[8];

    if (strcmp(add, "01001") == 0)
        return register_array[9];

    if (strcmp(add, "01010") == 0)
        return register_array[10];

    if (strcmp(add, "01011") == 0)
        return register_array[11];

    if (strcmp(add, "01100") == 0)
        return register_array[12];

    if (strcmp(add, "01101") == 0)
        return register_array[13];

    if (strcmp(add, "01110") == 0)
        return register_array[14];

    if (strcmp(add, "01111") == 0)
        return register_array[15];

    if (strcmp(add, "10000") == 0)
        return register_array[16];

    if (strcmp(add, "10001") == 0)
        return register_array[17];

    if (strcmp(add, "10010") == 0)
        return register_array[18];

    if (strcmp(add, "10011") == 0)
        return register_array[19];

    if (strcmp(add, "10100") == 0)
        return register_array[20];

    if (strcmp(add, "10101") == 0)
        return register_array[21];

    if (strcmp(add, "10110") == 0)
        return register_array[22];

    if (strcmp(add, "10111") == 0)
        return register_array[23];

    if (strcmp(add, "11000") == 0)
        return register_array[24];

    if (strcmp(add, "11001") == 0)
        return register_array[25];

    if (strcmp(add, "11010") == 0)
        return register_array[26];

    if (strcmp(add, "11011") == 0)
        return register_array[27];

    if (strcmp(add, "11100") == 0)
        return register_array[28];

    if (strcmp(add, "11101") == 0)
        return register_array[29];

    if (strcmp(add, "11110") == 0)
        return register_array[30];
    
    if (strcmp(add, "11111") == 0)
        return register_array[31];
    
    if (strcmp(add, "-1") == 0)
        return register_array[32];       

    if (strcmp(add, "-2") == 0)
        return register_array[33];       

    if (strcmp(add, "-3") == 0)
        return register_array[34];        

}

instruction* make_instruction(char* data)
{
    instruction* ins = (instruction*) calloc(1, sizeof(instruction));
    char* bin;
    bin = hex_to_bin(data);
    ins->binary = strdup(bin);
    char* temp;
    char* temp1;
    char* temp2;
    temp2 = strdup(slice(bin+11, 5));
    char* val = calloc(5, sizeof(char));
    temp = slice(bin + 26, 6);
    temp1 = slice(bin, 6);
    int flag = 0;
    if((strcmp("100000", temp) == 0) && (strcmp("000000", temp1) == 0))              //Addition (with overflow)          add rd, rs, rt
        flag = 1, ins->type = 1;

    if((strcmp("100100", temp) == 0) && (strcmp("000000", temp1) == 0))               //AND                               and rd, rs, rt
        flag = 1, ins->type = 3;

    if((strcmp("011000", temp) == 0) && (strcmp("000000", temp1) == 0))               //Multiply                          mult rs, rt
        flag = 1, ins->type = 4;

    if((strcmp("011100", temp1) == 0) &&  (strcmp("000000", temp) == 0))               //madd
        flag = 1, ins->type = 5;

    if((strcmp("100111", temp) == 0) && (strcmp("000000", temp1) == 0))               //NOR                               nor rd, rs, rt
        flag = 1, ins->type = 6;

    if((strcmp("100101", temp) == 0) && (strcmp("000000", temp1) == 0))               //OR                                or rd, rs, rt
        flag = 1, ins->type = 7;

    if((strcmp("000000", temp) == 0) && (strcmp("000000", temp1) == 0))               //Shift left logical                sll rd, rt, shamt
        flag = 1, ins->type = 9;

    if((strcmp("000100", temp) == 0) && (strcmp("000000", temp1) == 0))               //Shift left logical variable       sllv rd, rt, rs
        flag = 1, ins->type = 10;

    if((strcmp("100010", temp) == 0) && (strcmp("000000", temp1) == 0))               //Subtract (with overflow)          sub rd, rs, rt
        flag = 1, ins->type = 11;

    if((strcmp("101011", temp) == 0) && (strcmp("000000", temp1) == 0))               //Set less than unsigned            sltu rd, rs, rt
        flag = 1, ins->type = 13;
        
    if(strcmp("001000", temp1) == 0)               //Addition immediate (with overflow)    addi rt, rs, imm
        flag = 1, ins->type = 2;

    if(strcmp("001101", temp1) == 0)               //OR immediate                          ori rt, rs, imm
        flag = 1, ins->type = 8;

    if(strcmp("001111", temp1) == 0)               //Load upper immediate                  lui rt, imm
        flag = 1, ins->type = 12;

    if(strcmp("001010", temp1) == 0)               //Set less than immediate               slti rt, rs, imm
        flag = 1, ins->type = 14;

    if(strcmp("000100", temp1) == 0)               //Branch on equal                       beq rs, rt, label
        flag = 1, ins->type = 15;

    if(strcmp("000001", temp1) == 0 && strcmp("00001",temp2) == 0)   //Branch on greater than equal zero     bgez rs, label
        flag = 1, ins->type = 16;

    if(strcmp("000111", temp1) == 0)               //Branch on greater than zero           bgtz rs, label
        flag = 1, ins->type = 17;

    if(strcmp("000110", temp1) == 0)               //Branch on less than equal zero        blez rs, label
        flag = 1, ins->type = 18;

    if(strcmp("000001", temp1) == 0 && strcmp("00000",temp2) == 0)   //Branch on less than zero              bltz rs, label
        flag = 1, ins->type = 19;
    
    if(strcmp("100000", temp1) == 0)               //Load byte                             lb rt, address
        flag = 1, ins->type = 20;
    
    if(strcmp("100011", temp1) == 0)               //Load word                             lw rt, offset
        flag = 1, ins->type = 21;
    
    if(strcmp("101000", temp1) == 0)               //Store byte                            sb rt, offset
        flag = 1, ins->type = 22;
    
    if(strcmp("101011", temp1) == 0)               //Store word                            sw rt, offset
        flag = 1, ins->type = 23;

    if(strcmp("000010", temp1) == 0)               //Jump                                  j target
        flag = 1, ins->type = 24;

    if(strcmp("000011", temp1) == 0)               //Jump and link                         jal target
        flag = 1, ins->type = 25;

    if(strcmp("000000", temp1) == 0 && strcmp("001001", temp) == 0)   //Jump and link register                jalr rs, rd
        flag = 1, ins->type = 26;

    if(strcmp("000000", temp1) == 0 && strcmp("001000", temp) == 0)   //Jump register                         jr rs
        flag = 1, ins->type = 27;

    if(strcmp("000000", temp1) == 0 && strcmp("010010", temp) == 0)   //Move from lo                          mflo rd
        flag = 1, ins->type = 28;

    if(strcmp("000000", temp1) == 0 && strcmp("100001", temp) == 0)   //Move                                  Move rdest(=Rd), rsrc(=Rt)
        flag = 1, ins->type = 29;

    if(strcmp("000000", temp1) == 0 && strcmp("101010",temp) == 0)    //Set less than                         slt rd, rs, rt
        flag = 1, ins->type = 30;

    if(strcmp("001001", temp1) == 0)               //Addition Immediate (without overflow)                    addiu rt, rs, imm
        flag = 1, ins->type = 31;

    if(strcmp("000101", temp1) == 0)               //Branch on not equal                                      bne rs, rt, label
        flag = 1, ins->type = 32;

    if (flag == 1)
    {
        ins->Rd = strdup(slice(bin+16, 5));
        ins->Rs = strdup(slice(bin+6, 5));
        ins->Rt = strdup(slice(bin+11, 5));
        ins->offset = strdup(slice(bin+16, 16));
        ins->shamt = strdup(slice(bin+21, 5));
        ins->target = strdup(slice(bin+6, 26));
        return ins;
    }
    else
    {
        printf("Not a valid instruction - %s\n", data);
        ins->type = -10;
    }
}

void add_instruction(instruction* data)
{
    if(data->type != -10)
    {
        instruction_array[number] = data;
        number++;
    }
}

void make_registers()
{   
    registers* zero = (registers*) malloc(sizeof(registers));   
    zero->name = "$00";
    zero->address = "00000";
    zero->value = "00000000000000000000000000000000";
    register_array[0] = zero;

    registers* one = (registers*) malloc(sizeof(registers));   
    one->name = "$01";
    one->address = "00001";
    one->value = "00000000000000000000000000000000";
    register_array[1] = one;
    
    registers* two = (registers*) malloc(sizeof(registers));   
    two->name = "$02";
    two->address = "00010";
    two->value = "00000000000000000000000000000000";
    register_array[2] = two;
    
    registers* three = (registers*) malloc(sizeof(registers));   
    three->name = "$03";
    three->address = "00011";
    three->value = "00000000000000000000000000000000";
    register_array[3] = three;
    
    
    registers* four = (registers*) malloc(sizeof(registers));   
    four->name = "$04";
    four->address = "00100";
    four->value = "00000000000000000000000000000000";
    register_array[4] = four;
    
    
    registers* five = (registers*) malloc(sizeof(registers));   
    five->name = "$05";
    five->address = "00101";
    five->value = "00000000000000000000000000000000";
    register_array[5] = five;
    
    
    registers* six = (registers*) malloc(sizeof(registers));   
    six->name = "$06";
    six->address = "00110";
    six->value = "00000000000000000000000000000000";
    register_array[6] = six;
    
    
    registers* seven = (registers*) malloc(sizeof(registers));   
    seven->name = "$07";
    seven->address = "00111";
    seven->value = "00000000000000000000000000000000";
    register_array[7] = seven;
    
    
    registers* eight = (registers*) malloc(sizeof(registers));   
    eight->name = "$08";
    eight->address = "01000";
    eight->value = "00000000000000000000000000000000";
    register_array[8] = eight;
    
    
    registers* nine = (registers*) malloc(sizeof(registers));   
    nine->name = "$09";
    nine->address = "01001";
    nine->value = "00000000000000000000000000000000";
    register_array[9] = nine;

    
    registers* ten = (registers*) malloc(sizeof(registers));   
    ten->name = "$10";
    ten->address = "01010";
    ten->value = "00000000000000000000000000000000";
    register_array[10] = ten;
    

    registers* eleven = (registers*) malloc(sizeof(registers));   
    eleven->name = "$11";
    eleven->address = "01011";
    eleven->value = "00000000000000000000000000000000";
    register_array[11] = eleven;


    registers* twelve = (registers*) malloc(sizeof(registers));   
    twelve->name = "$12";
    twelve->address = "01100";
    twelve->value = "00000000000000000000000000000000";
    register_array[12] = twelve;
    
    
    registers* thirteen = (registers*) malloc(sizeof(registers));   
    thirteen->name = "$13";
    thirteen->address = "01101";
    thirteen->value = "00000000000000000000000000000000";
    register_array[13] = thirteen;
    
    
    registers* fourteen = (registers*) malloc(sizeof(registers));   
    fourteen->name = "$14";
    fourteen->address = "01110";
    fourteen->value = "00000000000000000000000000000000";
    register_array[14] = fourteen;
    
    
    registers* fifteen = (registers*) malloc(sizeof(registers));   
    fifteen->name = "$15";
    fifteen->address = "01111";
    fifteen->value = "00000000000000000000000000000000";
    register_array[15] = fifteen;
    
    
    registers* sixteen = (registers*) malloc(sizeof(registers));   
    sixteen->name = "$16";
    sixteen->address = "10000";
    sixteen->value = "00000000000000000000000000000000";
    register_array[16] = sixteen;
    
    
    registers* seventeen = (registers*) malloc(sizeof(registers));   
    seventeen->name = "$17";
    seventeen->address = "10001";
    seventeen->value = "00000000000000000000000000000000";
    register_array[17] = seventeen;
    
    
    registers* eighteen = (registers*) malloc(sizeof(registers));   
    eighteen->name = "$18";
    eighteen->address = "10010";
    eighteen->value = "00000000000000000000000000000000";
    register_array[18] = eighteen;
    
    
    registers* nineteen = (registers*) malloc(sizeof(registers));   
    nineteen->name = "$19";
    nineteen->address = "10011";
    nineteen->value = "00000000000000000000000000000000";
    register_array[19] = nineteen;
    
    
    registers* twenty = (registers*) malloc(sizeof(registers));   
    twenty->name = "$20";
    twenty->address = "10100";
    twenty->value = "00000000000000000000000000000000";
    register_array[20] = twenty;
    
    
    registers* twenty_one = (registers*) malloc(sizeof(registers));   
    twenty_one->name = "$21";
    twenty_one->address = "10101";
    twenty_one->value = "00000000000000000000000000000000";
    register_array[21] = twenty_one;
    
    
    registers* twenty_two = (registers*) malloc(sizeof(registers));   
    twenty_two->name = "$22";
    twenty_two->address = "10110";
    twenty_two->value = "00000000000000000000000000000000";
    register_array[22] = twenty_two;
    
    
    registers* twenty_three = (registers*) malloc(sizeof(registers));   
    twenty_three->name = "$23";
    twenty_three->address = "10111";
    twenty_three->value = "00000000000000000000000000000000";
    register_array[23] = twenty_three;
    
    
    registers* twenty_four = (registers*) malloc(sizeof(registers));   
    twenty_four->name = "$24";
    twenty_four->address = "11000";
    twenty_four->value = "00000000000000000000000000000000";
    register_array[24] = twenty_four;
    
    
    registers* twenty_five = (registers*) malloc(sizeof(registers));   
    twenty_five->name = "$25";
    twenty_five->address = "11001";
    twenty_five->value = "00000000000000000000000000000000";
    register_array[25] = twenty_five;
    

    registers* twenty_six = (registers*) malloc(sizeof(registers));   
    twenty_six->name = "$26";
    twenty_six->address = "11010";
    twenty_six->value = "00000000000000000000000000000000";
    register_array[26] = twenty_six;
    
    
    registers* twenty_seven = (registers*) malloc(sizeof(registers));   
    twenty_seven->name = "$27";
    twenty_seven->address = "11011";
    twenty_seven->value = "00000000000000000000000000000000";
    register_array[27] = twenty_seven;
    
    
    registers* twenty_eight = (registers*) malloc(sizeof(registers));   
    twenty_eight->name = "$28";
    twenty_eight->address = "11100";
    twenty_eight->value = "00000000000000000000000000000000";
    register_array[28] = twenty_eight;
    

    registers* twenty_nine = (registers*) malloc(sizeof(registers));   
    twenty_nine->name = "$29";
    twenty_nine->address = "11101";
    twenty_nine->value = "00000000000000000000000000000000";
    register_array[29] = twenty_nine;
    

    registers* thirty = (registers*) malloc(sizeof(registers));   
    thirty->name = "$30";
    thirty->address = "11110";
    thirty->value = "00000000000000000000000000000000";
    register_array[30] = thirty;
    
    
    registers* thirty_one = (registers*) malloc(sizeof(registers));   
    thirty_one->name = "$31";
    thirty_one->address = "11111";
    thirty_one->value = "00000000000000000000000000000000";
    register_array[31] = thirty_one;
    
    
    registers* low = (registers*) malloc(sizeof(registers));   
    low->name = "LO";
    low->address = "-2";
    low->value = "00000000000000000000000000000000";
    register_array[32] = low;
    
    
    registers* hi = (registers*) malloc(sizeof(registers));   
    hi->name = "HI";
    hi->address = "-1";
    hi->value = "00000000000000000000000000000000";
    register_array[33] = hi;
    
    
    registers* pc = (registers*) malloc(sizeof(registers));   
    pc->name = "pc";
    pc->address = "-3";
    pc->value = "00000000000000000000000000000000";
    register_array[34] = pc;

}

instruction* copy_instruction(instruction* ins)
{
    instruction* temp = (instruction*)malloc(sizeof(instruction));
    temp->type = ins->type;
    temp->binary = strdup(ins->binary);
    temp->Rd = strdup(ins->Rd);
    temp->Rt = strdup(ins->Rt);
    temp->Rs = strdup(ins->Rs);
    temp->shamt = strdup(ins->shamt);
    temp->offset = strdup(ins->offset);
    temp->target = strdup(ins->target);
    return temp;
}

void make_pipeline_registers()
{
    noop = (instruction*)malloc(sizeof(instruction));
    noop->type = 0;
    noop->binary = strdup("00000000000000000000000000000000");
    noop->Rd = strdup("");
    noop->Rt = strdup("");
    noop->Rs = strdup("");
    noop->shamt = strdup("");
    noop->offset = strdup("");
    noop->target = strdup("");

    IF_ID_write = (pipeline_register_1*) malloc(sizeof(pipeline_register_1));   
    IF_ID_write->name = "IF_ID_write";
    IF_ID_write->pc = 0; 
    IF_ID_write->ins = copy_instruction(noop); 
    
    IF_ID_read = (pipeline_register_1*) malloc(sizeof(pipeline_register_1));   
    IF_ID_read->name = "IF_ID_read";
    IF_ID_read->pc = 0;   
    IF_ID_read->ins = copy_instruction(noop);   

    
    ID_EX_write = (pipeline_register_2*) malloc(sizeof(pipeline_register_2));   
    ID_EX_write->name = "ID_EX_write";
    ID_EX_write->op1 = strdup("");
    ID_EX_write->op2 = strdup("");
    ID_EX_write->sign = strdup("");
    ID_EX_write->Rs = strdup("");
    ID_EX_write->Rt = strdup("");
    ID_EX_write->Rd = strdup("");
    ID_EX_write->shamt = strdup("");
    ID_EX_write->target = strdup("");
    ID_EX_write->REG_WRITE = -1;
    ID_EX_write->MEM_TO_REG = -1;
    ID_EX_write->MEM_WRITE = -1;
    ID_EX_write->MEM_READ = -1;
    ID_EX_write->ALU_src_1 = -1;
    ID_EX_write->ALU_src_2 = -1;
    ID_EX_write->zero_extend = -1;
    ID_EX_write->branch = -1;
    ID_EX_write->REG_DEST = -1;
    ID_EX_write->ALU_op = -1;

    ID_EX_read = (pipeline_register_2*) malloc(sizeof(pipeline_register_2));   
    ID_EX_read->name = "ID_EX_read";
    ID_EX_read->op1 = strdup("");
    ID_EX_read->op2 = strdup("");
    ID_EX_read->sign = strdup("");
    ID_EX_read->Rs = strdup("");
    ID_EX_read->Rt = strdup("");
    ID_EX_read->Rd = strdup("");
    ID_EX_read->shamt = strdup("");
    ID_EX_read->target = strdup("");
    ID_EX_read->REG_WRITE = -1;
    ID_EX_read->MEM_TO_REG = -1;
    ID_EX_read->MEM_WRITE = -1;
    ID_EX_read->MEM_READ = -1;
    ID_EX_read->ALU_src_1 = -1;
    ID_EX_read->ALU_src_2 = -1;
    ID_EX_read->zero_extend = -1;
    ID_EX_read->branch = -1;
    ID_EX_read->REG_DEST = -1;
    ID_EX_read->ALU_op = -1;


    EX_MEM_write = (pipeline_register_3*) malloc(sizeof(pipeline_register_3));   
    EX_MEM_write->name = "EX_MEM_write";
    EX_MEM_write->result = strdup("");
    EX_MEM_write->write_data = strdup("");
    EX_MEM_write->write_data_register = strdup("");
    EX_MEM_write->dest = strdup("");
    EX_MEM_write->mult_data = -1;
    EX_MEM_write->REG_WRITE = -1;
    EX_MEM_write->MEM_TO_REG = -1;
    EX_MEM_write->MEM_WRITE = -1;
    EX_MEM_write->MEM_READ = -1;


    EX_MEM_read = (pipeline_register_3*) malloc(sizeof(pipeline_register_3));   
    EX_MEM_read->name = "EX_MEM_read";
    EX_MEM_read->result = strdup("");
    EX_MEM_read->write_data = strdup("");
    EX_MEM_read->write_data_register = strdup("");
    EX_MEM_read->dest = strdup("");
    EX_MEM_read->mult_data = -1;
    EX_MEM_read->REG_WRITE = -1;
    EX_MEM_read->MEM_TO_REG = -1;
    EX_MEM_read->MEM_WRITE = -1;
    EX_MEM_read->MEM_READ = -1;


    MEM_WB_write = (pipeline_register_4*) malloc(sizeof(pipeline_register_4));   
    MEM_WB_write->name = "MEM_WB_write";
    MEM_WB_write->read_data = strdup("");
    MEM_WB_write->dest = strdup("");
    MEM_WB_write->ans = strdup("");
    MEM_WB_write->mult_data = -1;
    MEM_WB_write->REG_WRITE = -1;
    MEM_WB_write->MEM_TO_REG = -1;

    MEM_WB_read = (pipeline_register_4*) malloc(sizeof(pipeline_register_4));   
    MEM_WB_read->name = "MEM_WB_read";
    MEM_WB_read->read_data = strdup("");
    MEM_WB_read->dest = strdup("");
    MEM_WB_read->ans = strdup("");
    MEM_WB_read->mult_data = -1;
    MEM_WB_read->REG_WRITE = -1;
    MEM_WB_read->MEM_TO_REG = -1;
}

void init_memory()
{
    for(int i = 0; i < 67108864; i++)
        memory[i] = strdup("00000000");
}


void make_processor()
{
    faltu_register = (registers*)malloc(sizeof(registers));
    faltu_register->name = strdup("   ");
    faltu_register->value = strdup("00000000000000000000000000000000");
    faltu_register->address = strdup("12345");

    for(int i = 0; i < number; i++)
    {
        fprintf(output, "%d\n", instruction_array[i]->type);
    }
    init_memory();
    make_registers();
    make_pipeline_registers();
    start();
}



void read_cfg()
{
    FILE* cfg_file = fopen(cfg, "r");

    // char* line = calloc(100, sizeof(char));
    char* line = (char*)malloc(100 * sizeof(char));
    char* tokens = calloc(100, sizeof(char));
    int value;



    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // Comment

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // [perf_model/l1_icache]

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // perfect = false

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // cache_size = 8
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_SIZE_I, value);

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // associativity = 1
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_ASSOC_I, value);

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // replacement_policy = lru
    
    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // writethrough = 0
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_WRITETHROUGH_I, value);

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // block_size = 16
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_BLOCK_SIZE_I, value);
    
    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);

    fgets(line, 100, cfg_file);
    // [perf_model/l1_dcache]

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // perfect = false

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // cache_size = 8
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_SIZE_D, value);

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // associativity = 1
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_ASSOC_D, value);

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // replacement_policy = lru
    
    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // writethrough = 0
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_WRITETHROUGH_D, value);

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // block_size = 16
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_BLOCK_SIZE_D, value);

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);

    fgets(line, 100, cfg_file);
    // [perf_model/core]

    fgets(line, 100, cfg_file);
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    // set_cache_param(CACHE_PARAM_BLOCK_SIZE_D, value);

    fgets(line, 100, cfg_file);

    // fscanf(cfg_file, "%s", line);
    fgets(line, 100, cfg_file);
    // [perf_model/dram]

    fgets(line, 100, cfg_file);
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, " ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    printf("--------+++++++++++++++++++++++++++++++------------------%s  %s\n", line, tokens);
    printf("---------------------------------------------------------%s  %s\n", line, tokens);
    printf("---------------------------------------------------------%s  %d\n", line, value);
    // set_cache_param(CACHE_PARAM_BLOCK_SIZE_D, value);
    fclose(cfg_file);

    print_settings();
    init_cache();

    dump_settings();

} 


