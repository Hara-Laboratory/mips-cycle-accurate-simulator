/* ----------------------------------------

* File Name : mem_file.h

* Created on : 08-05-2014

* Last Modified on : Sat 14 Jun 2014 11:31:57 AM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/


#include<iostream>
#include<string>


using namespace std;


typedef struct memory_page{
  unsigned int address;
  bool is_int;
  unsigned int int_value;
  bool is_float;
  float float_value;
  memory_page* next;
  //unsigned int 
}*memory_page_ptr;


class dmem{
private:
  memory_page_ptr page_head;
  memory_page_ptr page_curr;
  memory_page_ptr page_temp;
public:
  dmem();
  void int_mem_write(unsigned int, unsigned int);
  void float_mem_write(unsigned int, float);
  unsigned int int_mem_read(unsigned int);
  float float_mem_read(unsigned int);
  void print_mem(void);
};
