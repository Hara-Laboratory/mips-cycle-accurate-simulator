/* ----------------------------------------

* File Name : mem.cpp

* Created on : 07-05-2014

* Last Modified on : Mon 23 Jun 2014 11:04:21 AM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/

#include <iostream>
#include <string>
#include "pipeline.h"
#include <stdio.h>

using namespace std;

mem::mem(){//empty constructor
  pipeline_mem.halt = 1;
}

pipe_mem mem::mem_access(pipe_exec pipe_exec, dmem dmem){

  unsigned char *dmemp;
  
  pipeline_mem.halt = pipe_exec.halt;
  pipeline_mem.is_int_wb = pipe_exec.is_int_wb;
  pipeline_mem.is_float_wb = pipe_exec.is_float_wb;

  /*=====================================*/
  pipeline_mem.int_wb_address = pipe_exec.int_wb_address;
  pipeline_mem.float_wb_address = pipe_exec.float_wb_address;
  /*=====================================*/
  pipeline_mem.int_wb_value = pipe_exec.int_wb_value;
  pipeline_mem.float_wb_value = pipe_exec.float_wb_value;

  unsigned int value;
  if (!pipeline_mem.halt){
    //pipeline_mem.rs = pipe_exec.rs;
    //pipeline_mem.rt = pipe_exec.rt;
    //pipeline_mem.rd = pipe_exec.rd;
    dmemp = &d_mem[pipe_exec.effective_address];
    if (pipe_exec.mem_operation == "lb"){
      //cout << "LB" << pipe_exec.effective_address << endl;
      pipeline_mem.is_int_wb = 1;
      unsigned int temp1 = *(dmemp+0);
      unsigned int temp2 = *(dmemp+1);
      unsigned int temp3 = *(dmemp+2);
      unsigned int temp4 = *(dmemp+3);
      //cout << pipe_exec.effective_address << endl;
      //temp4 = dmem.int_mem_read(pipe_exec.effective_address+0);
      pipeline_mem.int_wb_value = ((temp1 << 24) & 0xFF000000) | ((temp2 << 16) & 0x00FF0000) | ((temp3 << 8 )& 0x0000FF00) | (temp4 & 0x000000FF);
      //cout <<"LB\n";
    }
    else if (pipe_exec.mem_operation == "lh"){
      //cout << "LH" << pipe_exec.effective_address << endl;
      //cout <<"LH\n";
      pipeline_mem.is_int_wb = 1;
      unsigned int temp1 = *(dmemp+0);
      unsigned int temp2 = *(dmemp+1);
      unsigned int temp3 = *(dmemp+2);
      unsigned int temp4 = *(dmemp+3);
      //temp3 = dmem.int_mem_read(pipe_exec.effective_address+0);
      //temp4 = dmem.int_mem_read(pipe_exec.effective_address+1);
      pipeline_mem.int_wb_value = ((temp1 << 24) & 0xFF000000) | ((temp2 << 16) & 0x00FF0000) | ((temp3 << 8 )& 0x0000FF00) | (temp4 & 0x000000FF);
      
    }
    else if (pipe_exec.mem_operation == "lw"){
      //cout << "LW" << pipe_exec.effective_address << endl;
      //cout <<"LW\n";
      pipeline_mem.is_int_wb = 1;
      unsigned int temp1 = *(dmemp+0);
      unsigned int temp2 = *(dmemp+1);
      unsigned int temp3 = *(dmemp+2);
      unsigned int temp4 = *(dmemp+3);
      //temp1 = dmem.int_mem_read(pipe_exec.effective_address+0);
      //temp2 = dmem.int_mem_read(pipe_exec.effective_address+1);
      //temp3 = dmem.int_mem_read(pipe_exec.effective_address+3);
      //temp4 = dmem.int_mem_read(pipe_exec.effective_address+4);
      pipeline_mem.int_wb_value = ((temp1 << 24) & 0xFF000000) | ((temp2 << 16) & 0x00FF0000) | ((temp3 << 8 )& 0x0000FF00) | (temp4 & 0x000000FF);
    }
    else if (pipe_exec.mem_operation == "lbu"){
      //cout << "LBU" << pipe_exec.effective_address << endl;
      //cout <<"LBU\n";
      pipeline_mem.is_int_wb = 1;
      unsigned int temp1 = *(dmemp+0);
      unsigned int temp2 = *(dmemp+1);
      unsigned int temp3 = *(dmemp+2);
      unsigned int temp4 = *(dmemp+3);
      //temp1 = dmem.int_mem_read(pipe_exec.effective_address+0);
      //temp4 = dmem.int_mem_read(pipe_exec.effective_address+0);
      pipeline_mem.int_wb_value = ((temp1 << 24) & 0xFF000000) | ((temp2 << 16) & 0x00FF0000) | ((temp3 << 8 )& 0x0000FF00) | (temp4 & 0x000000FF);
    }
    else if (pipe_exec.mem_operation == "lhu"){
      //cout << "LHU" << pipe_exec.effective_address << endl;
      //cout <<"LHU\n";
      pipeline_mem.is_int_wb = 1;
      unsigned int temp1 = *(dmemp+0);
      unsigned int temp2 = *(dmemp+1);
      unsigned int temp3 = *(dmemp+2);
      unsigned int temp4 = *(dmemp+3);
      //temp3 = dmem.int_mem_read(pipe_exec.effective_address+0);
      //temp4 = dmem.int_mem_read(pipe_exec.effective_address+1);
      pipeline_mem.int_wb_value = ((temp1 << 24) & 0xFF000000) | ((temp2 << 16) & 0x00FF0000) | ((temp3 << 8 )& 0x0000FF00) | (temp4 & 0x000000FF);
    }
    else if (pipe_exec.mem_operation == "lwc1"){
      //cout <<"LWC1\n";
      pipeline_mem.is_int_wb = 1;
    }
    else if (pipe_exec.mem_operation == "sb"){
      //cout << "SB" << pipe_exec.effective_address << endl;
      //cout <<"SB\n";
      value = pipe_exec.value_to_be_store; 
      *(dmemp+0) = value & 0x000000FF;
      //dmem.int_mem_write((pipe_exec.effective_address + 0),(value & 0x000000FF));
      pipeline_mem.halt = 1;
    }
    else if (pipe_exec.mem_operation == "sh"){
      //cout << "SH" << pipe_exec.effective_address << endl;
      //cout <<"SH\n";
      value = pipe_exec.value_to_be_store; 
      *(dmemp + 0) = value & 0x000000FF;
      *(dmemp + 1) = (value & 0x0000FF00) >> 8;
      //dmem.int_mem_write((pipe_exec.effective_address + 0),(value & 0x000000FF));
      //dmem.int_mem_write((pipe_exec.effective_address + 1),((value & 0x0000FF00) >> 8));
      pipeline_mem.halt = 1;
    }
    else if (pipe_exec.mem_operation == "sw"){
      //cout <<"SW\n";
      //cout << "SW" << pipe_exec.effective_address << endl;
      value = pipe_exec.value_to_be_store; 
      *(dmemp + 0) = value & 0x000000FF;
      *(dmemp + 1) = (value & 0x0000FF00) >> 8;
      *(dmemp + 2) = (value & 0x00FF0000) >> 16;
      *(dmemp + 3) = (value & 0xFF000000) >> 24;
      
      //dmem.int_mem_write((pipe_exec.effective_address + 0),(value & 0x000000FF));
      //dmem.int_mem_write((pipe_exec.effective_address + 1),((value & 0x0000FF00) >> 8));
      //dmem.int_mem_write((pipe_exec.effective_address + 2),((value & 0x00FF0000) >> 16));
      //dmem.int_mem_write((pipe_exec.effective_address + 3),((value & 0xFF000000) >> 24));
      pipeline_mem.halt = 1;
    }
    else if (pipe_exec.mem_operation == "swc1"){
      //cout <<"SWC1\n";
    }
  }
  //printf(" ==MemAcc%d\n",pipeline_mem.halt);
  if (pipeline_mem.is_int_wb || pipeline_mem.is_float_wb){
    pipeline_mem.halt = 0;
  }
  return pipeline_mem;
}

void mem::get_from_mem(){
}

void mem::write_to_mem(){
}
