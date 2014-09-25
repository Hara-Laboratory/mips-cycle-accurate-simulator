/* ----------------------------------------

* File Name : read_file.cpp

* Created on : 08-05-2014

* Last Modified on : Mon 01 Sep 2014 12:13:45 PM JST

* Primary Author : Tanvir Ahmed 
* Email : tanvira@ieee.org
------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
//#include <elf.h>
#include <gelf.h>
#include "read_file.h"
#include "pipeline.h"
//#include "sh2_sim.h"

/*  A.OUT STRUCT */

struct aout_head {
	unsigned short  a_info;
	unsigned short  a_magic;
	int	 	a_text;
	int		a_data;
	int		a_bss;
	int		a_syms;
	int		a_entry;
	int		a_trsize;
	int		a_drsize;
};
struct aout_head Ahead;

struct rel {
	int		r_off;
	unsigned int    r_sym: 24,
			r_extern: 1,
			     : 2,
			r_type: 5;
	int 		r_addend;
};

struct sym {
	int		s_name;
	unsigned int	s_type: 8,
			      : 24;
	int		s_value;
};

struct rel *Drel;
struct rel *Trel;
struct sym *Sym;

/*  ELF  STRUCT  */

Elf32_Ehdr	Ehead;

Elf32_Rel	*E_textrel;
Elf32_Rel	*E_datarel;
Elf32_Sym	*E_sym;

Elf32_Phdr	Phead[3];
Elf32_Shdr	Shead[32];

/*  BUFFER  */

char *Text_p, *Data_p, *Textrel_p;
char *Datarel_p, *Sym_p, *Str_p;
char *Shstr_p;
int  len, count;
int  trsz_num, drsz_num;
int  textndx   = 0;
int  datandx   = 0;
int  bssndx    = 0;
int  symtabndx = 0;
int  strtabndx = 0;

int  baseaddr = 0;

#ifdef SPARC
unsigned swap1(a) unsigned char a;
{ return (a); }

unsigned swap2(a) unsigned short a;
{ return (a); }

unsigned swap3(a) unsigned a;
{ return (a); }

unsigned swap4(a) unsigned a;
{ return (a); }
#endif

//#ifdef INTEL
unsigned swap1(unsigned char a){ return (a); }

unsigned swap2(unsigned short a){ return (((a&0xff)<<8)|(a>>8)); }

unsigned swap3(unsigned a){ return (((a&0xff)<<16)|(a&0xff00)|(a>>16)); }

unsigned swap4(unsigned a){ return ((a<<24)|((a&0xff00)<<8)|((a&0xff0000)>>8)|(a>>24)); }
//#endif

/* READ_ELF() */
unsigned int read_elf(char *file) {
  /* FILEOPEN */
  FILE *fp;
  int i;
  int ret;

  unsigned char *memp;
 
  fp = fopen(file, "r");
  if (fp == NULL){
    printf("Can't open elf_file\n");
    exit(1);
  }
  
  /* READ ELF_HEADER */
  if (fread(&Ehead, 1, sizeof Ehead, fp) !=  (sizeof Ehead)){
    printf("Can't read elf_header\n");
    exit(1);
  }
  
  Ehead.e_type      = swap2(Ehead.e_type);      /****/
  Ehead.e_machine   = swap2(Ehead.e_machine);   /****/
  Ehead.e_version   = swap4(Ehead.e_version);   /****/
  Ehead.e_entry     = swap4(Ehead.e_entry);     /****/
  Ehead.e_phoff     = swap4(Ehead.e_phoff);     /****/
  Ehead.e_shoff     = swap4(Ehead.e_shoff);     /****/
  Ehead.e_flags     = swap4(Ehead.e_flags);     /****/
  Ehead.e_ehsize    = swap2(Ehead.e_ehsize);    /****/
  Ehead.e_phentsize = swap2(Ehead.e_phentsize); /****/
  Ehead.e_phnum     = swap2(Ehead.e_phnum);     /****/
  Ehead.e_shentsize = swap2(Ehead.e_shentsize); /****/
  Ehead.e_shnum     = swap2(Ehead.e_shnum);     /****/
  Ehead.e_shstrndx  = swap2(Ehead.e_shstrndx);  /****/
  
  fseek(fp, Ehead.e_shoff, 0);
  for (i=0; i<32 && i<Ehead.e_shnum; i++) {
    fread(&Shead[i], 1, Ehead.e_shentsize, fp);
    Shead[i].sh_name      = swap4(Shead[i].sh_name);      /****/
    Shead[i].sh_type      = swap4(Shead[i].sh_type);      /****/
    Shead[i].sh_flags     = swap4(Shead[i].sh_flags);     /****/
    Shead[i].sh_addr      = swap4(Shead[i].sh_addr);      /****/
    Shead[i].sh_offset    = swap4(Shead[i].sh_offset);    /****/
    Shead[i].sh_size      = swap4(Shead[i].sh_size);      /****/
    Shead[i].sh_link      = swap4(Shead[i].sh_link);      /****/
    Shead[i].sh_info      = swap4(Shead[i].sh_info);      /****/
    Shead[i].sh_addralign = swap4(Shead[i].sh_addralign); /****/
    Shead[i].sh_entsize   = swap4(Shead[i].sh_entsize);   /****/
  }
  
  Shstr_p = (char *)malloc(Shead[Ehead.e_shstrndx].sh_size);
  fseek(fp, Shead[Ehead.e_shstrndx].sh_offset, 0);
  if (fread(Shstr_p, 1, Shead[Ehead.e_shstrndx].sh_size, fp) != Shead[Ehead.e_shstrndx].sh_size) {
    printf("Can't read shstr\n");
    exit(1);
  }
  
  for (i=0; i<32 && i<Ehead.e_shnum; i++) {
    if      (!strcmp(Shstr_p+Shead[i].sh_name, ".text"))
      textndx = i;
    else if (!strcmp(Shstr_p+Shead[i].sh_name, ".data"))
      datandx = i;
    else if (!strcmp(Shstr_p+Shead[i].sh_name, ".bss"))
      bssndx = i;
    else if (!strcmp(Shstr_p+Shead[i].sh_name, ".symtab"))
      symtabndx = i;
    else if (!strcmp(Shstr_p+Shead[i].sh_name, ".strtab"))
      strtabndx = i;
  }
  /*for (i=0; i<32 && i<Ehead.e_shnum; i++) {
    if (i == textndx || i == datandx){
      printf("-->");
    }
    printf("%x\n",Shead[i].sh_addr);
  }*/

  //printf("textndx: %d\n",textndx);
  //printf("datandx: %d\n",datandx);
  printf(GREEN" ELF:text=%8.8x "RESET, Shead[textndx].sh_size);
  printf(GREEN"data=%8.8x "RESET, Shead[datandx].sh_size);
  printf(GREEN"bss=%8.8x "RESET, Shead[bssndx].sh_size);
  printf(GREEN"symt=%8.8x "RESET, Shead[symtabndx].sh_size);
  printf(GREEN"strt=%8.8x\n"RESET, Shead[strtabndx].sh_size-1);
  
  /* READ PROGRAM_HEADER */
  fseek(fp, Ehead.e_phoff, 0);
  if ((ret = fread(Phead, Ehead.e_phnum, sizeof *Phead, fp)) !=  (sizeof *Phead)){
    printf("Can't read program_header %lx, Ehead.e_phoff=%x, ret=%0d\n", sizeof *Phead, Ehead.e_phoff, ret);
    exit(1);
  }
  Phead[0].p_type   = swap4(Phead[0].p_type);   /****/
  Phead[0].p_offset = swap4(Phead[0].p_offset); /****/
  Phead[0].p_vaddr  = swap4(Phead[0].p_vaddr);  /****/
  Phead[0].p_paddr  = swap4(Phead[0].p_paddr);  /****/
  Phead[0].p_filesz = swap4(Phead[0].p_filesz); /****/
  Phead[0].p_memsz  = swap4(Phead[0].p_memsz);  /****/
  Phead[0].p_flags  = swap4(Phead[0].p_flags);  /****/
  Phead[0].p_align  = swap4(Phead[0].p_align);  /****/
  Phead[1].p_type   = swap4(Phead[1].p_type);   /****/
  Phead[1].p_offset = swap4(Phead[1].p_offset); /****/
  Phead[1].p_vaddr  = swap4(Phead[1].p_vaddr);  /****/
  Phead[1].p_paddr  = swap4(Phead[1].p_paddr);  /****/
  Phead[1].p_filesz = swap4(Phead[1].p_filesz); /****/
  Phead[1].p_memsz  = swap4(Phead[1].p_memsz);  /****/
  Phead[1].p_flags  = swap4(Phead[1].p_flags);  /****/
  Phead[1].p_align  = swap4(Phead[1].p_align);  /****/
  
  //printf("Shead[datandx].sh_size: %x\n",Shead[datandx].sh_size);
  if (Shead[datandx].sh_size == 0) {
    /* MALLOC */
    if ((Text_p = (char *)malloc(Phead[0].p_memsz)) == NULL){
      printf("Can't malloc elf_text\n");
      exit(1);
    }
    memset(Text_p, 0, Phead[0].p_memsz-1);
  }
  else {
    /* MALLOC */
    //if ((Text_p = (char *)malloc(Phead[1].p_vaddr-Phead[0].p_vaddr)) == NULL){
    if ((Text_p = (char *)malloc(Shead[datandx].sh_addr-Shead[textndx].sh_addr)) == NULL){
      printf("Can't malloc elf_text\n");
      exit(1);
    }
    //memset(Text_p, 0, Phead[1].p_vaddr-Phead[0].p_vaddr);
    memset(Text_p, 0, Shead[datandx].sh_addr-Shead[textndx].sh_addr);
    if ((Data_p = (char *)malloc(((Phead[1].p_memsz-1)|7)+1)) == NULL){
      printf("Can't malloc elf_data\n");
      exit(1);
    }
    memset(Data_p, 0, ((Phead[1].p_memsz-1)|7)+1);
  }
  
  /* READ (text, data) */
  if (Shead[textndx].sh_size) {
    fseek(fp, Phead[0].p_offset, 0);
    if (fread(Text_p, 1, Phead[0].p_filesz, fp) != Phead[0].p_filesz){
      printf("Can't read elf_text\n");
      exit(1);
    }
    printf(YELLOW"program text size=%8.8x, offset=%8.8x\n"RESET, Phead[0].p_filesz, Phead[0].p_offset);
  }
  if (Shead[datandx].sh_size) {
    fseek(fp, Phead[1].p_offset, 0);
    if (fread(Data_p, 1, Phead[1].p_filesz, fp) != Phead[1].p_filesz){
      printf("Can't read elf_data\n");
      exit(1);
    }
  }
  
  fclose(fp);

/* CONV_DATA() */
  
  /* ELF_HEADER --> A.OUT_HEADER */
  if (Shead[datandx].sh_size == 0) {
    Ahead.a_info   = 0x00c8;
    Ahead.a_magic  = 0x0107;
    Ahead.a_text   = Shead[textndx].sh_size ? Phead[0].p_memsz:0;
    Ahead.a_data   = 0;
    Ahead.a_bss    = 0;
    Ahead.a_syms   = 0;
    Ahead.a_entry  = Phead[0].p_vaddr;
    Ahead.a_trsize = 0;
    Ahead.a_drsize = 0;
  }
  else {
    Ahead.a_info   = 0x00c8;
    Ahead.a_magic  = 0x0107;
    //Ahead.a_text   = Phead[1].p_vaddr-Phead[0].p_vaddr;
    Ahead.a_text   = Shead[datandx].sh_addr-Shead[textndx].sh_addr;

    /*printf("Ahead Text %X\n",Ahead.a_text);
    printf("Phead memsize[1] %X\n",(Phead[1].p_vaddr));
    printf("Phead memsize[0] %X\n",(Phead[0].p_vaddr));
    printf("Phead memsize[diff] %X\n",Ahead.a_text);*/

    Ahead.a_data   = ((Phead[1].p_memsz-1)|7)+1;
    Ahead.a_bss    = 0;
    Ahead.a_syms   = 0;
    Ahead.a_entry  = Phead[0].p_vaddr;
    Ahead.a_trsize = 0;
    Ahead.a_drsize = 0;
  }
  
  printf(BLUE" BSD:text=%8.8x ", Ahead.a_text);
  printf("data=%8.8x ", Ahead.a_data);
  printf("bss=-------- ");
  printf("symt=-------- ");
  printf("strt=%8.8x\n", Shead[strtabndx].sh_size-1);
  printf("e_entry=%8.8x, a_entry=%8.8x\n"RESET, Ehead.e_entry, Ahead.a_entry);

/* WRITE_AOUT() */

  if (Ahead.a_entry + Ahead.a_text + Ahead.a_data > ALOCLIMIT) {
    printf("Program too large\n");
    exit(1);
  }

  memp = &i_mem[HDRADDR];                 /* initial malloc pointer */
  * memp    = ((Ahead.a_entry+Ahead.a_text+Ahead.a_data)>>24)&0xff;
  *(memp+1) = ((Ahead.a_entry+Ahead.a_text+Ahead.a_data)>>16)&0xff;
  *(memp+2) = ((Ahead.a_entry+Ahead.a_text+Ahead.a_data)>> 8)&0xff;
  *(memp+3) = ((Ahead.a_entry+Ahead.a_text+Ahead.a_data)    )&0xff;


  memp = &i_mem[HDRADDR+4];               /* malloc limit */
  * memp    = ((ALOCLIMIT)>>24)&0xff;
  *(memp+1) = ((ALOCLIMIT)>>16)&0xff;
  *(memp+2) = ((ALOCLIMIT)>> 8)&0xff;
  *(memp+3) = ((ALOCLIMIT)    )&0xff;


  memp = &i_mem[HDRADDR+8];               /* initial stack pointer */
  * memp    = ((MEMSIZE-0x80)>>24)&0xff;
  *(memp+1) = ((MEMSIZE-0x80)>>16)&0xff;
  *(memp+2) = ((MEMSIZE-0x80)>> 8)&0xff;
  *(memp+3) = ((MEMSIZE-0x80)    )&0xff;

  memcpy(&i_mem[Ahead.a_entry], Text_p, Ahead.a_text);
  memcpy(&i_mem[Ahead.a_entry+Ahead.a_text], Data_p, Ahead.a_data);

  //free(Text_p);
  //free(Data_p);
  //free(Shstr_p);
  return (Ehead.e_entry);
}
