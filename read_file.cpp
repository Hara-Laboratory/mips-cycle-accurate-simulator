/* ----------------------------------------

* File Name : read_file.cpp

* Created on : 08-05-2014

* Last Modified on : Thu 25 Sep 2014 08:33:18 AM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/
/**
 * @file
 * @brief Load program from given ELF executable file.
 * @author Tanvir Ahmed <tanvira@ieee.org>
 * @author SAKAMOTO Noriaki <noriakis@cad.ce.titech.ac.jp>
 * @date 2014-08-05
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <libelf.h>
#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#include "read_file.h"
#include "pipeline.h"
#include <math.h>
#define forcenonnull(ptr,...) errorif((ptr)==NULL,__VA_ARGS__)
#define errorif(...) errorif2(__VA_ARGS__,0)
#define errorif2(test,fmt,...) do{if(test){fprintf(stderr,"%s:%s:%d: "fmt"\n",__FILE__,__func__,__LINE__,__VA_ARGS__);exit(EXIT_FAILURE);}}while(0)

/**
 * Read data of given section from ELF file.
 *
 * @param elf file
 * @param index of section to read
 * @return pointer to malloced data
 */
static char *read_section(Elf *elf, size_t index) {
    Elf_Scn *scn = elf_getscn(elf, index);
    forcenonnull(scn,"elf_getscn() failed\n");

    Elf32_Shdr *shdr = elf32_getshdr(scn);
    forcenonnull(shdr,"elf32_getshdr() failed\n");

    char *dst = (char *)malloc(shdr->sh_size);
    forcenonnull(dst,"malloc() failed\n");

    Elf_Data *data = NULL;
    size_t n = 0;

    while (n < shdr->sh_size &&
	  (data = elf_getdata(scn, data)) != NULL) {
      memcpy(dst + n, data->d_buf, data->d_size);
      n += data->d_size;
    }

    return dst;
}

/**
 * Read entry point from ELF file.
 *
 * @param elf file
 * @return entry point
 */
static size_t read_entry(Elf *elf) {
  Elf32_Ehdr *ehdr = elf32_getehdr(elf);
  forcenonnull(elf, "elf32_getehdr() failed");

  return ehdr->e_entry;
}

/**
 * Write given int value to the memory.
 *
 * @param pos relative address from HDRADDR
 * @param x   value
 */
static void write_imem(unsigned int pos, unsigned int x) {
#if !defined(CONFIG_SEPARATE_I_MEM)
  unsigned char *memp = &i_mem[HDRADDR+pos];
  * memp    = ((x)>>24)&0xff;
  *(memp+1) = ((x)>>16)&0xff;
  *(memp+2) = ((x)>> 8)&0xff;
  *(memp+3) = ((x)    )&0xff;
#else
  /* I think you can modify like as follows */
  /* I assume there is "char i_mem0[MEMSIZE], i_mem1[MEMSIZE], i_mem2[MEMSIZE], i_mem3[MEMSIZE];" */
  /* If you use dmem::int_mem_write and dmem::int_mem_read over whole the project,
   * this kind of modification is only applied for dmem::* methods. */
  i_mem0[HDRADDR+pos  ] = ((x)>>24)&0xff;
  i_mem1[HDRADDR+pos+1] = ((x)>>16)&0xff;
  i_mem2[HDRADDR+pos+2] = ((x)>> 8)&0xff;
  i_mem3[HDRADDR+pos+3] = ((x)    )&0xff;
#endif
}

/**
 * Write given buffer to the memory.
 *
 * @param pos relative address from HDRADDR
 * @param x   value
 */
static void write_imem(size_t pos, char *buf, int len) {
#if !defined(CONFIG_SEPARATE_I_MEM)
  memcpy(&i_mem[pos], buf, len);
#else
  /* I think you can modify like as follows */
  /* I assume there is "char i_mem0[MEMSIZE], i_mem1[MEMSIZE], i_mem2[MEMSIZE], i_mem3[MEMSIZE];" */
  /* If you use dmem::int_mem_write (this method's behavior should be equivalent to write_imem(unsigned int, unsigned int)) and dmem::int_mem_read over whole the project,
   * this kind of modification is only applied for dmem::* methods. */
  for (size_t i = 0; i < len; ++i) {
    char i_mem;
	switch (i % 4) {
      case 0: i_mem = imem0; break;
      case 1: i_mem = imem1; break;
      case 2: i_mem = imem2; break;
      case 3: i_mem = imem3; break;
	}
	imem[(pos+i)>>2] = buf[i];
  }
#endif
}

/* READ_ELF() */
unsigned int read_elf(const char *file) {
  /* FILEOPEN */
  FILE *fp;
  int i;

  Elf_Data *data;
  GElf_Sym dst;

  unsigned char *memp;
 
  fp = fopen(file, "r");
  if (fp == NULL){
    printf("Can't open elf_file\n");
    exit(1);
  }

  int fd = fileno(fp);

  elf_version(EV_CURRENT);
  Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
  forcenonnull(elf, "elf_begin() failed");
  
  /* READ ELF_HEADER */
  size_t shnum;
  if (elf_getshdrnum (elf, &shnum) != 0) {
    fprintf(stderr, "elf_getshdrnum() failed");
    exit(EXIT_FAILURE);
  }

  Elf32_Shdr	*Shead = (Elf32_Shdr *)malloc(shnum * sizeof(Elf32_Shdr));
  size_t symtabndx;
  for (size_t i = 0;  i < shnum; ++i) {
    Elf_Scn *scn = elf_getscn(elf, i);
    Elf32_Shdr *shdr = elf32_getshdr(scn);
    Shead[i] = *shdr;
    if (shdr->sh_type == SHT_SYMTAB)
      symtabndx = i;
  }
  
  size_t shstrndx;
  if (elf_getshdrstrndx (elf , &shstrndx) != 0) {
    fprintf(stderr, " elf_getshdrstrndx () failed. ");
    exit(EXIT_FAILURE);
  }

  bool found_text = false, found_data = false, found_bss = false, found_rodata = false;
  size_t textndx, datandx, bssndx, strtabndx, rodatandx;
  for (size_t i = 0; i < shnum; ++i) {
    char *name;
    if ((name = elf_strptr(elf, shstrndx, Shead[i].sh_name)) == NULL) {
      fprintf(stderr, "elf_strptr() failed.\n");
      exit(EXIT_FAILURE);
    }
    //printf("section %s: addr=%x offset=%x link=%x\n", name, Shead[i].sh_addr, Shead[i].sh_offset, Shead[i].sh_link);

    if      (!strcmp(name, ".text")) {
      found_text = true;
      textndx = i;
    } 
    else if (!strcmp(name, ".data")) {
      found_data = true;
      datandx = i;
    } 
    else if (!strcmp(name, ".bss")) {
      found_bss = true;
      bssndx = i;
//    else if (!strcmp(name, ".symtab"))
//      symtabndx = i;
    } 
    else if (!strcmp(name, ".rodata")) {
      found_rodata = true;
      rodatandx = i;
    }
    else if (!strcmp(name, ".strtab")) {
      strtabndx = i;
    }
  }

  errorif(!found_text, "not found .text section");

  //printf("textndx: %d\n",textndx);
  //printf("datandx: %d\n",datandx);
  printf(GREEN" ELF:text=%8.8x "RESET, Shead[textndx].sh_size);
  printf(GREEN"data=%8.8x "RESET, Shead[datandx].sh_size);
  if (found_bss)
    printf(GREEN"bss=%8.8x "RESET, Shead[bssndx].sh_size);
  if (found_rodata)
    printf(GREEN"rodata=%8.8x "RESET, Shead[rodatandx].sh_size);
  printf(GREEN"symt=%8.8x "RESET, Shead[symtabndx].sh_size);
  printf(GREEN"strt=%8.8x\n"RESET, Shead[strtabndx].sh_size-1);
  
  /* READ PROGRAM_HEADER */
  size_t phdrnum;
  if (elf_getphdrnum(elf, &phdrnum) != 0) {
    fprintf(stderr, "elf_getphdrnum failed.\n");
    exit(EXIT_FAILURE);
  }

  if (phdrnum == 0) {
    fprintf(stderr, "the file has no program headers.\n");
    exit(EXIT_FAILURE);
  }

  GElf_Phdr *Phead = (GElf_Phdr *)malloc(phdrnum * sizeof(GElf_Phdr));
  forcenonnull(Phead, "malloc of Phead failed.");

  for (size_t i = 0; i < phdrnum; i++) {
    if (gelf_getphdr(elf, i, &Phead[i]) != &Phead[i]) {
      fprintf(stderr, "gelf_getphdr() failed");
      exit(EXIT_FAILURE);
    }
    //printf(YELLOW"program %ld size=%8.8lx, offset=%8.8lx\n"RESET, i, Phead[i].p_filesz, Phead[i].p_offset);
  }

  
  /* READ (text, data) */
  char *Text_p = NULL;
  if (found_text) {
    Text_p = read_section(elf, textndx);
  }


  char *rodata_p = NULL;
  if (found_rodata) {
    rodata_p = read_section(elf, rodatandx);
  }

  char *Data_p = NULL;
  if (found_data) {
    Data_p = read_section(elf, datandx);
  }
  
//  char *bss_p;
/*  if (found_bss) {
    bss_p = read_section(elf, bssndx);
  }*/
/*  printf("WORKING\n");*/


/* CONV_DATA() */
  
  //printf("text sh_size: %x, phdr.p_memsz: %lx\n", Shead[textndx].sh_size, Phead[0].p_memsz);
  /* ELF_HEADER --> A.OUT_HEADER */
  unsigned int a_text   = Shead[textndx].sh_size;
  unsigned int a_rodata = found_rodata ? Shead[rodatandx].sh_size : 0;
  unsigned int a_data   = found_data ? Shead[datandx].sh_size : 0;
  unsigned int a_bss    = found_bss ? Shead[bssndx].sh_size : 0;
  unsigned int a_entry  = Phead[0].p_vaddr;
  Elf32_Addr e_entry = read_entry(elf);
  
  printf(BLUE" BSD:text=%8.8x ", a_text);
  printf("data=%8.8x ", a_data);
  printf("bss=%8.8x", a_bss);
  printf("rodata=%8.8x", a_rodata);
  printf("symt=-------- ");
  printf("strt=%8.8x\n", Shead[strtabndx].sh_size-1);
  printf(" e_entry=%8.8x, a_entry=%8.8x\n"RESET, e_entry, a_entry);

/* LOAD_SECTIONS() */
  unsigned int ptrheap = 0;
  int ndxs[] = {textndx, rodatandx, datandx, bssndx};
  int existsHdr[] = {found_text, found_rodata, found_data, found_bss};
  for(int i = 0; i < sizeof(ndxs) / sizeof(int); ++i) {
    if(existsHdr[i]) {
      ptrheap = max(ptrheap, Shead[ndxs[i]].sh_addr + Shead[ndxs[i]].sh_size);
    }
  }

  if (ptrheap > ALOCLIMIT) {
    printf("Program too large\n");
    exit(EXIT_FAILURE);
  }

  write_imem(0, ptrheap); // initial malloc pointer
  write_imem(4, ALOCLIMIT);					// malloc limit
  write_imem(8, MEMSIZE-0x80);					// initial stack pointer

  write_imem(Shead[textndx].sh_addr, Text_p, a_text);
  if (found_rodata)
    write_imem(Shead[rodatandx].sh_addr, rodata_p, a_rodata);
  if (found_data)
    write_imem(Shead[datandx].sh_addr, Data_p, a_data);
/*  if (found_bss)
    write_imem(Shead[bssndx].sh_addr, bss_p, a_bss);*/

  free(Shead);
  free(Phead);
  free(Text_p);
  free(Data_p);
//  free(bss_p);
  free(rodata_p);
  elf_end(elf);
  fclose(fp);
  return (e_entry);
}
// vim: ts=8 sw=2 :
