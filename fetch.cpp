/* ----------------------------------------

* File Name : fetch.cpp

* Created on : 08-05-2014

* Last Modified on : Wed 24 Sep 2014 04:15:38 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/


#include <iostream>
#include <string>
#include "pipeline.h"
#include <stdio.h>
//#include "fetch.h"



using namespace std;

//unsigned char i_mem[MEMSIZE];

//implementation of the fetch class
//
pipe_fetch::pipe_fetch() : pc(0), halt(true), is_exit(false) {/*constructor*/
}

fetch::fetch() : pipeline_fetch() {/*constructor*/
}

pipe_fetch fetch::fetch_instruction(pipe_control pipe_control){
  /*instruction memory declaration*/
  unsigned char *memp;

  /*update the pc from the pipe_control register*/
  pipeline_fetch.npc = pipe_control.npc;
  pipeline_fetch.pc = pipeline_fetch.npc;

  /*initializing the memory*/
  memp = &i_mem[pipeline_fetch.pc];
  pipeline_fetch.instruction = *(memp+0) << 24 | *(memp+1) << 16 | *(memp+2) << 8 | *(memp+3);
  pipeline_fetch.npc = pipeline_fetch.npc + 4;
  pipeline_fetch.halt = 0;
  //}

  return pipeline_fetch;
}
