/* ----------------------------------------

* File Name : core.cpp

* Created on : 07-05-2014

* Last Modified on : Thu 25 Sep 2014 04:01:06 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/


#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include "pipeline.h"
#include "read_file.h"
#include <time.h>
#include <stdlib.h>

using namespace std;

unsigned int STATUS;
unsigned int noc;

unsigned char i_mem[MEMSIZE];
unsigned char d_mem[MEMSIZE];
float float_reg_file[32];
unsigned int int_reg_file[32];
unsigned int HI;
unsigned int LO;
unsigned int read_elf (const char *);
pipe_control pipeline_control(pipe_fetch, pipe_decode, pipe_exec, pipe_mem, pipe_wb);//unsigned int next_pc, pipe_fetch pipe_fetch

void print_message(pipe_fetch, pipe_decode, pipe_exec, pipe_mem, pipe_wb, unsigned int *);

int main(int argc, char** argv){
  char *prog;
  unsigned int flag;
  unsigned int flag_dump;
  unsigned int trace_on = 0x00000000;
  unsigned int trace_off = 0x00000000;
  unsigned char *memp;
  noc = 0;
  flag = 0X00000000;
  //unsigned int next_pc;
  for (argc--, argv++; argc; argc--, argv++){
    if (**argv != '-'){/*regards as a file name*/
      strcpy (prog = (char*)malloc(strlen(*argv)+1), *argv);
      break;
    }
    switch (*(*argv + 1)){
      case 't':
	flag |= TRACE_PIPE;
	break;
      case 'f':
	flag |= TRACE_FETCH;
	break;
      case 'd':
	flag |= TRACE_DECODE;
	break;
      case 'e':
	flag |= TRACE_EXEC;
	break;
      case 'm':
	flag |= TRACE_MA;
	break;
      case 'w':
	flag |= TRACE_WB;
	break;
      case 'b':
	flag |= USE_BRANCH_PRED;
	break;
      case 'c':
	flag |= USE_CACHE_SIM;
	break;
      case 'S':
	flag_dump |= USE_TRACE_RANGE;
	sscanf(*argv+2, "%x", &trace_on);
	trace_off = 0xffffffff;
	break;
      case 'E':
	flag_dump |= USE_TRACE_RANGE;
	sscanf(*argv+2, "%x", &trace_off);
	break;
      default:
	printf("Usage:\tsim [-Option1 -Option2 ....] <object>\n");
	printf("\t -t: Trace pipeline\n");
	printf("\t -d: Trace decode stage\n");
	printf("\t -e: Trace execution stage\n");
	printf("\t -m: Trace memory access stage\n");
	printf("\t -w: Trace write back stage\n");
	printf("\t -b: Use branch prediction\n");
	printf("\t -c: Use cache simulator\n");
        printf("\t -Sxxxx: trace(begin steps\n");
        printf("\t -Exxxx: trace(end steps\n");
	exit(1);
    }
  }

  printf(RED" MIPS 32-bit Simulator\n"RESET);
  printf(RED" Vesion: 0.3\n"RESET);
  printf(RED" Trace_flag = 0x%8.8X\n"RESET, flag);

  if (flag_dump & USE_TRACE_RANGE){
    printf(" TRACE ON Value: %x\n", trace_on);
    printf(" TRACE OFF Value: %x\n", trace_off);
  }
/*This is an example update*/

/*  for (int i = 0; i < MEMSIZE; ++i){
    memp = &i_mem[i];
    *memp = 0;
  }*/

  pipe_control pipe_control;

  pipe_control.npc = read_elf(prog);
  printf(" Start address = 0x%8.8X\n", pipe_control.npc);
  memp = &i_mem[MINADDR];
  printf(" Malloc top address = 0x%8.8X\n", (*(memp+3)<<24|*(memp+2)<<16|*(memp+1)<<8|*(memp+0)));
  memp = &i_mem[MINADDR+4];
  printf(" Malloc limit = 0x%8.8X\n", (*(memp+3)<<24|*(memp+2)<<16|*(memp+1)<<8|*(memp+0)));
  memp = &i_mem[MINADDR+8];
  printf(" Stack pointer = 0x%8.8X\n", (*(memp+3)<<24|*(memp+2)<<16|*(memp+1)<<8|*(memp+0)));
  memp = &i_mem[MINADDR+12];
  *memp = argc;
  *(memp + 1) = argc >> 8;
  *(memp + 2) = argc >> 16;
  *(memp + 3) = argc >> 24;

  int i = MINADDR+0X80;
  for (memp+=4; argc; argc--, argv++, memp+=4){
    *memp = i;
    *(memp + 1) = i >> 8;
    *(memp + 2) = i >> 16;
    *(memp + 3) = i >> 24;
    do {
      i_mem[i++] = **argv;
    } while (*(*argv)++);
  }

  for (i = 0; i < 256; i++){
    if ((i % 4) == 0)
      cout << " ";
    printf("%0.02x",i_mem[MINADDR+i]);
    if ((i % 32) == 31){
      cout << endl;
    }
  }

  STATUS = STATUS_NORMAL;


  //define the register file and the memory (the classes for the memory and the register file)
  reg reg;

  unsigned int *temp;
  for (int i=0; i < 32; i++){
    temp = &int_reg_file[i];
    *temp = 0;
  }


  //imem imem;
  dmem dmem;
  //icache icache;
  //dcache dcache;
  
  //define the pipeline stages (the classes that will execute the pipelines)
  fetch fetch;
  exec exec;
  decode decode;
  mem mem;
  wb wb;

  //define the pipeline reg (mainly the struct which contains the information of the pipeline stage)
  pipe_fetch pipe_fetch;
  pipe_decode pipe_decode;
  pipe_exec pipe_exec;
  pipe_mem pipe_mem;
  pipe_wb pipe_wb;


  //stack pointer
  int_reg_file[29] = 0x00FFFF0D;

  /*mainly the struct which contains the information of the pipeline stage*/
  pipe_control.decode_halt = 0;
  pipe_control.exec_halt = 0;
  pipe_wb.is_exit = 0;
  pipe_mem.is_exit = 0;
  pipe_exec.is_exit = 0;



  //printf("%d %d",pipe_control.decode_halt,pipe_control.exec_halt);
  pipe_decode.halt = 1;
  pipe_exec.halt = 1;
  pipe_mem.halt = 1;
  pipe_wb.halt = 1;



  while (STATUS == STATUS_NORMAL/*STATUS != STATUS_ERROR || STATUS != STATUS_EXIT*/){
    pipe_wb = wb.write_back(pipe_mem, reg);
    pipe_mem = mem.mem_access (pipe_exec, dmem);
    pipe_exec = exec.exec_instruction (pipe_decode, pipe_control, reg);
    pipe_decode = decode.decode_instruction (pipe_fetch, pipe_control);
    pipe_fetch = fetch.fetch_instruction(pipe_control);

    print_message(pipe_fetch, pipe_decode, pipe_exec, pipe_mem, pipe_wb, &flag);
    pipe_control = pipeline_control(pipe_fetch, pipe_decode, pipe_exec, pipe_mem, pipe_wb);
  }

  cout << " ==Simulation Ends Normally==\n";
  return 0;
}


pipe_control pipeline_control(pipe_fetch pipe_fetch, pipe_decode pipe_deocde, pipe_exec pipe_exec, pipe_mem pipe_mem, pipe_wb pipe_wb){
  pipe_control pipe_control;
  unsigned int exit_couter;/*this is the exit counter. after four cycle the exit counter is be zero and exit from the loop*/
  if (STATUS == STATUS_NORMAL){
    pipe_control.npc = pipe_fetch.npc;
    if (pipe_exec.is_jump){
      pipe_control.npc = pipe_exec.jump_address;
      pipe_control.decode_halt = 1;
      pipe_control.exec_halt = 1;
    }
    else{
      pipe_control.decode_halt = 0;
      pipe_control.exec_halt = 0;
    }

    if (pipe_wb.is_exit){
      STATUS = STATUS_EXIT;
    }
  }

  if (STATUS == STATUS_EXIT){
    printf(" ==Total Number of Cycle: %8.8x\n", noc);
  }
  noc++;
  return pipe_control;
}



void print_message(pipe_fetch pipe_fetch, pipe_decode pipe_decode, pipe_exec pipe_exec, pipe_mem pipe_mem, pipe_wb pipe_wb, unsigned int *flag){
  if (*flag){
    printf("\n");
    printf("  ================");
    printf("Cycle %d",noc);
    printf("================\n");
  }

  if (*flag & TRACE_FETCH || *flag & TRACE_PIPE){
    printf(RED"  IF:: "RESET);
    printf("pc: 0x%8.8X, Instruction: 0x%8.8X\n", pipe_fetch.pc, pipe_fetch.instruction);
  }  

  if (*flag & TRACE_DECODE || *flag & TRACE_PIPE){
    printf(YELLOW"  ID:: "RESET);
    if (pipe_decode.halt){
      printf("Halt");
    }
    else {
      cout << pipe_decode.iType << ", ";
      printf("OPCODE: %6.6X, FUNCT: %6.6X, RS: %d, RT: %d, RD: %d", pipe_decode.opcode, pipe_decode.function, pipe_decode.rs, pipe_decode.rt, pipe_decode.rd);
    }
    printf("\n");
  }



  if (*flag & TRACE_EXEC || *flag & TRACE_PIPE){
    printf(GREEN"  EX:: "RESET);
    if (pipe_exec.is_jump){
      printf("JUMP ADDRESS: %8.8x", pipe_exec.jump_address);
    }
    else if (pipe_exec.is_mem){
      printf("MEM ADDRESS: %8.8x ", pipe_exec.effective_address);
      if (pipe_exec.is_int_wb){
        printf("WB ADDRESS (LD): %d", pipe_exec.int_wb_address);
      }
      else{
        printf("STORE VALUE (ST): %d", pipe_exec.value_to_be_store);
      }
    }
    else {
      printf("RS_V: %d, RT_V: %d, RD_V: %d, IMM: %d, WB VALUE: %d, WB ADDRESS:%d", pipe_exec.rs_v, pipe_exec.rt_v, pipe_exec.rd_v,pipe_exec.offset_value,pipe_exec.int_wb_value, pipe_exec.int_wb_address);
    }
    printf("\n");
  }



  if (*flag & TRACE_MA || *flag & TRACE_PIPE){
    printf(BLUE"  MA:: "RESET);
    if (pipe_mem.is_int_wb || pipe_mem.is_float_wb){
      printf("LD or from EX stage");
    }
    else {
      printf("ST instructions or HALT");
    }
#if 0
    if (pipe_mem.halt){
      if (pipe_mem.is_int_wb || pipe_mem.is_float_wb){
        printf("LD or other instruction");
        //printf("NOT A MEMORY INSTRUCTION");
      }
      else {
        printf("HALT");
      }
    }
    else {
      printf("MEMORY INSTRUCTION");
    }
#endif
    printf("\n");
  }



  if (*flag & TRACE_WB || *flag & TRACE_PIPE){
    printf(MAGENTA"  WB:: "RESET);
    if (pipe_wb.halt){
      printf("HALT");
    }
    else {
      printf("WB ADDRESS: %d, WB VALUE:%x",pipe_wb.int_wb_address,pipe_wb.int_wb_value);
    }
    printf("\n");
  }
}
