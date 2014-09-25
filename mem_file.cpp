/* ----------------------------------------

* File Name : mem_file.cpp

* Created on : 08-05-2014

* Last Modified on : Sat 14 Jun 2014 12:01:34 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/


#include<iostream>
#include<string>
#include"mem_file.h"

using namespace std;

dmem::dmem(){//constructor
  page_head = NULL;
  page_curr = NULL;
  page_temp = NULL;
}

void dmem::int_mem_write(unsigned int address, unsigned int value){
/*
  memory_page_ptr page_new = new memory_page;
  page_new->address = address;
  page_new->is_int = 1;
  page_new->int_value = value;
  page_new->is_float = 0;
  page_new->float_value = 0;
  page_new->next = NULL;*/
  bool is_overwrite = 0;
  //cout << "WRITE: address: " <<  address << " value: " << value << endl;
  if (page_head != NULL){
    page_curr = page_head;
    while (page_curr->next != NULL){
      if (page_curr->address == address){
        page_curr->int_value = value;
        is_overwrite = 1;
      }
      page_curr = page_curr->next;
    }
    if (is_overwrite){
      memory_page_ptr page_new = new memory_page;
      page_new->address = address;
      page_new->is_int = 1;
      page_new->int_value = value;
      page_new->is_float = 0;
      page_new->float_value = 0.0;
      page_new->next = NULL;
      page_curr->next = page_new;
    }
  }
  else {
    memory_page_ptr page_new = new memory_page;
    page_new->address = address;
    page_new->is_int = 1;
    page_new->int_value = value;
    page_new->is_float = 0;
    page_new->float_value = 0.0;
    page_new->next = NULL;
    page_head = page_new;
  }

}

void dmem::float_mem_write(unsigned int address, float value){
  bool is_overwrite = 0;
  if (page_head != NULL){
    page_curr = page_head;
    while (page_curr->next != NULL){
      if (page_curr->address == address){
        page_curr->int_value = value;
        is_overwrite = 1;
      }
      page_curr = page_curr->next;
    }
    if (is_overwrite){
      memory_page_ptr page_new = new memory_page;
      page_new->address = address;
      page_new->is_int = 0;
      page_new->int_value = 0;
      page_new->is_float = 1;
      page_new->float_value = value;
      page_new->next = NULL;
      page_curr->next = page_new;
    }
  }
  else {
    memory_page_ptr page_new = new memory_page;
    page_new->address = address;
    page_new->is_int = 0;
    page_new->int_value = 0;
    page_new->is_float = 1;
    page_new->float_value = value;
    page_new->next = NULL;
    page_head = page_new;
  }
#if 0
  memory_page_ptr page_new = new memory_page;
  page_new->address = address;
  page_new->is_int = 0;
  page_new->int_value = 0;
  page_new->is_float = 1;
  page_new->float_value = value;
  page_new->next = NULL;
  if (page_head != NULL){
    page_curr = page_head;
    while (page_curr->next != NULL){
      page_curr = page_curr->next;
    }
    page_curr->next = page_new;
  }
  else {
    page_head = page_new;
  }
#endif
}

unsigned int dmem::int_mem_read(unsigned int address){
  //cout << "READ: address: " <<  address << endl;
  unsigned int value;
  bool is_page_fault = 1;
  page_curr = page_head;
  while (page_curr != NULL){
    if (page_curr->is_int == 1 && page_curr->address == address){
      value = page_curr->int_value;
      is_page_fault = 0;
    }
    page_curr = page_curr->next;
  }
  if (is_page_fault){
    cout << "====== Error:Page Fault ======" << endl;
  }
  return value;
}

float dmem::float_mem_read(unsigned int address){
  float value;
  bool is_page_fault = 1;
  page_curr = page_head;
  while (page_curr != NULL){
    if (page_curr->is_int == 1 && page_curr->address == address){
      value = page_curr->int_value;
      is_page_fault = 0;
    }
    page_curr = page_curr->next;
  }
  if (is_page_fault){
    cout << "====== Error:Page Fault ======" << endl;
  }
  return value;
}


void dmem::print_mem(){
  cout << "TEST_OK\n";
  page_curr = page_head;
  //cout << page_curr->address << endl;
  while (page_curr != NULL){
    cout << page_curr->address << endl;
    page_curr = page_curr->next;
  }
}




