/* ----------------------------------------

* File Name : reg_file.cpp

* Created on : 07-05-2014

* Last Modified on : Mon 22 Sep 2014 10:04:30 AM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/

#include <iostream>
#include <string>
#include "reg_file.h"
//#include "pipeline.h"

using namespace std;

//float float_reg_file[32];
//unsigned int int_reg_file[32];
//unsigned int HI;
//unsigned int LO;



reg::reg(){
  //initialize the register files with zeros;
#if 0
  for (int i = 0; i < 32; ++i)
    int_reg_file[i] = 0;
  for (int i = 0; i < 32; ++i)
    float_reg_file[i] = 0;
  HI = 0;
  LO = 0;
#endif
}


unsigned int reg::read_from_int_reg(unsigned int reg_address){
  unsigned int value;
  //cout << "Get value from " << reg_address << "address ";
  //value = int_reg_file[reg_address];
  //cout << "and value is " << value << endl;
  //for (int i = 0; i < 32; i++){
    //cout << int_reg_file[reg_address] << " ";
  //}
  //cout << endl;
  // get the value from the regsiter filereg_value = 
  return value;
}


void reg::write_to_int_reg(unsigned int reg_address, unsigned int value){
  //unsigned int value;
  //int_reg_file[reg_address] = value;
  //cout << value << " write back to " << reg_address << endl;
  //for (int i = 0; i < 32; i++){
    //cout << int_reg_file[i] << " ";
  //}
  //cout << endl;
  //write the value to the register file
}


float reg::read_from_float_reg(unsigned int reg_address){
  float value;
  // get the value from the floating point register
  //value = float_reg_file[reg_address];
  return value;
}


void reg::write_to_float_reg(unsigned int reg_address, float value){
  //write the value to the floating point register
  //float_reg_file[reg_address] = value;
}



void reg::write_to_sr(unsigned int value, string reg_name){
  if (reg_name == "HI"){
    HI = value;
  }
  else if (reg_name == "LO"){
    LO = value;
  }
}


unsigned int reg:: read_from_sr(string reg_name){
  unsigned int value;
  if (reg_name == "HI"){
    value = HI;
  }
  else if (reg_name == "LO"){
    value = LO;
  }
  return value;
}

#if 0
void reg::print_reg(){
  for (int i = 0; i < 32 ; i++){
    cout << int_reg_file[i] << "  ";
  }
  cout << endl;

  cout << "HI " << HI;
  cout << " LO " << LO << endl;
}
#endif
