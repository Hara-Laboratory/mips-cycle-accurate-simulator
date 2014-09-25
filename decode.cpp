/* ----------------------------------------

* File Name : decode.cpp

* Created on : 07-05-2014

* Last Modified on : Wed 24 Sep 2014 04:31:29 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/


#include <iostream>
#include <string>
#include "pipeline.h"
#include <stdio.h>

using namespace std;


pipe_decode::pipe_decode() : halt(true), is_exit(false), iType("empty"), opcode(0), format(0XFFFFFFFF){
}


decode::decode() : pipeline_decode() {
//  pipeline_decode.halt = 1;
}

pipe_decode decode::decode_instruction(pipe_fetch pipe_fetch, pipe_control pipe_control){
  /*updating the control information*/
  unsigned int opCode;
  if (pipe_control.decode_halt){
    pipeline_decode.halt = 1;//pipe_control.decode_halt;
    //cout << "DECODE " << pipe_control.decode_halt << endl;
    //pipe_control.decode_halt = 0;
  }
  else {
    pipeline_decode.halt = pipe_fetch.halt;
  }
  //printf("DECODE HALT: %d\n",pipeline_decode.halt);

  pipeline_decode.is_exit = pipe_fetch.is_exit;
  pipeline_decode.pc = pipe_fetch.pc;

  
  pipeline_decode.format = 0XFFFFFFFF;
  pipeline_decode.function = 0XFFFFFFFF;
  pipeline_decode.rs = 0XFFFFFFFF;
  pipeline_decode.rt = 0XFFFFFFFF;
  pipeline_decode.rd = 0XFFFFFFFF;
  pipeline_decode.imm = 0XFFFFFFFF;
  pipeline_decode.offset = 0XFFFFFFFF;
  pipeline_decode.base = 0XFFFFFFFF;
  pipeline_decode.sa = 0XFFFFFFFF;
  pipeline_decode.bltz = 0XFFFFFFFF;

  /**/

  if (!pipeline_decode.halt){/*halt == 0*/
    unsigned int instruction = pipe_fetch.instruction;
    opCode = instruction >> 26;
    if (instruction == 0){
      pipeline_decode.halt = 1;/*if there is a nop instruction, the next stage will halt*/
    }
    else{
      if (opCode == 0X00000000) decode_r_type(instruction);//R-Type Instruction
      else if (opCode == 0X00000003 || opCode == 0X00000002) decode_j_type(instruction);//J-Type instruction
      else if (opCode == 0X00000011) decode_c_type(instruction);//C-Type Instruction
      else decode_i_type(instruction);//I-Type Instruction
    }
  }
  return pipeline_decode;
}

void decode::decode_r_type(unsigned int instruction){
  pipeline_decode.iType = "r_type";
  pipeline_decode.opcode = instruction >> 26;
  pipeline_decode.format = 0;
  pipeline_decode.function = instruction & 0X0000003F;
  pipeline_decode.rs = (instruction >> 21) & 0X0000001F;
  pipeline_decode.rt = (instruction >> 16) & 0X0000001F;
  pipeline_decode.rd = (instruction >> 11) & 0X0000001F;
  //cout << pipeline_decode.rd << endl;
  pipeline_decode.imm = 0;
  pipeline_decode.offset = 0;//this is also immidiate
  pipeline_decode.base = (instruction >> 21) & 0X0000001F;
  pipeline_decode.sa = (instruction >> 6) & 0X0000001F;
  pipeline_decode.bltz = (instruction >> 16) & 0X0000001F;
  pipeline_decode.instr_index = instruction & 0X03FFFFFF;
}

void decode::decode_j_type(unsigned int instruction){
  pipeline_decode.iType = "j_type"; //1 is J-Type
  pipeline_decode.opcode = instruction >> 26;
  pipeline_decode.format = 0;
  pipeline_decode.function = 0;
  pipeline_decode.rs = 0;
  pipeline_decode.rt = 0;
  pipeline_decode.rd = 0;
  pipeline_decode.imm = 0;
  pipeline_decode.offset = instruction & 0X03FFFFFF;
  pipeline_decode.base = (instruction >> 21) & 0X0000001F;
  pipeline_decode.sa = (instruction >> 6) & 0X0000001F;
  pipeline_decode.bltz = (instruction >> 16) & 0X0000001F;
  pipeline_decode.instr_index = instruction & 0X03FFFFFF;
}

void decode::decode_c_type(unsigned int instruction){
  pipeline_decode.iType = "c_type"; //2 is C-Type
  pipeline_decode.opcode = instruction >> 26;
  pipeline_decode.format = (instruction >> 21) & 0X0000001F;
  pipeline_decode.function = instruction & 0X0000003F;
  pipeline_decode.rs = (instruction >> 11) & 0X0000001F;
  pipeline_decode.rt = (instruction >> 16) & 0X0000001F;
  pipeline_decode.rd = (instruction >> 6) & 0X0000001F;
  pipeline_decode.imm = 0;
  pipeline_decode.offset = instruction & 0X03FFFFFF;
  pipeline_decode.base = (instruction >> 21) & 0X0000001F;
  pipeline_decode.sa = (instruction >> 6) & 0X0000001F;
  pipeline_decode.bltz = (instruction >> 16) & 0X0000001F;
  pipeline_decode.instr_index = instruction & 0X03FFFFFF;
}

void decode::decode_i_type(unsigned int instruction){
  pipeline_decode.iType = "i_type"; //3 is I-Type
  pipeline_decode.opcode = instruction >> 26;
  pipeline_decode.format = 0;
  pipeline_decode.function = 0;
  pipeline_decode.rs = (instruction >> 21) & 0X0000001F;
  pipeline_decode.rt = (instruction >> 16) & 0X0000001F;
  pipeline_decode.rd = 0;
  pipeline_decode.imm = instruction & 0X0000FFFF;
  pipeline_decode.offset = instruction & 0X0000FFFF;
  pipeline_decode.base = (instruction >> 21) & 0X0000001F;
  pipeline_decode.sa = (instruction >> 6) & 0X0000001F;
  pipeline_decode.bltz = (instruction >> 16) & 0X0000001F;
  pipeline_decode.instr_index = instruction & 0X03FFFFFF;
}
