/* ----------------------------------------

* File Name : pipeline.h

* Created on : 08-05-2014

* Last Modified on : Thu 25 Sep 2014 02:26:28 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/
#include <iostream>
#include <string>
#include "reg_file.h"
#include "mem_file.h"
#include "read_file.h"

//#define	DEBUG


using namespace std;

#define	RED	"\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"


/*==========================================*
 * 					    *
 * 					    *
 * 	processor input arguments	    *
 * 					    *
 *==========================================*/
#define	TRACE_PIPE	0X00000001
#define TRACE_FETCH	0x00000010
#define	TRACE_DECODE	0x00000100
#define	TRACE_EXEC	0X00001000
#define	TRACE_MA	0X00010000
#define	TRACE_WB	0X00100000
#define	USE_BRANCH_PRED	0X01000000
#define	USE_CACHE_SIM	0X10000000
#define USE_TRACE_RANGE 0x00000002

/*============================================*
 * 					      *
 * 	Processor status definenation         *
 * 					      *
 *============================================*/
#define STATUS_START	0X00000000
#define STATUS_NORMAL	0X00000001
#define	STATUS_ERROR	0X00000010
#define STATUS_EXIT	0X00001000



/*============================================*
 * 					      *
 * 	file reading information	      *
 * 					      *
 *============================================*/

//unsigned char imem[MEMSIZE]
//
extern unsigned char i_mem[MEMSIZE];
//extern unsigned char d_mem[MEMSIZE];
extern float float_reg_file[32];
extern unsigned int int_reg_file[32];
extern unsigned int HI;
extern unsigned int LO;

/*============================================*
 * 					      *
 * 		pipe control       	      *
 * 					      *
 *============================================*/

struct pipe_control{
  unsigned int npc;
  bool decode_halt;
  bool exec_halt;
};

/*============================================*
 * 					      *
 * 	start of instruction fetch stage      *
 * 					      *
 *============================================*/
struct pipe_fetch{
  unsigned int pc;	/*pc is used to get the value from the i_mem. it is updated by npc at every cycle*/
  unsigned int npc;	/*npc is the next pc, npc = npc + 4, npc is updated in the pipecontrol npc*/
  unsigned int instruction;	/*variables for the instructions from the memory*/
  bool halt;		/*holds the pipeline.*/
  bool is_exit;		/*when the exit condition meet. it will be propagating  towards the wb stage*/

  pipe_fetch();
};
//
//
class fetch {
private:
  pipe_fetch pipeline_fetch;
public:
  fetch(); //constructor;
  //pipe_fetch initialize(pipe_fetch);
  pipe_fetch fetch_instruction(pipe_control);
  void get_from_i_mem(unsigned int);
};
#if 0
struct pipe_fetch{
  unsigned int pc;	/*pc is used to get the value from the i_mem. it is updated by npc at every cycle*/
  unsigned int npc;	/*npc is the next pc, npc = npc + 4, npc is updated in the pipecontrol npc*/
  unsigned int instruction;	/*variables for the instructions from the memory*/
  bool halt;		/*holds the pipeline.*/
  bool is_exit;		/*when the exit condition meet. it will be propagating  towards the wb stage*/
};
//
//
class fetch {
private:
  pipe_fetch pipeline_fetch;
public:
  fetch(); //constructor;
  //pipe_fetch initialize(pipe_fetch);
  pipe_fetch fetch_instruction(pipe_control);
  void get_from_i_mem(unsigned int);
};
#endif
/*==============================================*
 * 	end of instruction fetch stage		*
 *==============================================*/

/*========================================================*
 *							  *
 *                  start of decode stage		  *
 * 							  *
 * =======================================================*/
struct pipe_decode{
  bool is_exit;		/*if exit condition*/
  bool halt;		/*when the processor need to halt*/
  string iType;		/*instruction type*/
  unsigned int opcode;	/**/
  unsigned int format;	/**/
  unsigned int function;	/**/
  unsigned int rs;		/*register number*/
  unsigned int rt;		/*register number*/
  unsigned int rd;		/*register number*/
  unsigned int imm;		/*immidiate value*/
  unsigned int offset;		/*offset*/
  unsigned int base;		/*base address*/
  unsigned int sa;		/*sa in the instruction*/
  unsigned int bltz;		/*bltz/bgez in the instruction*/
  unsigned int instr_index;	/*for jump and jal instruction*/
  unsigned int pc;
  
  pipe_decode();	
};

class decode{
private:
  pipe_decode pipeline_decode;
public:

  decode();//constructor
  pipe_decode decode_instruction(pipe_fetch, pipe_control);//checking the type of instructions

  void decode_r_type(unsigned int); //assign the value of the instruction on the structure
  void decode_j_type(unsigned int); //assign the value of the instruction on the structure
  void decode_c_type(unsigned int); //assign the value of the instruction on the structure
  void decode_i_type(unsigned int); //assign the value of the instruction on the structure

/*void decodeMain();//check the initial condition
  void decodeArith();//decode the arithmetic operations
  void decodeLogic();//decode the logic operations
  void decodeMemAccess();//decode the memory access operations
  void decodeBranch();//decode the branch instructions*/
};

/*==============================================*
 * 	end of decode stage			*
 *==============================================*/



/*============================================*
 * 					      *
 * 	start of execution stage	      *
 * 					      *
 *============================================*/
struct pipe_exec{
  bool halt;
  bool is_exit;
  unsigned int pc;
  //bool is_nope;		//if the instruction is nop it would be 1 or else 0;
  /*these are for the int write back operations such as, add r1 --> r2 + r3;*/
  bool is_int_wb;
  unsigned int rs_v;
  unsigned int rd_v;
  unsigned int rt_v;
  unsigned int int_wb_value;
  unsigned int sa;
  unsigned int HI;
  unsigned int LO;
  unsigned int offset_value;
  unsigned int int_wb_address;
  /*these are for the jump condition such as, jump L1*/
  bool is_jump;
  unsigned int jump_address;
  /*These are for the floating point */
  bool is_float_wb;
  float rs_fv;
  float rd_fv;
  float rt_fv;
  float float_wb_value;
  unsigned int float_wb_address;
  /*This are for the memory operations*/
  bool is_mem;
  string mem_operation;
  unsigned int effective_address;
  /*first load operations*/
  //bool is_ld;
  /*second store operations*/
  //bool is_st;
  unsigned int value_to_be_store;
  /*need to propagate from the previous stage*/
  unsigned int rs;
  unsigned int rt;
  unsigned int rd;


  /*save the value for data forwarding*/
  bool forwarding_required_1;
  bool forwarding_required_2;
  unsigned int forwarding_value;
  unsigned int forwarding_reg;
  unsigned int forwarding_value_2;
  unsigned int forwarding_reg_2;

  pipe_exec();
};

class exec{
private:
  pipe_exec pipeline_exec;
public:
  exec();//constructor
  pipe_exec exec_instruction(pipe_decode, pipe_control, reg/*registers*/);//will check the conditions and execute the exact group of instructions
  void exec_r_type(pipe_decode, reg);//execute the arithmetic operations
  void exec_j_type(pipe_decode, reg);//execture the logic operations
  void exec_c_type(pipe_decode, reg);//execute the memory access operations;
  void exec_i_type(pipe_decode, reg);//execute the branch operations
};

/*==============================================*
 * 	end of execution stage			*
 *==============================================*/




/*============================================*
 * 					      *
 * 	start of memory access stage	      *
 * 					      *
 *============================================*/

struct pipe_mem{
  bool halt;
  bool is_exit;
  bool is_int_wb;
  bool is_float_wb;

  unsigned int value_from_mem;
  unsigned int effective_address;
  unsigned int value_to_be_store;

  /*unsigned int rs;
  unsigned int rt;
  unsigned int rd;*/
  unsigned int int_wb_address;
  unsigned int float_wb_address;
  unsigned int int_wb_value;
  float float_wb_value;

  pipe_mem();

};

class mem{
private:
  pipe_mem pipeline_mem;
public:
  mem();
  pipe_mem mem_access(pipe_exec, dmem);
  void get_from_mem();
  void write_to_mem();
};


// the structure and the class will be defined here

/*==============================================*
 * 	end of memory access stage		*
 *==============================================*/

/*============================================*
 * 					      *
 * 	start of write back stage	      *
 * 					      *
 *============================================*/

struct pipe_wb{
  bool halt;
  bool is_exit;
  //add the member of the struct;
  unsigned int int_wb_address;
  unsigned int float_wb_address;
  unsigned int int_wb_value;
  float float_wb_value;

  pipe_wb();
};

class wb{
private:
  pipe_wb pipeline_wb;
public:
  wb();//constructor
  pipe_wb write_back(pipe_mem, reg);
};

// the structure  and the class will be defined here.

/*==============================================*
 * 	end of write back stage			*
 *==============================================*/
