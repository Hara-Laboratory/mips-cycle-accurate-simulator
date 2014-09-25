/* ----------------------------------------

* File Name : reg_file.h

* Created on : 07-05-2014

* Last Modified on : Fri 20 Jun 2014 10:44:19 AM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/

#include<iostream>
#include<string>

using namespace std;


struct registers{
  unsigned int int_reg[32];
  float float_reg[32];
  unsigned int HI;
  unsigned int LO;
};


extern float float_reg_file[32];
extern unsigned int int_reg_file[32];
extern unsigned int HI;
extern unsigned int LO;

//registers register_file;


//registers reg_file;

class reg{
private:
//  float float_reg_file[32];
//  unsigned int int_reg_file[32];
//  unsigned int HI;
//  unsigned int LO;
public:
  reg();
  unsigned int read_from_int_reg(unsigned int);
  void write_to_int_reg(unsigned int, unsigned int);
  float read_from_float_reg(unsigned int);
  void write_to_float_reg(unsigned int, float);
  void write_to_sr(unsigned int, string);
  unsigned int read_from_sr(string);

  void print_reg();
};
