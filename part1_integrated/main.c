#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include "processor.h"
#include "cache.h"

void store_multiply(double a);
void store_multiply_add(double a);
char* multiply(double a);
char* multiply_add(double a);

// GLobal variables
int number;
char* memory[67108864];
instruction* instruction_array[20000];
registers* register_array[34];
int type_array[6];
instruction* print_instruction_array[6];
int svg_array[5];
instruction* noop;
int cycle;
int idle_cycles = 0;
char* breakpoint_array[100];
int breakpoints_count = 0;

pipeline_register_1* IF_ID_write;
pipeline_register_1* IF_ID_read;
pipeline_register_2* ID_EX_write;
pipeline_register_2* ID_EX_read;
pipeline_register_3* EX_MEM_write;
pipeline_register_3* EX_MEM_read;
pipeline_register_4* MEM_WB_write;
pipeline_register_4* MEM_WB_read;

// Global
int pc = 0;
int pc_branch = 0;
char* pc_branch_addr = "";
int branch_flag = 0;
int wb_done = 0;
int continue_flag = 0;
int run_flag = 0;

// Control signals
int PC_SRC = 0;
int fwdA = 0;
int fwdB = 0;
int fwd_Result = 0;
int fwd_lo = 0;
int fwd_write_data = 0;
int stall = 0;
int default_31 = 0;
int update_registers = 0;

// File variables
int end;
int rise_fetch;
int rise_decode;
int rise_execute;
int rise_memory;
int rise_write_back;
int stall;
int no_controller = 0;
int fetch_done = 1;
int decode_done = 1;
int execute_done = 1;
int mem_done = 1;
int write_done = 1;

// Functions
void update()
{
    IF_ID_read->pc = IF_ID_write->pc;
    IF_ID_read->ins = copy_instruction(IF_ID_write->ins);

    ID_EX_read->op1 = ID_EX_write->op1;
    ID_EX_read->op2 = ID_EX_write->op2;
    ID_EX_read->sign = strdup(ID_EX_write->sign);
    ID_EX_read->Rs = strdup(ID_EX_write->Rs);
    ID_EX_read->Rt = strdup(ID_EX_write->Rt);
    ID_EX_read->Rd = strdup(ID_EX_write->Rd);
    ID_EX_read->shamt = strdup(ID_EX_write->shamt);
    ID_EX_read->target = strdup(ID_EX_write->target);
    ID_EX_read->REG_WRITE = ID_EX_write->REG_WRITE;
    ID_EX_read->MEM_TO_REG = ID_EX_write->MEM_TO_REG;
    ID_EX_read->MEM_WRITE = ID_EX_write->MEM_WRITE;
    ID_EX_read->MEM_READ = ID_EX_write->MEM_READ;
    ID_EX_read->ALU_src_1 = ID_EX_write->ALU_src_1;
    ID_EX_read->ALU_src_2 = ID_EX_write->ALU_src_2;
    ID_EX_read->zero_extend = ID_EX_write->zero_extend;
    ID_EX_read->branch = ID_EX_write->branch;
    ID_EX_read->ALU_op = ID_EX_write->ALU_op;
    ID_EX_read->REG_DEST = ID_EX_write->REG_DEST;

    EX_MEM_read->result = strdup(EX_MEM_write->result);
    EX_MEM_read->write_data = strdup(EX_MEM_write->write_data);
    EX_MEM_read->write_data_register = strdup(EX_MEM_write->write_data_register);
    EX_MEM_read->dest = strdup(EX_MEM_write->dest);
    EX_MEM_read->mult_data = EX_MEM_write->mult_data;
    EX_MEM_read->REG_WRITE = EX_MEM_write->REG_WRITE;
    EX_MEM_read->MEM_TO_REG = EX_MEM_write->MEM_TO_REG;
    EX_MEM_read->MEM_WRITE = EX_MEM_write->MEM_WRITE;
    EX_MEM_read->MEM_READ = EX_MEM_write->MEM_READ;

    MEM_WB_read->read_data = strdup(MEM_WB_write->read_data);
    MEM_WB_read->dest = strdup(MEM_WB_write->dest);
    MEM_WB_read->ans = strdup(MEM_WB_write->ans);
    MEM_WB_read->mult_data = MEM_WB_write->mult_data;
    MEM_WB_read->REG_WRITE = MEM_WB_write->REG_WRITE;
    MEM_WB_read->MEM_TO_REG = MEM_WB_write->MEM_TO_REG;   
}

void print_breakpoint_array()
{
    int i;
    for(i=0;i<breakpoints_count;i++)
    {
        printf("%d - %s\n", (i+1),breakpoint_array[i]);
    }
}

int check_for_breakpoint(char* pc)
{
    int i;
    for(i=0;i<breakpoints_count;i++)
    {
        if(strcmp(breakpoint_array[i],pc) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void* clock_function(void* args)
{
    rise_fetch = 0;
    rise_decode = 0;
    rise_execute = 0;
    rise_memory = 0;
    rise_write_back = 0;
    cycle = 0;
    register_array[34]->value = dec_to_bin(4194304);
    register_array[29]->value = dec_to_bin(1024 + 268500992);
    char* temp = calloc(100,sizeof(char));
    char * pch = calloc(100,sizeof(char));
    while(end != 1)
    {
        printf("----------\n");
        char* pc_check = calloc(100,sizeof(char));
        pc_check = strdup("0x");
        pc_check = concat(pc_check,bin_to_hex(register_array[34]->value,32));
        int break_flag = check_for_breakpoint(pc_check);
        if(break_flag == 1 && continue_flag == 1) printf("Reached breakpoint - %s\n", pc_check);
        continue_flag = continue_flag - break_flag;
        no_controller = 0;
        if(run_flag != 1 && continue_flag != 1)
        {
            scanf("%s", pch);
            if(strcmp("step",pch) != 0)
                no_controller = 1;
        }
        
        if (cycle > 1)
        {
            if(no_controller == 0) control_function();
            update();
        }

        svg_array[0] = type_array[1] > 0 ? 1 : 0;
        svg_array[1] = type_array[2] > 0 ? 1 : 0;
        svg_array[2] = type_array[3] > 0 ? 1 : 0;
        svg_array[3] = type_array[4] > 0 ? 1 : 0;
        svg_array[4] = type_array[5] > 0 ? 1 : 0;
        char* a;
        char* b;
        char* c;
        char* d;
        char* e;

        a = strdup(print_ins(print_instruction_array[1]));
        b = strdup(print_ins(print_instruction_array[2]));
        c = strdup(print_ins(print_instruction_array[3]));
        d = strdup(print_ins(print_instruction_array[4])); 
        e = strdup(print_ins(print_instruction_array[5]));
        build_svg(svg_array[0],svg_array[1],svg_array[2],svg_array[3],svg_array[4], a, b, c, d, e, fetch_done, decode_done, execute_done, mem_done, write_done);

        if ((strcmp("step", pch) == 0) || run_flag == 1 || continue_flag == 1)
        {


            if(run_flag == 1 || continue_flag == 1)
            {
                sleep(1);
            }

            if (stall == 0 && PC_SRC == 0)
            {
                rise_fetch = 1;
                rise_decode = 1;
                rise_execute = 1;
                rise_memory = 1;
                rise_write_back = 1;
            }
            if (stall == 1 && PC_SRC == 0)
            {
                rise_memory = 1;
                rise_write_back = 1;
            }
            if (stall == 0 && PC_SRC == 1)
            {
                rise_fetch = 1;
                rise_memory = 1;
                rise_write_back = 1;
            }

            sleep(1);

            if (stall == 0 && PC_SRC == 0)
            {
                type_array[5] = type_array[4];
                type_array[4] = type_array[3];
                type_array[3] = type_array[2];
                type_array[2] = type_array[1];
                type_array[1] = type_array[0];

                print_instruction_array[5] = print_instruction_array[4];
                print_instruction_array[4] = print_instruction_array[3];
                print_instruction_array[3] = print_instruction_array[2];
                print_instruction_array[2] = print_instruction_array[1];
                print_instruction_array[1] = print_instruction_array[0];

            }
            if (stall == 1 && PC_SRC == 0)
            {
                type_array[5] = type_array[4];
                type_array[4] = type_array[3];
                type_array[3] = 0;
                print_instruction_array[5] = print_instruction_array[4];
                print_instruction_array[4] = print_instruction_array[3];
                print_instruction_array[3] = copy_instruction(noop);
                EX_MEM_write->dest = "00002";
            }
            if (stall == 0 && PC_SRC == 1)
            {
                type_array[5] = type_array[4];
                type_array[4] = type_array[3];
                type_array[3] = 0;
                type_array[2] = 0;
                type_array[1] = type_array[0];

                print_instruction_array[5] = print_instruction_array[4];
                print_instruction_array[4] = print_instruction_array[3];
                print_instruction_array[3] = copy_instruction(noop);
                print_instruction_array[2] = copy_instruction(noop);
                print_instruction_array[1] = print_instruction_array[0];
            }
        
            rise_fetch = 0;
            rise_decode = 0;
            rise_execute = 0;
            rise_memory = 0;
            rise_write_back = 0;
        
            update();      
            cycle++;
        }
        else if(strcmp("continue", pch) == 0)
        {
            continue_flag = check_for_breakpoint(pc_check) + 1;
        }
        else if(strcmp("break", pch) == 0)
        {
            char* point = calloc(100,sizeof(char));
            scanf("%s", point);
            if(point[0] != '0' || point[1] != 'x' || strlen(point) != 10)
            {
                printf("Invalid breakpoint format\n");
            }
            else
            {
                breakpoint_array[breakpoints_count++] = strdup(point);
                printf("Successfully added breakpoint\n");
            }
        }
        else if(strcmp("run", pch) == 0)
        {
            run_flag = 1;
        }
        else if(strcmp("delete", pch) == 0)
        {
            char* point = calloc(100,sizeof(char));
            scanf("%s", point);
            if(point[0] != '0' || point[1] != 'x' || strlen(point) != 10)
            {
                printf("Invalid breakpoint format\n");
            }
            else
            {
                int x;
                for(x=0;x<breakpoints_count;x++)
                {
                    if(strcmp(breakpoint_array[x],point) == 0)
                    {
                        breakpoint_array[x] = "0x00000000";
                        printf("Successfully deleted breakpoint\n");
                        break;
                    }
                }
                int y;
                for(y=x;y<breakpoints_count-1;y++){
                    breakpoint_array[y] = breakpoint_array[y+1];
                }
                breakpoints_count--;
            }
        }
        else if(strcmp("print_breakpoints",pch) == 0)
        {
            print_breakpoint_array();
        }

        else if (strcmp("memdump", pch) == 0)
        {
            scanf("%s", pch);
            char* ad = strdup(pch);
            ad += 2;
            int total;
            scanf("%d", &total);
            char* tt = hex_to_bin(ad);
            int add = (int) (bin_to_dec(tt) - 268500992);
            for (int i = 0; i < total; i++)
            {
                char* t = bin_to_hex(memory[add], 8);
                printf("0x%s:\t0x%s\t - %s\n", ad, t, memory[add]);
                ad = bin_to_hex(dec_to_bin(bin_to_dec(hex_to_bin(ad)) + 1.0), 32);
                add++;
            }
        }
        else if (strcmp("regdump", pch) == 0)
        {
            char* t;
            for (int i = 0; i < 32; i++)
            {
                t = bin_to_hex(register_array[i]->value, 32);
                // printf("%s\n", bin_to_hex(register_array[i]->value, 32));
                printf("$%d:\t0x%s\t%s\t%d\n", i, t, register_array[i]->value, (int)bin_to_dec(register_array[i]->value));
            }
            t = bin_to_hex(register_array[32]->value, 32);
            printf("lo:\t0x%s\t%s\t%d\n", t, hex_to_bin(t), (int)bin_to_dec(hex_to_bin(t)));
            t = bin_to_hex(register_array[33]->value, 32);
            printf("hi:\t0x%s\t%s\t%d\n", t, hex_to_bin(t), (int)bin_to_dec(hex_to_bin(t)));
            t = bin_to_hex(register_array[34]->value, 32);
            printf("PC:\t0x%s\t%s\t%d\n", t, hex_to_bin(t), (int)bin_to_dec(hex_to_bin(t)));
        }
        else
        {
            printf("Error input.\n");
            continue;
        }

    }
}

void print_type_array()
{
    for(int i = 0; i < 5; i++)
        printf("%d ", type_array[i]);

    printf("\n");    
}

void* fetch_function(void* args)
{
    int done = 0;
    int idx =   0;
    while(end != 1)
    {
        if(done != 1) fetch_done = 0;
        sleep(0.5);
        idx = ((int)bin_to_dec(register_array[34]->value) - 4194304)/4;
        if(rise_fetch == 0)
        {
            done = 0;
            fetch_done = 1;
            continue;
        }
        if(done == 1)
        {
            fetch_done = 1;
            continue;
        }
        if(PC_SRC == 1) register_array[34]->value = strdup(pc_branch_addr);
        if(idx >= number)
        {
            type_array[0] = -1;
            continue;
        }
        printf("PC_SRC = %d\n", PC_SRC);
        //printf("pc_branch_addr = %s || PC = %s\n", pc_branch_addr,register_array[34]->value);
        printf("PC = 0x%s\t%d\n", bin_to_hex(register_array[34]->value,32), ((int)bin_to_dec(register_array[34]->value) - 4194304)/4);
        pc = ((int)bin_to_dec(register_array[34]->value) - 4194304)/4;
        instruction* ins = instruction_array[pc++];
        register_array[34]->value = strdup(dec_to_bin(pc*4 + 4194304));
        printf("PC = 0x%s\t%d\n", bin_to_hex(register_array[34]->value,32), ((int)bin_to_dec(register_array[34]->value) - 4194304)/4);

        printf("%d(%d) -> %d\n", idx,number,ins->type);

        type_array[0] = ins->type;
        print_instruction_array[0] = copy_instruction(ins);
        IF_ID_write->pc = pc;
        IF_ID_write->ins = copy_instruction(ins);

        done = 1;
        printf("Fetch Done\n");
    }
}

void* decode_function(void* args)
{
    int done = 0;
    while(end != 1)
    {
        sleep(0.5);
        if (rise_decode == 0)
        {
            done = 0;
            continue;
        }

        if (done == 1)
            continue;

        if (type_array[1] == 0 || type_array[1] == -1)
            continue;  

        // printf("\nPIPELINE 1 READ\n");
        // print_p_1(IF_ID_write);
        // printf("\n");

        // printf("DECODE Control Signals - \n");
        // printf("MEM_WB_read->REG_WRITE - %d\n", MEM_WB_read->REG_WRITE);

        instruction* ins = IF_ID_read->ins;   
        if(MEM_WB_read->REG_WRITE == 1)
        {
            while(wb_done == 0)
            {
                sleep(0.1);
                printf("WAITING\n");
            }
            printf("wb_done = %d\n", wb_done);
        }

        ID_EX_write->op1 = strdup((find_register(ins->Rs))->value);
        ID_EX_write->op2 = strdup((find_register(ins->Rt))->value);    
        ID_EX_write->Rs = strdup(ins->Rs);
        ID_EX_write->Rt = strdup(ins->Rt);    
        ID_EX_write->Rd = strdup(ins->Rd);    
        ID_EX_write->sign = strdup(sign_extend(ins->offset, 16));
        ID_EX_write->shamt = strdup(ins->shamt);
        ID_EX_write->target = strdup(sign_extend(ins->target, 26));
        ID_EX_write->ALU_op = (IF_ID_read->ins)->type;

        int t = (int) bin_to_dec(sign_extend(ins->offset, 16));
        int branch_to = IF_ID_read->pc + t;
        // printf("Branch_to - %d - %d\n", IF_ID_read->pc,branch_to);
        ID_EX_write->branch = branch_to;

        // printf("\nPIPELINE 2 WRITE\n");
        // print_p_2(ID_EX_write);
        // printf("\n");

        done = 1;
        printf("Decode done\n");
    }
}

void* execute_function(void* args)
{
    int done = 0;
    while(end != 1)
    {
        sleep(0.5);
        if (rise_execute == 0)
        {
            done = 0;
            continue;
        }

        if (done == 1)
            continue;

        if (type_array[2] == 0 || type_array[2] == -1)
            continue;        
        
        char* x = calloc(100,sizeof(char));
        char* y = calloc(100,sizeof(char));

        // printf("\nPIPELINE 2 READ\n");
        // print_p_2(ID_EX_write);
        // printf("\n");

        // printf("EXECUTE Control Signals - \n");
        printf("fwdA - %d\n", fwdA);
        printf("fwdB - %d\n", fwdB);
        printf("fwd_write_data - %d\n", fwd_write_data);
        printf("fwd_lo - %d\n", fwd_lo);
        // printf("MEM_WB_read->MEM_TO_REG - %d\n", MEM_WB_read->MEM_TO_REG);
        printf("ALU_src_1 - %d\n", ID_EX_read->ALU_src_1);
        printf("ALU_src_2 - %d\n", ID_EX_read->ALU_src_2);
        // printf("zero_extend - %d\n", ID_EX_read->zero_extend);
        // printf("REG_DEST - %d\n", ID_EX_read->REG_DEST);

        // printf("Other Control Signals\n");
        // printf("REG_WRITE - %d\n", ID_EX_read->REG_WRITE);
        // printf("MEM_TO_REG - %d\n", ID_EX_read->MEM_TO_REG);
        // printf("MEM_WRITE - %d\n", ID_EX_read->MEM_WRITE);
        // printf("MEM_READ - %d\n", ID_EX_read->MEM_READ);

        if (fwdA == 0)
        {
            if(ID_EX_read->ALU_src_1 == 1)
            {
                printf("1. OP_1 - $%d - %s\t%s\n", (int)bin_to_dec(zero_extend_func(ID_EX_read->Rs,5)),ID_EX_read->op1,ID_EX_read->Rs);
                x = strdup(ID_EX_read->op1);
            }
            else if(ID_EX_read->ALU_src_1 == 0)
            {
                printf("2. OP_1 - shamt - %s\n", sign_extend(ID_EX_read->shamt,5));
                x = strdup(sign_extend(ID_EX_read->shamt,5));
            }
            else if(ID_EX_read->ALU_src_1 == 2)
            {
                printf("6. OP_1 - target - %s\n", ID_EX_read->target);
                x = strdup(ID_EX_read->target);
            }
            else if(ID_EX_read->ALU_src_1 == 3)
            {
                if(fwd_lo == 0)
                {
                    printf("7.0. OP_1 - lo - %s\n", register_array[32]->value);
                    x = strdup(register_array[32]->value);
                }
                else if(fwd_lo == 1)
                {
                    printf("7.1. OP_1 - lo - %s\n", EX_MEM_read->result);
                    x = strdup(EX_MEM_read->result);
                }
                else if(fwd_lo == 2)
                {
                    printf("7.2. OP_1 - lo - %s\n", MEM_WB_read->ans);
                    x = strdup(MEM_WB_read->ans);
                }
            }
        }
        else if(fwdA == 1)
        {
            printf("3. OP_1 - EX_MEM_read->result - %s\n", EX_MEM_read->result);
            x = strdup(EX_MEM_read->result);                
        }
        else if(fwdA == 2)
        {
            if(MEM_WB_read->MEM_TO_REG == 1)
            {
                printf("4. OP_1 - MEM_WB_read->read_data - %s\n", MEM_WB_read->read_data);
                x = strdup(MEM_WB_read->read_data);
            }
            else
            {
                printf("5. OP_1 - MEM_WB_read->ans - %s\n", MEM_WB_read->ans);
                x = strdup(MEM_WB_read->ans);
            }
        }

        if (fwdB == 0)
        {
            if(ID_EX_read->ALU_src_2 == 1)
            {
                printf("1. OP_2 - $%d - %s\t%s\n", (int)bin_to_dec(zero_extend_func(ID_EX_read->Rt,5)),ID_EX_read->op2,ID_EX_read->Rt);
                y = strdup(ID_EX_read->op2);
            }
            else
            {
                if(ID_EX_read->zero_extend == 1)
                {
                    printf("2. OP_2 - Zero_Extend - %s\n", zero_extend_func(ID_EX_read->sign + 16, 16));
                    y = strdup(zero_extend_func(ID_EX_read->sign + 16, 16));
                }
                else
                {
                    printf("3. OP_2 - Sign_Extend - %s\n", ID_EX_read->sign);
                    y = strdup(ID_EX_read->sign);
                }
            }
        }
        else if(fwdB == 1)
        {
            printf("4. OP_2 - EX_MEM_read->result - %s\n", EX_MEM_read->result);
            y = strdup(EX_MEM_read->result);
        }
        else if(fwdB == 2)
        {
            if(MEM_WB_read->MEM_TO_REG)
            {
                printf("5. OP_2 - MEM_WB_read->read_data - %s\n", MEM_WB_read->read_data);
                y = strdup(MEM_WB_read->read_data);
            }
            else
            {
                printf("6. OP_2 - MEM_WB_read->ans - %s\n", MEM_WB_read->ans);
                y = strdup(MEM_WB_read->ans);
            }
        }

        if(ID_EX_read->REG_DEST == 1)
        {
            printf("1. DEST - Rt - $%d - %s\n", (int)bin_to_dec(zero_extend_func(ID_EX_read->Rt,5)),ID_EX_read->Rt);
            EX_MEM_write->dest = ID_EX_read->Rt;
        }
        if(ID_EX_read->REG_DEST == 0)
        {
            if(default_31 == 0)
            {
                printf("2.1. DEST - Rd - $%d - %s\n", (int)bin_to_dec(zero_extend_func(ID_EX_read->Rd,5)),ID_EX_read->Rd);
                EX_MEM_write->dest = ID_EX_read->Rd;
            }
            else if(default_31 == 1)
            {
                printf("2.2. DEST - Rd - $%d - %s\n", (int)bin_to_dec(zero_extend_func("11111",5)),"11111");
                EX_MEM_write->dest = "11111";
            }
        }
        if(ID_EX_read->REG_DEST == 2)
        {
            printf("3. DEST - No Destination for opcode %d\n", ID_EX_read->ALU_op);
        }

        if(fwd_write_data == 0)
        {
            EX_MEM_write->write_data = strdup(ID_EX_read->op2);
        }
        else if(fwd_write_data == 1)
        {
            EX_MEM_write->write_data = EX_MEM_read->result;
        }
        else if(fwd_write_data == 2)
        {
            if(MEM_WB_read->MEM_TO_REG)
            {
                EX_MEM_write->write_data = MEM_WB_read->read_data;
            }
            else
            {
                EX_MEM_write->write_data = MEM_WB_read->ans;
            }
        }
        EX_MEM_write->write_data_register = ID_EX_read->Rt;

        printf("ALU_OP - %d\n", ID_EX_read->ALU_op);
        printf("ALU_PARAM_1 - %s = %d\n", x,(int)bin_to_dec(x));
        printf("ALU_PARAM_2 - %s = %d\n", y,(int)bin_to_dec(y));

        char* t = alu(x, y,ID_EX_read->ALU_op);
        printf("ALU_RESULT - %s = %d\n", t,(int)bin_to_dec(t));
        pc_branch_addr = strdup(dec_to_bin(ID_EX_read->branch*4 + 4194304));
        EX_MEM_write->result = t;
        EX_MEM_write->MEM_WRITE = ID_EX_read->MEM_WRITE;
        EX_MEM_write->MEM_READ = ID_EX_read->MEM_READ;
        EX_MEM_write->MEM_TO_REG = ID_EX_read->MEM_TO_REG;
        EX_MEM_write->REG_WRITE = ID_EX_read->REG_WRITE;

        // printf("\nPIPELINE 3 WRITE\n");
        // print_p_3(EX_MEM_write);
        // printf("\n");

        done = 1;
        printf("Execute done\n");
    }
}

void* memory_function(void* args)
{
    int done = 0;
    while(end != 1)
    {
        sleep(0.5);
        if (rise_memory == 0)
        {
            done = 0;
            continue;
        }

        if (done == 1)
            continue;

        if (type_array[3] == 0 || type_array[3] == -1)
            continue;

        // printf("\nPIPELINE 3 READ\n");
        // print_p_3(EX_MEM_write);
        // printf("\n");

        // printf("MEMORY Control Signals - \n");
        // printf("EX_MEM_read->MEM_WRITE - %d\n", EX_MEM_read->MEM_WRITE);
        // printf("EX_MEM_read->MEM_READ - %d\n", EX_MEM_read->MEM_READ);

        // printf("MEMORY Control Signals - \n");       
        // printf("EX_MEM_read->MEM_WRITE - %d\n", EX_MEM_read->MEM_WRITE);
        // printf("EX_MEM_read->MEM_READ - %d\n", EX_MEM_read->MEM_READ);

        printf("fwd_Result - %d\n", fwd_Result);

        char* actual_result;
        if(fwd_Result == 1)
        {
            actual_result = MEM_WB_read->MEM_TO_REG == 1 ? MEM_WB_read->read_data : MEM_WB_read->ans;
            printf("1. Actual result = %s\n", actual_result);
            if(MEM_WB_read->MEM_TO_REG == 1)
                printf("MEM_WB_read->read_data = %s\n", MEM_WB_read->read_data);
            if(MEM_WB_read->MEM_TO_REG == 0)
                printf("MEM_WB_read->ans = %s\n", MEM_WB_read->ans);
        }
        if(fwd_Result == 0)
        {
            actual_result = strdup(EX_MEM_read->write_data);
            printf("2. Actual result = %s\n", actual_result);
            printf("EX_MEM_read->write_data = %s\n", EX_MEM_read->write_data);
        }
        int addr = (int)(bin_to_dec(EX_MEM_read->result));
        
        MEM_WB_write->ans = EX_MEM_read->result;    
        MEM_WB_write->dest = EX_MEM_read->dest;
        MEM_WB_write->mult_data = EX_MEM_read->mult_data;

        MEM_WB_write->REG_WRITE = EX_MEM_read->REG_WRITE;
        MEM_WB_write->MEM_TO_REG = EX_MEM_read->MEM_TO_REG;

        char* aa;
        char* t;
        switch(type_array[3])
        {
            case 20:
                if(EX_MEM_read->MEM_READ == 1) //ld
                {
                    aa = perform_access(addr * 4, 0, 0, "000000");
                    // aa = strdup(memory[addr * 4]);
                    aa = sign_extend(aa, 8);                   
                    MEM_WB_write->read_data = strdup(aa);
                    printf("20 - Loaded data %s = %d\n", MEM_WB_write->read_data,(int)bin_to_dec(MEM_WB_write->read_data));
                }   
                break;


            case 21:
                if(EX_MEM_read->MEM_READ == 1) //ld
                {
                    t = perform_access(addr * 4, 0, 0, "000000");
                    // t = strdup(memory[addr * 4]);
                    aa = perform_access(addr * 4 + 1, 0, 0, "000000");
                    // aa = strdup(memory[addr * 4 + 1]);
                    strncat(t, aa, 8);
                    aa = perform_access(addr * 4 + 2, 0, 0, "000000");
                    // aa = strdup(memory[addr * 4 + 2]);
                    strncat(t, aa, 8);
                    aa = perform_access(addr * 4 + 3, 0, 0, "000000");
                    // aa = strdup(memory[addr * 4 + 3]);
                    strncat(t, aa, 8);                
                    MEM_WB_write->read_data = strdup(t);
                    printf("21 - Loaded data %s = %d\n", MEM_WB_write->read_data,(int)bin_to_dec(MEM_WB_write->read_data));
                }                   
                break;

            case 22:  
                if(EX_MEM_read->MEM_WRITE == 1) //st
                {
                    // aa = strdup(actual_result + 24);
                    // memory[addr * 4] = strdup(aa);
                    perform_access(addr * 4, 1, 0, aa);
                    
                    printf("22 - Stroring %s at %d\n", aa,(addr * 4));
                }             
                break;
            
            case 23:           
                if(EX_MEM_read->MEM_WRITE == 1) //st
                {
                    printf("23 - Stroring value %s = %d\n", actual_result,(int)bin_to_dec(actual_result));
                    perform_access(addr * 4, 1, 0, slice(actual_result, 8));
                    // memory[addr * 4]     = slice(actual_result, 8);
                    perform_access(addr * 4 + 1, 1, 0, slice(actual_result + 8, 8));
                    // memory[addr * 4 + 1] = slice(actual_result + 8, 8);
                    perform_access(addr * 4 + 2, 1, 0, slice(actual_result + 16, 8));
                    // memory[addr * 4 + 2] = slice(actual_result + 16, 8);
                    perform_access(addr * 4 + 3, 1, 0, slice(actual_result + 24, 8));
                    // memory[addr * 4 + 3] = slice(actual_result + 24, 8);
                }   
        }


        // printf("\nPIPELINE 4 WRITE\n");
        // print_p_4(MEM_WB_write);
        // printf("\n");

        done = 1;
        printf("Memory done\n");
    }
}

void* write_back_function(void* args)
{   
    int done = 0;
    wb_done = 0;
    while(end != 1)
    {
        
        if (rise_write_back == 0)
        {
            done = 0;
            continue;
        }

        if (done == 1)
            continue;

        if(type_array[4] == 0 || type_array[4] == -1)
            continue;        

        wb_done = 0;
        // printf("\nPIPELINE 4 READ\n");
        // print_p_4(MEM_WB_write);
        // printf("\n");

        // printf("WB Control Signals - \n");
        // printf("MEM_WB_read->MEM_TO_REG - %d\n", MEM_WB_read->MEM_TO_REG);
        printf("MEM_WB_read->REG_WRITE - %d\n", MEM_WB_read->REG_WRITE);

        char* y = calloc(100,sizeof(char));
        if(MEM_WB_read->MEM_TO_REG == 0)
            y = strdup(MEM_WB_read->ans);

        else
            y = strdup(MEM_WB_read->read_data);

        if(MEM_WB_read->REG_WRITE == 1)
        {
            registers* dest_reg = find_register(MEM_WB_read->dest);
            int value = (int)bin_to_dec(y);
            int reg_index = (int)bin_to_dec(zero_extend_func(MEM_WB_read->dest,5));
            printf("Storing %d at $%d\n",value,reg_index);
            printf("Initial - %s = %d\n", dest_reg->name,(int)bin_to_dec(register_array[reg_index]->value));
            dest_reg->value = strdup(y);
            printf("Final - %s = %d\n", dest_reg->name,(int)bin_to_dec(register_array[reg_index]->value));
            if(update_registers != 0) 
                printf("Update registers = %d\n", update_registers);
            if(update_registers == 1)
            {
                ID_EX_write->op1 = strdup(y);
                ID_EX_read->op1 = strdup(y);
            }
            if(update_registers == 2)
            {
                ID_EX_write->op2 = strdup(y);
                ID_EX_read->op2 = strdup(y);
            }
        }
        if(MEM_WB_read->REG_WRITE == 2)
        {
            if(type_array[4] == 4) store_multiply(MEM_WB_read->mult_data);
            if(type_array[4] == 5) store_multiply_add(MEM_WB_read->mult_data);
            MEM_WB_read->REG_WRITE = 0;
        }
            
        done = 1;  
        wb_done = 1;  
        printf("Write_Back done\n");
    }
}

void control_function()
{
    printf("Controller Input - \n");
    print_type_array();
    printf("Branch Flag - %d\n", branch_flag);

    if(branch_flag == 1)
    {
        PC_SRC = 1;
        printf("Branching to - %d\n",((int)bin_to_dec(pc_branch_addr) - 4194304)/4);
    }
    else
    {
        PC_SRC = 0;
    }
    branch_flag = 0;

    if(type_array[2] != 9 && type_array[2] != 24 && type_array[2] != 25 && type_array[2] != 29)
    {
        ID_EX_write->ALU_src_1 = 1;
    }
    if(type_array[2] == 24 || type_array[2] == 25)
    {
        ID_EX_write->ALU_src_1 = 2;
    }
    if(type_array[2] == 28)
    {
        ID_EX_write->ALU_src_1 = 3;
    }
    if(type_array[2] == 9)
    {
        ID_EX_write->ALU_src_1 = 0;
    }

    if(type_array[2] == 28 && (type_array[3] == 4 || type_array[3] == 5))
    {
        fwd_lo = 1;
    }
    else if(type_array[2] == 28 && (type_array[3] == 4 || type_array[3] == 5))
    {
        fwd_lo = 2;
    }
    else
    {
        fwd_lo = 0;
    }

    if(type_array[2] == 8)
    {
        ID_EX_write->zero_extend = 1;
    }
    else
    {
        ID_EX_write->zero_extend = 0;
    }

    if(type_array[2] == 1 || type_array[2] == 3 || type_array[2] == 4 || type_array[2] == 5 || type_array[2] == 6 || type_array[2] == 7 || type_array[2] == 9 || type_array[2] == 10 || type_array[2] == 11 || type_array[2] == 13 || type_array[2] == 15 || type_array[2] == 29 || type_array[2] == 30 || type_array[2] == 32)
    {
        ID_EX_write->ALU_src_2 = 1;
    }
    else
    {
        ID_EX_write->ALU_src_2 = 0;
    }

    stall = 0;
    update_registers = 0;
    if((EX_MEM_read->REG_WRITE == 1) && (strcmp(EX_MEM_read->dest,ID_EX_read->Rs) == 0) && (ID_EX_write->ALU_src_1 == 1))
    {
        fwdA = 1;
        if(EX_MEM_read->MEM_TO_REG == 1)
        {
            stall = 1;
            if(strcmp(ID_EX_read->Rs,MEM_WB_read->dest) == 0) 
                update_registers = 1;
            if(strcmp(ID_EX_read->Rt,MEM_WB_read->dest) == 0)
                update_registers = 2;
            printf("Stalling due fwdA\n");
            fwdA = 0;
        }
    }
    else if((MEM_WB_read->REG_WRITE == 1) && (strcmp(MEM_WB_read->dest,ID_EX_read->Rs) == 0) && (ID_EX_write->ALU_src_1 == 1))
    {
        fwdA = 2;
    }
    else
    {
        fwdA = 0;
    }

    if((EX_MEM_read->REG_WRITE == 1) && (strcmp(EX_MEM_read->dest,ID_EX_read->Rt) == 0) && (ID_EX_write->ALU_src_2 == 1))
    {
        fwdB = 1;
        if(EX_MEM_read->MEM_TO_REG == 1)
        {
            stall = 1;
            if(strcmp(ID_EX_read->Rs,MEM_WB_read->dest) == 0) 
                update_registers = 1;
            if(strcmp(ID_EX_read->Rt,MEM_WB_read->dest) == 0)
                update_registers = 2;
            printf("Stalling due fwdB\n");
            fwdB = 0;
        }
    }
    else if((MEM_WB_read->REG_WRITE == 1) && (strcmp(MEM_WB_read->dest,ID_EX_read->Rt) == 0) && (ID_EX_write->ALU_src_2 == 1))
    {
        fwdB = 2;
    }
    else
    {
        fwdB = 0;
    }

    if((EX_MEM_read->REG_WRITE == 1) && (strcmp(EX_MEM_read->dest,ID_EX_read->Rt) == 0) && (type_array[2] == 22 || type_array[2] == 23))
    {
        fwd_write_data = 1;
        if(EX_MEM_read->MEM_TO_REG == 1)
        {
            stall = 1;
            if(strcmp(ID_EX_read->Rs,MEM_WB_read->dest) == 0) 
                update_registers = 1;
            if(strcmp(ID_EX_read->Rt,MEM_WB_read->dest) == 0)
                update_registers = 2;
            printf("Stalling due fwd_write_data\n");
            fwd_write_data = 0;
        }
    }
    else if((MEM_WB_read->REG_WRITE == 1) && (strcmp(MEM_WB_read->dest,ID_EX_read->Rt) == 0) && (type_array[2] == 22 || type_array[2] == 23))
    {
        fwd_write_data = 2;
    }
    else
    {
        fwd_write_data = 0;
    }

    if((type_array[3] == 22 || type_array[3] == 23) && (strcmp(MEM_WB_read->dest,EX_MEM_read->write_data_register) == 0) && (MEM_WB_read->REG_WRITE == 1))
    {
        fwd_Result = 1;
    }
    else
    {
        fwd_Result = 0;
    }

    ID_EX_write->REG_WRITE = 0;
    if(type_array[2] != 15 && type_array[2] != 16 && type_array[2] != 17 && type_array[2] != 18 && type_array[2] != 19 && type_array[2] != 22 && type_array[2] != 23 && type_array[2] != 24 && type_array[2] != 27 && type_array[2] != 32)
    {
        ID_EX_write->REG_WRITE = 1;
    }
    if(type_array[2] == 4 || type_array[2] == 5)
    {
        ID_EX_write->REG_WRITE = 2;
    }

    if(type_array[2] == 20 || type_array[2] == 21)
    {
        ID_EX_write->MEM_TO_REG = 1;
        ID_EX_write->MEM_READ = 1;
    }
    else
    {
        ID_EX_write->MEM_TO_REG = 0;
        ID_EX_write->MEM_READ = 0;
    }

    if(type_array[2] == 22 || type_array[2] == 23)
    {
        ID_EX_write->MEM_WRITE = 1;
    }
    else
    {
        ID_EX_write->MEM_WRITE = 0;
    }

    if(type_array[2] == 2 || type_array[2] == 8 || type_array[2] == 12 || type_array[2] == 14 || type_array[2] == 20 || type_array[2] == 21 || type_array[2] == 31)
    {
        ID_EX_write->REG_DEST = 1;
    }
    else if(type_array[2] == 4 || type_array[2] == 5 || (type_array[2] < 20 && type_array[2] > 14) || type_array[2] == 22 || type_array[2] == 23 || type_array[2] == 24 || type_array[2] == 27 || type_array[2] == 32)
    {
        ID_EX_write->REG_DEST = 2;
    }
    else
    {
        ID_EX_write->REG_DEST = 0;
    }

    if(type_array[2] == 25)
    {
        default_31 = 1;
    }
    else
    {
        default_31 = 0;
    }
}   

char* bit_and(char* a, char* b)
{
    char* ans = calloc(32, sizeof(char));
    int* aa = calloc(32, sizeof(32));
    for(int i = 0; i < 32; i++)
        aa[i] = (a[i] - '0') & (b[i] - '0');

    for(int i = 0; i < 32; i++)
    {
        char y = aa[i] + '0';
        strncat(ans, &y, 1); 
    }

    return ans;
}

void store_multiply(double a)
{
    if (a < 0)
        a = pow(2, 64) + a;

    int* ans = calloc(64, sizeof(int));
    int idx = 63;
    while(a > 0)
    {
        ans[idx] = fmod(a , 2);
        a = floor(a / 2);
        idx--;
    }
    char* q = calloc(32, sizeof(char));
    for(int i = 0; i < 32; i++)
    {
        char y = ans[i] + '0';
        strncat(q, &y, 1);
    }
    register_array[33]->value = strdup(q);
    char* w = calloc(32, sizeof(char));
    for(int i = 32; i < 64; i++)
    {
        char y = ans[i] + '0';
        strncat(w, &y, 1);
    }
    register_array[32]->value = strdup(w);
}

void store_multiply_add(double a)
{
    if (a < 0)
        a = (long long int)pow(2, 64) + a;

    int* ans = calloc(64, sizeof(int));
    int idx = 63;
    while(a > 0)
    {
        ans[idx] = fmod(a , 2);
        a = floor(a / 2);
        idx--;
    }
    char* q = calloc(32, sizeof(char));
    for(int i = 0; i < 32; i++)
    {
        char y = ans[i] + '0';
        strncat(q, &y, 1);
    }
    char* y = dec_to_bin(bin_to_dec(register_array[33]->value) + bin_to_dec(q));
    register_array[33]->value = strdup(y);
    char* w = calloc(32, sizeof(char));
    for(int i = 32; i < 64; i++)
    {
        char y = ans[i] + '0';
        strncat(w, &y, 1);
    }
    char* s = dec_to_bin(bin_to_dec(register_array[32]->value) + bin_to_dec(w));
    register_array[32]->value = strdup(s);
}

char* multiply(double a)
{
    if (a < 0)
        a = pow(2, 64) + a;

    int* ans = calloc(64, sizeof(int));
    int idx = 63;
    while(a > 0)
    {
        ans[idx] = fmod(a , 2);
        a = floor(a / 2);
        idx--;
    }
    char* q = calloc(32, sizeof(char));
    for(int i = 0; i < 32; i++)
    {
        char y = ans[i] + '0';
        strncat(q, &y, 1);
    }
    char* w = calloc(32, sizeof(char));
    for(int i = 32; i < 64; i++)
    {
        char y = ans[i] + '0';
        strncat(w, &y, 1);
    }
    return w;
}

char* multiply_add(double a)
{
    if (a < 0)
        a = (long long int)pow(2, 64) + a;

    int* ans = calloc(64, sizeof(int));
    int idx = 63;
    while(a > 0)
    {
        ans[idx] = fmod(a , 2);
        a = floor(a / 2);
        idx--;
    }
    char* q = calloc(32, sizeof(char));
    for(int i = 0; i < 32; i++)
    {
        char y = ans[i] + '0';
        strncat(q, &y, 1);
    }
    char* y = dec_to_bin(bin_to_dec(register_array[33]->value) + bin_to_dec(q));
    char* w = calloc(32, sizeof(char));
    for(int i = 32; i < 64; i++)
    {
        char y = ans[i] + '0';
        strncat(w, &y, 1);
    }
    char* s;
    s = strdup(dec_to_bin(bin_to_dec(register_array[32]->value) + bin_to_dec(w)));
    return s;
}

char* bit_nor(char* a, char* b)
{
    char* ans = calloc(32, sizeof(char));
    int* aa = calloc(32, sizeof(32));
    for(int i = 0; i < 32; i++)
        aa[i] = (a[i] - '0') | (b[i] - '0');

    for(int i = 0; i < 32; i++)
        aa[i] = aa[i] == 0 ? 1 : 0; 
        
    for(int i = 0; i < 32; i++)
    {
        char y = aa[i] + '0';
        strncat(ans, &y, 1); 
    }
    
    return ans;
}

char* bit_or(char* a, char* b)
{
    char* ans = calloc(32, sizeof(char));
    int* aa = calloc(32, sizeof(32));
    for(int i = 0; i < 32; i++)
        aa[i] = (a[i] - '0') | (b[i] - '0');
        
    for(int i = 0; i < 32; i++)
    {
        char y = aa[i] + '0';
        strncat(ans, &y, 1); 
    }
    
    return ans;
}

char* shift_left(char* a, char* b)
{
    long int c = (long int)bin_to_dec(a);
    long int d = (long int)bin_to_dec(b);
    c = c << d;
    double e = (double)c;
    return dec_to_bin (e);
}

char* load_upper(char* a)
{
    char* ans = calloc(32, sizeof(char));
    memcpy(ans, a+16, 16);
    char y = '0';
    for(int i = 16; i < 32; i++)
        strncat(ans, &y, 1);

    return ans;    
}

char* less_than_unsigned(char* a, char* b)
{
    double x = 0;
    double y = 0;
    for(int i = 0; i < 32; i++)
        x += (a[i] - '0') * pow(2, 31 - i); 
   
    for(int i = 0; i < 32; i++)
        y += (b[i] - '0') * pow(2, 31 - i); 
            
    char* ans = calloc(32, sizeof(char));
    if (x < y)
        ans = "00000000000000000000000000000001";

    else
        ans = "00000000000000000000000000000000";

    return ans;    
}

char* less_than_offset(char* a, char* b)
{
    double x = bin_to_dec(a);
    double y = bin_to_dec(b);
    char* ans = calloc(32, sizeof(char));
    if (x < y)
        ans = "00000000000000000000000000000001";

    else
        ans = "00000000000000000000000000000000";

    return ans;    
}

char* alu(char* x, char* y,int ALU_op)
{
    double a, b, c;
    char* aa;
    char* t = strdup("00000000000000000000000000000000");
    long long int add;
    branch_flag = 0;
    switch(ALU_op)
    {
        case 1:
            a = bin_to_dec(x);
            b = bin_to_dec(y);
            c = a + b;
            t = dec_to_bin(c);
            break;

        case 2:  
            a = bin_to_dec(x);
            b = bin_to_dec(y);
            c = a + b;
            t = dec_to_bin(c);
            break;    

        case 3:
            t = bit_and(x, y);
            break;

        case 4:
            a = bin_to_dec(x);
            b = bin_to_dec(y);
            c = a * b;
            EX_MEM_write->mult_data = c;
            t = multiply(c);
            break;

        case 5:
            a = bin_to_dec(x);
            b = bin_to_dec(y);
            c = a * b;
            EX_MEM_write->mult_data = c;
            t = multiply(c);
            break;
            
        case 6:
            t = bit_nor(x, y);
            break;

        case 7:
            t = bit_or(x, y);
            break;

        case 8:
            t = bit_or(x, y);
            break;

        case 9:
            t = shift_left(y, x);
            break;

        case 10:
            t = shift_left(y, x);
            break;
    
        case 11:
            a = bin_to_dec(x);
            b = bin_to_dec(y);
            c = a - b;
            t = dec_to_bin(c);
            break;    
    
        case 12:
            t = load_upper(y);
            break;    

        case 13:
            t = less_than_unsigned(x, y);
            break;    

        case 14:
            t = less_than_offset(x, y);
            break;    

        case 15:        
           a = bin_to_dec(x);
           b = bin_to_dec(y);
           if (a == b)
            {
                branch_flag = 1;
                printf("branch_flag set\n");
            }

            break;    
    
        case 16:        
            a = bin_to_dec(x);
            if (a >= 0)
            {
                branch_flag = 1;
                printf("branch_flag set\n");
            }

            break;    

        case 17:        
            a = bin_to_dec(x);
            if (a > 0)
            {
                branch_flag = 1;
                printf("branch_flag set\n");
            }

            break;    
            
        case 18:        
            a = bin_to_dec(x);
            if (a <= 0)
            {
                branch_flag = 1;
                printf("branch_flag set\n");
            }

            break;  

        case 19:        
            a = bin_to_dec(x);
            if (a < 0)
            {
                branch_flag = 1;
                printf("branch_flag set\n");
            }

            break;    
              
        case 20:     
            add = (long long int) (bin_to_dec(x) + bin_to_dec(y));
            add -= 268500992;
            if (add < 0)
                printf("Error: Negative memory addressing.\n");

            t = dec_to_bin(add);
            break;    
               
        case 21:        
            add = (long long int) (bin_to_dec(x) + bin_to_dec(y));
            add -= 268500992;
            if (add < 0)
                printf("Error: Negative memory addressing.\n");

            t = dec_to_bin(add);
            break;    
            
        case 22:        
            add = (long long int) (bin_to_dec(x) + bin_to_dec(y));
            add -= 268500992;
            if (add < 0)
                printf("Error: Negative memory addressing.\n");

            t = dec_to_bin(add);
            break;    
        
        case 23:
            add = (long long int) (bin_to_dec(x) + bin_to_dec(y));
            add -= 268500992;
            if (add < 0)
                printf("Error: Negative memory addressing.\n");

            t = dec_to_bin(add);
            break; 

        case 24:
            branch_flag = 1;
            ID_EX_read->branch = ((int)bin_to_dec(x)*4 - 4194304)/4;
            break;

        case 25:
            branch_flag = 1;
            t = dec_to_bin(ID_EX_read->branch - (int)bin_to_dec(ID_EX_read->sign));
            ID_EX_read->branch = ((int)bin_to_dec(x)*4 - 4194304)/4;
            break;

        case 26:
            branch_flag = 1;
            t = dec_to_bin(ID_EX_read->branch - (int)bin_to_dec(ID_EX_read->sign));
            ID_EX_read->branch = bin_to_dec(x);
            break;

        case 27:
            branch_flag = 1;
            ID_EX_read->branch = bin_to_dec(x);
            break;

        case 28:
            t = x;
            break;

        case 29:
            t = y;
            break;

        case 30:
            a = bin_to_dec(x);
            b = bin_to_dec(y);
            if(a < b)
                t = "00000000000000000000000000000001";
            else
                t = "00000000000000000000000000000000";
            break;

        case 31:
            a = bin_to_dec(x);
            b = bin_to_dec(y);
            c = a + b;
            t = dec_to_bin(c);
            break;

        case 32:
            a = bin_to_dec(x);
            b = bin_to_dec(y);
            if (a != b)
            {
                branch_flag = 1;
                printf("branch_flag set\n");
            }
            break;    
    }
    return t;
}

char* print_ins(instruction* ins)
{
    char* ans = calloc(100, sizeof(char));
    int type = ins->type;
    if (type == 0)
    {
        ans = strdup("NOOP");
        return ans;
    }

    switch(type)
    {
        case 1:
            ans = concat(ans, "add ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            break;

        case 2:
            ans = concat(ans, "addi ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", imm");
            break;

        case 3:
            ans = concat(ans, "and ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            break;

        case 4:
            ans = concat(ans, "mult ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            break;
     
        case 5:
            ans = concat(ans, "madd ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            break;

        case 6:
            ans = concat(ans, "nor ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            break;
            
        case 7:
            ans = concat(ans, "or ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            break;
     
        case 8:
            ans = concat(ans, "ori ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", imm");
            break;
    
        case 9:
            ans = concat(ans, "sll ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", shamt");
            break;
        
        case 10:
            ans = concat(ans, "sllv ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            break;
        
        case 11:
            ans = concat(ans, "sub ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            break;
        
        case 12:
            ans = concat(ans, "lui ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", imm");
            break;
        
        case 13:
            ans = concat(ans, "sltu ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            break;
        
        case 14:
            ans = concat(ans, "slti ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", imm");
            break;
        
        case 15:
            ans = concat(ans, "beq ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", label");
            break;
        
        case 16:
            ans = concat(ans, "bgez ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", label");
            break;

        case 17:
            ans = concat(ans, "bgtz ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", label");
            break;

        case 18:
            ans = concat(ans, "blez ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", label");
            break;
    
        case 19:
            ans = concat(ans, "bltz ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", label");
            break;
    
        case 20:
            ans = concat(ans, "lb ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", address");
            break;

        case 21:
            ans = concat(ans, "lw ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", address");
            break;

        case 22:
            ans = concat(ans, "sb ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", address");
            break;

        case 23:
            ans = concat(ans, "sw ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", address");
            break;

        case 24:
            ans = concat(ans, "j ");
            ans = concat(ans, ", target");
            break;

        case 25:
            ans = concat(ans, "jal ");
            ans = concat(ans, ", target");
            break;

        case 26:
            ans = concat(ans, "jalr ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            break;

        case 27:
            ans = concat(ans, "jr ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            break;

        case 28:
            ans = concat(ans, "mflo ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            break;

        case 29:
            ans = concat(ans, "move ");
            ans = concat(ans, (find_register(ins->Rd))->name);
            ans = concat(ans, (find_register(ins->Rs))->name);
            break;

        case 30:
            ans = concat(ans, "slt ");
        
        case 31:
            ans = concat(ans, "addiu ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", imm");
            break;

        case 32:
            ans = concat(ans, "bne ");
            ans = concat(ans, (find_register(ins->Rs))->name);
            ans = concat(ans, ", ");
            ans = concat(ans, (find_register(ins->Rt))->name);
            ans = concat(ans, ", label");
            break;

    }

    return ans;
}

// int main()
void start()
{
    type_array[0] = 0;
    type_array[1] = 0;
    type_array[2] = 0;
    type_array[3] = 0;
    type_array[4] = 0;

    print_instruction_array[0] = copy_instruction(noop);
    print_instruction_array[1] = copy_instruction(noop);
    print_instruction_array[2] = copy_instruction(noop);
    print_instruction_array[3] = copy_instruction(noop);
    print_instruction_array[4] = copy_instruction(noop);
    print_instruction_array[5] = copy_instruction(noop);

    // printf("%d\n", number);
    end = 0;
    pthread_t clock_thread;
    pthread_create(&clock_thread , NULL, clock_function, NULL);
    pthread_t fetch_thread, decode_thread, execute_thread, memory_thread, write_back_thread, display_thread;
    pthread_create(&fetch_thread , NULL, fetch_function, NULL);
    pthread_create(&decode_thread, NULL, decode_function, NULL);
    pthread_create(&execute_thread, NULL, execute_function, NULL);
    pthread_create(&memory_thread, NULL, memory_function, NULL);
    pthread_create(&write_back_thread, NULL, write_back_function, NULL);
    // pthread_create(&display_thread, NULL, display_function, NULL);

    while(1)
    {
        if (type_array[4] == -1)
        {
            end = 1;
            break;
        }   
        sleep(1); 
    }
    sleep(2);
    pthread_exit(NULL);
    print_result_file(cycle, number, idle_cycles);
    return;
}