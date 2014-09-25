/* ----------------------------------------

* File Name : exec.cpp

* Created on : 07-05-2014

* Last Modified on : Wed 24 Sep 2014 04:38:02 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/

#include <iostream>
#include <string>
#include <stdio.h>
#include "pipeline.h"
//#include"reg_file.h"

using namespace std;



unsigned int checking_forward (unsigned int forward_reg, unsigned int reg_address, unsigned int forward_value){
  unsigned int *address;
  unsigned int value;
  if (forward_reg != reg_address){
    address = &int_reg_file[reg_address];
    value = *address;
  }
  else {
    value = forward_value;
  }
  return value;
}


pipe_exec::pipe_exec() : halt(true), is_exit(false),
  forwarding_value(0),
  forwarding_value_2(0),
  forwarding_reg(0),
  forwarding_reg_2(0),
  int_wb_value(0), 
  int_wb_address(32),
  is_int_wb(false),
  is_float_wb(false),
  float_wb_address(0){
}


exec::exec() : pipeline_exec() {//Constructor
}

pipe_exec exec::exec_instruction(pipe_decode pipe_decode, pipe_control pipe_control, reg reg){
  if (pipe_control.exec_halt){
    pipeline_exec.halt = 1;//pipe_control.exec_halt;
  }
  else {
    pipeline_exec.halt = pipe_decode.halt;
  }
  pipeline_exec.is_exit = 0;//pipe_decode.is_exit;//pipe_decode.is_exit;
  pipeline_exec.pc = pipe_decode.pc;
  pipeline_exec.is_int_wb = 0;
  pipeline_exec.rs_v = 0;
  pipeline_exec.rd_v = 0;
  pipeline_exec.rt_v = 0;
  pipeline_exec.sa = 0xFFFFFFFF;
  pipeline_exec.HI = 0;
  pipeline_exec.LO = 0;
  pipeline_exec.offset_value = 0xFFFFFFFF;
  pipeline_exec.int_wb_address = 0xFFFFFFFF;
  pipeline_exec.is_jump = 0;
  pipeline_exec.jump_address = 0xFFFFFFFF;
  pipeline_exec.is_float_wb = 0;
  pipeline_exec.rs_fv = 0.0;
  pipeline_exec.rd_fv = 0.0;
  pipeline_exec.rt_fv = 0.0;
  pipeline_exec.float_wb_address = 0;
  pipeline_exec.is_mem = 0;
  pipeline_exec.mem_operation = "empty";
  pipeline_exec.effective_address = 0;
  pipeline_exec.value_to_be_store = 0;
  pipeline_exec.rs = pipe_decode.rs;
  pipeline_exec.rt = pipe_decode.rt;
  pipeline_exec.rd = pipe_decode.rd;
  pipeline_exec.int_wb_value = 0;
  pipeline_exec.int_wb_address = 32;

  if (!pipeline_exec.halt){/*halt === 0*/
    if (pipe_decode.iType == "r_type") exec_r_type(pipe_decode, reg);
    else if (pipe_decode.iType == "j_type") exec_j_type(pipe_decode, reg);
    else if (pipe_decode.iType == "c_type") exec_c_type(pipe_decode, reg);
    else if (pipe_decode.iType == "i_type") exec_i_type(pipe_decode, reg);
  }
  return pipeline_exec;
}


/*R type instructions*/
void exec::exec_r_type(pipe_decode pipe_decode, reg reg){

  unsigned int *rs_address;
  unsigned int *rt_address;
  unsigned int *HI_address;
  HI_address = &HI;
  unsigned int *LO_address;
  LO_address = &LO;
  unsigned int *syscall_value;

  pipeline_exec.sa = pipe_decode.sa;


  if (pipe_decode.function == 0X00){ /*sll*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.rd_v = pipeline_exec.rt_v << pipeline_exec.sa; 
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X02){ /*srl*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.rd_v = pipeline_exec.rt_v >> pipeline_exec.sa;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X03){ /*sra*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    if ((pipeline_exec.rt_v >> 31) == 1){/*negative value*/
      pipeline_exec.rd_v = (pipeline_exec.rt_v >> pipeline_exec.sa) | (0xFFFFFFFF << (32 - pipeline_exec.sa));
      pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    }
    else {
      pipeline_exec.rd_v = pipeline_exec.rt_v >> pipeline_exec.sa;
      pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    }
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X04){ /*sllv*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = pipeline_exec.rt_v << pipeline_exec.rs_v;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X06){ /*srlv*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = pipeline_exec.rt_v >> pipeline_exec.rs_v;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X07){ /*srav*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    if ((pipeline_exec.rt >> 31) == 1){
      pipeline_exec.rd_v = (pipeline_exec.rt_v >> pipeline_exec.rs_v) | (0xFFFFFFFF << (32 - pipeline_exec.rs_v));
      pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    }
    else {
      pipeline_exec.rd_v = pipeline_exec.rt_v >> pipeline_exec.rs_v;
      pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    }
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X08){ /*jr*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.is_jump = 1;
    pipeline_exec.halt = 1;
    pipeline_exec.jump_address = pipeline_exec.rs_v;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  else if (pipe_decode.function == 0X09){ /*jalr*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.is_jump = 1;
    pipeline_exec.halt = 1;
    pipeline_exec.jump_address = pipeline_exec.rs_v;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_value = pipeline_exec.pc +4;
    pipeline_exec.int_wb_address = 31;

    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X0C){ //syscall
    unsigned int temp;
    //exception in forwarding for syscall
    if (pipeline_exec.forwarding_reg == 2){
      temp = pipeline_exec.forwarding_value;
    }
    else {
      syscall_value = &int_reg_file[2];
      temp = *syscall_value;
    }
    if (temp == 10){
      pipeline_exec.is_exit = 1;
    }
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //else if (pipe_decode.function == 0X0D) //break
  else if (pipe_decode.function == 0X10){ /*mfhi*/
    //data forwarding is not required for this instructions
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = *HI_address;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X11){ /*mthi*/
    //exception in r-type data forwarding
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    *HI_address = pipeline_exec.rs_v;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  else if (pipe_decode.function == 0X12){ /*mflo*/
    //data forwarding is not required for this instructions
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = *LO_address;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X13){ /*mtlo*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    *LO_address = pipeline_exec.rs_v;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  else if (pipe_decode.function == 0X18){ /*mult*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    unsigned long long temp;
    temp = pipeline_exec.rs_v * pipeline_exec.rt_v;
    *HI_address = temp >> 32;
    *LO_address = (temp & 0x00000000FFFFFFFF);
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  else if (pipe_decode.function == 0X19){ /*multu*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    unsigned long long temp;
    temp = pipeline_exec.rs_v * pipeline_exec.rt_v;
    *HI_address = temp >> 32;
    *LO_address = (temp & 0x00000000FFFFFFFF);
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  else if (pipe_decode.function == 0X1A){ /*div*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    unsigned long long temp;
    temp = pipeline_exec.rs_v / pipeline_exec.rt_v;
    *HI_address = temp >> 32;
    *LO_address = (temp & 0x00000000FFFFFFFF);
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  else if (pipe_decode.function == 0X1B){ //divu
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    unsigned long long temp;
    temp = pipeline_exec.rs_v / pipeline_exec.rt_v;
    *HI_address = temp >> 32;
    *LO_address = (temp & 0x00000000FFFFFFFF);
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  else if (pipe_decode.function == 0X20){ //add (add with overflow)
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    unsigned long long temp;
    temp = pipeline_exec.rs_v + pipeline_exec.rt_v;
    if (((temp & 0x00000000FFFFFFFF) >> 31) == ((temp & 0x00000001FFFFFFFF) >> 32)){
      pipeline_exec.is_int_wb = 1;
      pipeline_exec.int_wb_address = pipe_decode.rd;
      pipeline_exec.rd_v = temp;
      pipeline_exec.int_wb_value = pipeline_exec.rd_v;
      pipeline_exec.int_wb_address = pipeline_exec.rd;
    }
    else {
      pipeline_exec.is_int_wb = 0;
    }
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X21){ /*addu (add without overflow)*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = pipeline_exec.rs_v + pipeline_exec.rt_v;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X22){ /*sub*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    unsigned long long temp;
    temp = pipeline_exec.rs_v - pipeline_exec.rt_v;
    if (((temp & 0x00000000FFFFFFFF) >> 31) == ((temp & 0x00000001FFFFFFFF) >> 32)){
      pipeline_exec.is_int_wb = 1;
      pipeline_exec.int_wb_address = pipe_decode.rd;
      pipeline_exec.rd_v = temp;
      pipeline_exec.int_wb_value = pipeline_exec.rd_v;
      pipeline_exec.int_wb_address = pipeline_exec.rd;
    }
    else {
      pipeline_exec.is_int_wb = 0;
    }
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X23){ /*subu*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = pipeline_exec.rs_v - pipeline_exec.rt_v;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X24){ /*and*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = pipeline_exec.rs_v & pipeline_exec.rt_v;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X25){ /*or*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = pipeline_exec.rs_v | pipeline_exec.rt_v;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X26){ /*xor*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = pipeline_exec.rs_v ^ pipeline_exec.rt_v;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X27){ /*nor*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = ~(pipeline_exec.rs_v | pipeline_exec.rt_v);
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X2A){ /*slt*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = ((int) pipeline_exec.rs_v < (int) pipeline_exec.rt_v)? 1:0;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else if (pipe_decode.function == 0X2B){ /*sltu*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rd;
    pipeline_exec.rd_v = (pipeline_exec.rs_v < pipeline_exec.rd_v)? 1:0;
    pipeline_exec.int_wb_value = pipeline_exec.rd_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else{
    cout << "XXX: R-Type unimplemented instruction detected" << endl;
  }//else //unimplemented instruction
#if 0
  pipeline_exec.forwarding_value_2 = pipeline_exec.forwarding_value;
  pipeline_exec.forwarding_reg_2 = pipeline_exec.forwarding_reg_2;
  pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
  pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
#endif
}



/*J type instructions*/
void exec::exec_j_type(pipe_decode pipe_decode, reg reg){
  if (pipe_decode.opcode == 0X02){ //j
    //forwarding is not required
    pipeline_exec.halt = 1;
    pipeline_exec.is_jump = 1;
    pipeline_exec.jump_address = pipe_decode.instr_index << 2;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  else if (pipe_decode.opcode == 0X03){ //jal
    //forwarding is not required
    pipeline_exec.halt = 1;
    pipeline_exec.is_jump = 1;
    pipeline_exec.jump_address = pipe_decode.instr_index << 2;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_value = pipeline_exec.pc + 4;
    pipeline_exec.int_wb_address = 31;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  else { //unimplemented instruction
    cout << "XXX: J-Type unimplemented instruction detected" << endl;
  }
}


/*C type instructions*/
void exec::exec_c_type(pipe_decode pipe_decode, reg reg){
  //if (pipe_decode.format == 0X00){ /*mfc1*/
  //  if (pipline_decode.function == 0X00)//mfct1
  //  else //unimplemented instructions
  //}
  //
  //else if (pipe_decode.format == 0X04){ /*mtc1*/
  //  if (pipe_decode.function == 0X00)//mtc1
  //  else //unimplemented instruction
  //}
  //
  //else if (pipe_decode.format == 0X10){ /*add.s, sub.s, mul.s, div.s, mov.s, cvt.w.s*/
  //  if (pipe_decode.function == 0X00) //add.s
  //  else if (pipe_decode.function == 0X01) //sub.s
  //  else if (pipe_decode.function == 0X02) //mul.s
  //  else if (pipe_decode.function == 0X03) //div.s
  //  else if (pipe_decode.function == 0X06) //mov.s
  //  else if (pipe_decode.function == 0X24) //cvt.w.s
  //  else //unimplemented instructions
  //}
  //
  //else if (pipe_decode.format == 0X14){ /*cvt.s.w*/
  //  if (pipe_decode.function == 0X20) //cvt.s.w
  //  else //unimplemented instruction
  //}
  //
  //else //unimplemented format
}



/*I type instructions*/
void exec::exec_i_type(pipe_decode pipe_decode, reg reg){

  unsigned int *rs_address;
  unsigned int *rt_address;
  unsigned int *HI_address;
  HI_address = &HI;
  unsigned int *LO_address;
  LO_address = &LO;
#if 0
  if (pipeline_exec.forwarding_reg == pipe_decode.rs/* && pipeline_exec.forwarding_required_1*/){/*forwardinging_needed*/
    pipeline_exec.rs_v = pipeline_exec.forwarding_value;
  }
#if 0
  else if (pipeline_exec.forwarding_reg_2 == pipe_decode.rs/* && pipeline_exec.forwarding_required_2*/){
    pipeline_exec.rs_v = pipeline_exec.forwarding_value_2;
  }
#endif
  else if (pipeline_exec.forwarding_reg == pipe_decode.rt){
    pipeline_exec.rt_v = pipeline_exec.forwarding_value;
  }
  else {
    rs_address = &int_reg_file[pipe_decode.rs];
    pipeline_exec.rs_v = *rs_address;


    rt_address = &int_reg_file[pipe_decode.rt];
    pipeline_exec.rt_v = *rt_address;

    //pipeline_exec.rs_v = reg.read_from_int_reg(pipe_decode.rs);
  }
#endif
  pipeline_exec.sa = pipe_decode.sa;
  pipeline_exec.offset_value = pipe_decode.offset;

  if (pipe_decode.opcode == 0X01){
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    if (pipe_decode.bltz == 0X00){/*bltz*/
      signed short temp = pipe_decode.imm;
      if ((signed)pipeline_exec.rs_v < 0){
        pipeline_exec.is_jump = 1;
        pipeline_exec.jump_address = ((signed int)temp << 2) + pipe_decode.pc + 4 ;
      }
    }
    else if (pipe_decode.bltz == 0X01){/*bgez*/
      signed short temp = pipe_decode.imm;
      if ((signed) pipeline_exec.rs_v >= 0){
        pipeline_exec.is_jump = 1;
        pipeline_exec.jump_address = ((signed int)temp << 2) + pipe_decode.pc + 4 ;
      }
    }
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //1.bltz	(opcode-->0x01)
  //2.bgez	(opcode-->0x01)
  //
  //
  else if (pipe_decode.opcode == 0X04){/*beq*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    signed short temp = pipe_decode.imm;
    if (pipeline_exec.rs_v == pipeline_exec.rt_v){
      pipeline_exec.is_jump = 1;
      pipeline_exec.jump_address = ((signed int)temp << 2) + pipe_decode.pc + 4 ;
    }
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //3.beq	(opdoce-->0x04)
  else if (pipe_decode.opcode == 0X05){/*bne*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    signed short temp = pipe_decode.imm;
    if (pipeline_exec.rs_v != pipeline_exec.rt_v){
      pipeline_exec.is_jump = 1;
      pipeline_exec.jump_address = ((signed int)temp << 2) + pipe_decode.pc + 4 ;
    }
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //4.bne	(opcode-->0x05)
  else if (pipe_decode.opcode == 0X06){/*blez*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    signed short temp = pipe_decode.imm;
    if (pipeline_exec.rs_v <= 0){
      pipeline_exec.is_jump = 1;
      pipeline_exec.jump_address = ((signed int)temp << 2) + pipe_decode.pc + 4 ;
    }
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //5.blez	(opcode-->0x06)
  else if (pipe_decode.opcode == 0X07){/*bgtz*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    signed short temp = pipe_decode.imm;
    if (pipeline_exec.rs_v > 0){
      pipeline_exec.is_jump =1;
      pipeline_exec.jump_address = ((signed int)temp << 2) + pipe_decode.pc + 4 ;
    }
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //6.bgtz	(opcode-->0x07)
  //
  //
  else if (pipe_decode.opcode == 0X08){/*addi*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.rt_v = pipeline_exec.rs_v + pipeline_exec.offset_value;//FIXME for the overflow
    pipeline_exec.int_wb_value = pipeline_exec.rt_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  //7.addi	(opcode-->0x08)
  else if (pipe_decode.opcode == 0X09){/*addiu*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    //signed short temp = pipeline_exec.offset_value & 0x0000FFFF;
    //pipeline_exec.rt_v = pipeline_exec.rs_v + temp;
    signed short temp = pipeline_exec.offset_value;
    signed temp2 = pipeline_exec.rs_v + temp;
    //printf("OFFSET:%d",temp);
    //printf("\tSRC1:%d ",pipeline_exec.rs_v);
    //printf("\tRES:%d\n",temp2);

    pipeline_exec.int_wb_value = temp2;
    //pipeline_exec.rt_v = pipeline_exec.rs_v + pipeline_exec.offset_value;
    //pipeline_exec.int_wb_value = pipeline_exec.rt_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  //8.addiu	(opcode-->0x09)
  //
  //
  else if (pipe_decode.opcode == 0X0A){/*slti*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    if ((signed) pipeline_exec.rs_v < (signed) pipeline_exec.offset_value){//FIXME for signed int
      pipeline_exec.rt_v = 1;
    }
    else{
      pipeline_exec.rt_v = 0;
    }
    pipeline_exec.int_wb_value = pipeline_exec.rt_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  //9.slti	(opcode-->0x0A)
  else if (pipe_decode.opcode == 0X0B){/*sltiu*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    if (pipeline_exec.rs_v < pipeline_exec.offset_value){
      pipeline_exec.rt_v = 1;
    }
    else{
      pipeline_exec.rt_v = 0;
    }
    pipeline_exec.int_wb_value = pipeline_exec.rt_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  //10.sltiu	(opcode-->0x0B)
  else if (pipe_decode.opcode == 0X0C){/*andi*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.rt_v = pipeline_exec.rs_v & pipeline_exec.offset_value;
    pipeline_exec.int_wb_value = pipeline_exec.rt_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  //11.andi	(opcode-->0x0C)
  else if (pipe_decode.opcode == 0X0D){/*ori*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.rt_v = pipeline_exec.rs_v | pipeline_exec.offset_value;
    pipeline_exec.int_wb_value = pipeline_exec.rt_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  //12.ori	(opcode-->0x0D)
  else if (pipe_decode.opcode == 0X0E){/*xori*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.rt_v = pipeline_exec.rs_v ^ pipeline_exec.offset_value;
    pipeline_exec.int_wb_value = pipeline_exec.rt_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  //13.xori	(opcode-->0x0E)
  else if (pipe_decode.opcode == 0X0F){/*lui*/
    //exception forwarding is not required
    pipeline_exec.halt = 1;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.rt_v = (pipeline_exec.offset_value << 16) | 0X0000;
    pipeline_exec.int_wb_value = pipeline_exec.rt_v;
    pipeline_exec.forwarding_value = pipeline_exec.int_wb_value;
    pipeline_exec.forwarding_reg = pipeline_exec.int_wb_address;
  }
  //14.lui	(opcode-->0x0F)
  //
  //
  else if (pipe_decode.opcode == 0X20){/*lb*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.is_mem = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.mem_operation = "lb";
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //15.lb	(opcode-->0x20)
  else if (pipe_decode.opcode == 0X21){/*lh*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.is_mem = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.mem_operation = "lh";
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //16.lh	(opcode-->0x21)
  //
  //
  else if (pipe_decode.opcode == 0X23){/*lw*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.is_mem = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.mem_operation = "lw";
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    //pipeline_exec.int_wb_value = 0;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //17.lw	(opcode-->0x23)
  else if (pipe_decode.opcode == 0X24){/*lbu*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.is_mem = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.mem_operation = "lbu";
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    //pipeline_exec.int_wb_value = 0;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //18.lbu	(opcode-->0x24)
  else if (pipe_decode.opcode == 0X25){/*lhu*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.is_mem = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.mem_operation = "lhu";
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    //pipeline_exec.int_wb_value = 0;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //19.lhu	(opcode-->0x25)
  //
  //
  else if (pipe_decode.opcode == 0X28){/*sb*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 0;
    pipeline_exec.is_mem = 1;
    pipeline_exec.mem_operation = "sb";
    pipeline_exec.value_to_be_store = pipeline_exec.rt_v;
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //20.sb	(opcode-->0x28)
  else if (pipe_decode.opcode == 0X29){/*sh*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 0;
    pipeline_exec.is_mem = 1;
    pipeline_exec.mem_operation = "sh";
    pipeline_exec.value_to_be_store = pipeline_exec.rt_v;
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //21.sh	(opcode-->0x29)
  //
  //
  else if (pipe_decode.opcode == 0X2B){/*sw*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 0;
    pipeline_exec.is_mem = 1;
    pipeline_exec.mem_operation = "sw";
    pipeline_exec.value_to_be_store = pipeline_exec.rt_v;
    //printf("%x\n",pipeline_exec.rt_v);
    //printf("SW: Value to be store %x\n", pipeline_exec.value_to_be_store);
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //22.sw	(opcode-->0x2B)
  //
  //
  else if (pipe_decode.opcode == 0X31){/*lwc1*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 1;
    pipeline_exec.is_mem = 1;
    pipeline_exec.int_wb_address = pipeline_exec.rt;
    pipeline_exec.mem_operation = "lwc1";
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    //pipeline_exec.int_wb_value = 0;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //23.lwc1	(opcode-->0x31)
  //
  //
  else if (pipe_decode.opcode == 0X39){/*swc1*/
    pipeline_exec.rs_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rs, pipeline_exec.forwarding_value);
    pipeline_exec.rt_v = checking_forward (pipeline_exec.forwarding_reg, pipe_decode.rt, pipeline_exec.forwarding_value);
    pipeline_exec.halt = 0;
    pipeline_exec.is_int_wb = 0;
    pipeline_exec.is_mem = 1;
    pipeline_exec.mem_operation = "swc1";
    pipeline_exec.value_to_be_store = pipeline_exec.rt_v;
    pipeline_exec.effective_address = pipeline_exec.rs_v + pipeline_exec.offset_value;
    pipeline_exec.forwarding_value = 0;
    pipeline_exec.forwarding_reg = 32;
  }
  //24.swc1	(opcode-->0x39)
  
  else {
    cout << "XXX: I-Type unimplemented instruction detected" << endl;
    //unimplemented instruction
  }
}
