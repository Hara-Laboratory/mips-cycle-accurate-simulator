/* ----------------------------------------

* File Name : mem.cpp

* Created on : 07-05-2014

* Last Modified on : Wed 24 Sep 2014 04:37:01 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/

#include <iostream>
#include <string>
#include "pipeline.h"
#include <stdio.h>

using namespace std;


//unsigned char i_mem[MEMSIZE];
//unsigned char d_mem[MEMSIZE];


pipe_mem::pipe_mem() : halt(true), is_exit(false) {
}


mem::mem() : pipeline_mem() {//empty constructor
}

pipe_mem mem::mem_access(pipe_exec pipe_exec, dmem dmem){
  unsigned char *dmemp;
  pipeline_mem.effective_address = pipe_exec.effective_address;
  pipeline_mem.value_to_be_store = pipe_exec.value_to_be_store;
  pipeline_mem.halt = pipe_exec.halt;
  pipeline_mem.is_int_wb = pipe_exec.is_int_wb;
  pipeline_mem.is_float_wb = pipe_exec.is_float_wb;
  /*=====================================*/
  pipeline_mem.int_wb_address = pipe_exec.int_wb_address;
  pipeline_mem.float_wb_address = pipe_exec.float_wb_address;
  /*=====================================*/
  pipeline_mem.int_wb_value = pipe_exec.int_wb_value;
  pipeline_mem.float_wb_value = pipe_exec.float_wb_value;

  pipeline_mem.is_exit = pipe_exec.is_exit;

  unsigned int value = 0;
  if (!pipeline_mem.halt){
    dmemp = &i_mem[pipe_exec.effective_address];
    if (pipe_exec.mem_operation == "lb"){
      pipeline_mem.is_int_wb = 1;
      signed int temp1 = *(dmemp+0);
      pipeline_mem.int_wb_value = temp1;
    }
    else if (pipe_exec.mem_operation == "lh"){
      pipeline_mem.is_int_wb = 1;
      unsigned int temp1 = *(dmemp+0);
      unsigned int temp2 = *(dmemp+1);
      //unsigned int temp3 = *(dmemp+2);
      //unsigned int temp4 = *(dmemp+3);
      signed int temp = (temp1 << 8) | temp2;
      pipeline_mem.int_wb_value = temp;
    }
    else if (pipe_exec.mem_operation == "lw"){
      pipeline_mem.is_int_wb = 1;
      //printf("LW:: Address= %x ", pipe_exec.effective_address);
      unsigned int temp1 = *(dmemp+0);
      unsigned int temp2 = *(dmemp+1);
      unsigned int temp3 = *(dmemp+2);
      unsigned int temp4 = *(dmemp+3);
      signed int temp = (temp1 << 24) | (temp2 << 16) | (temp3 << 8) | temp4;
      //printf(" value=%x\n",temp);
      pipeline_mem.int_wb_value = temp;
    }
    else if (pipe_exec.mem_operation == "lbu"){
      pipeline_mem.is_int_wb = 1;
      unsigned int temp1 = *(dmemp+0);
      //unsigned int temp2 = *(dmemp+1);
      //unsigned int temp3 = *(dmemp+2);
      //unsigned int temp4 = *(dmemp+3);
      pipeline_mem.int_wb_value = temp1;
      //pipeline_mem.int_wb_value = (temp1 << 24) | (temp2 << 16) | (temp3 << 8 ) | temp4;
    }
    else if (pipe_exec.mem_operation == "lhu"){
      pipeline_mem.is_int_wb = 1;
      unsigned int temp1 = *(dmemp+0);
      unsigned int temp2 = *(dmemp+1);
      //unsigned int temp3 = *(dmemp+2);
      //unsigned int temp4 = *(dmemp+3);
      pipeline_mem.int_wb_value = (temp1 << 8) | temp2;
    }
    else if (pipe_exec.mem_operation == "lwc1"){
      pipeline_mem.is_int_wb = 1;
    }
    else if (pipe_exec.mem_operation == "sb"){
      value = pipe_exec.value_to_be_store; 
      *(dmemp+0) = (value);
      pipeline_mem.halt = 1;
    }
    else if (pipe_exec.mem_operation == "sh"){
      value = pipe_exec.value_to_be_store; 
      //printf("%d %d\n", (value>>8), (value>>0));
      *(dmemp+0) = (value >> 8);
      *(dmemp+1) = (value >> 0);
      pipeline_mem.halt = 1;
    }
    else if (pipe_exec.mem_operation == "sw"){
      value = pipe_exec.value_to_be_store; 
      //printf("SW:: Address= %x ", pipe_exec.effective_address);
      *(dmemp+0) = (value >> 24);
      *(dmemp+1) = (value >> 16);
      *(dmemp+2) = (value >> 8);
      *(dmemp+3) = (value);
      //printf(" value=%x\n",value);
      pipeline_mem.halt = 1;
    }
    else if (pipe_exec.mem_operation == "swc1"){
    }
  }
  if (pipeline_mem.is_int_wb || pipeline_mem.is_float_wb){
    pipeline_mem.halt = 0;
  }
  return pipeline_mem;
}

void mem::get_from_mem(){
}

void mem::write_to_mem(){
}
