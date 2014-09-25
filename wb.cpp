/* ----------------------------------------

* File Name : pipeWriteBack.cpp

* Created on : 07-05-2014

* Last Modified on : Wed 24 Sep 2014 04:29:15 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/

#include <iostream>
#include <string>
#include <stdio.h>
#include "pipeline.h"

using namespace std;


pipe_wb::pipe_wb() : is_exit(false){
}



wb::wb() : pipeline_wb() {
}

pipe_wb wb::write_back(pipe_mem pipe_mem, reg reg){
  unsigned int *wb_address;

  pipeline_wb.halt = pipe_mem.halt;
  //cout << pipeline_wb.is_exit << endl;
  pipeline_wb.is_exit = pipe_mem.is_exit;
  pipeline_wb.int_wb_address = pipe_mem.int_wb_address;
  pipeline_wb.int_wb_value = pipe_mem.int_wb_value;
  if (!pipeline_wb.halt){/*halt = 0*/
    //if (pipe_mem.is_wb){/*write the value to the reg file*/
      //reg.write_to_int_reg()
    //}
    if (pipe_mem.is_int_wb || pipe_mem.is_float_wb){
      wb_address = &int_reg_file[pipe_mem.int_wb_address];
      //printf("%d %x %d --> %d ",pipe_mem.int_wb_address, wb_address, *wb_address, pipe_mem.int_wb_value);
      
      *wb_address = pipe_mem.int_wb_value;
      //printf("%d %d\n",pipe_mem.int_wb_value, *wb_address);
      //printf("\n");
      //reg.write_to_int_reg(pipe_mem.int_wb_address, pipe_mem.int_wb_value);
    }
  }
  return pipeline_wb;
}



