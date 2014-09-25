/*  This is a suite of benchmarks that are relatively short, both in program
    size and execution time.  It requires no input, and prints the execution
    time for each program, using the system- dependent routine Getclock,
    below, to find out the current CPU time.  It does a rudimentary check to
    make sure each program gets the right output.  These programs were
    gathered by John Hennessy and modified by Peter Nye. */

#include <sys/types.h>
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>


#define  false           0
#define  true            1
    /* Towers */
#define maxcells         18
    /* Towers */ /*
    discsizrange = 1..maxcells; */
#define    stackrange   3

struct element {
  int discsize;
  int next;
};


    /* Towers */
stack[stackrange+1];

struct element cellspace[maxcells+1];

int freelist,
    movesdone;



Makenull (s){
  stack[s]=0;
};


int Getelement (){
  int temp;
  if ( freelist>0 ){
    temp = freelist;
    freelist = cellspace[freelist].next;
  }
  return (temp);
};


Push(i,s) int i, s;{
  int errorfound, localel;
  errorfound=false;
  if ( stack[s] > 0 )
    if ( cellspace[stack[s]].discsize<=i ){
      errorfound=true;
    };
  if ( ! errorfound ){
    localel=Getelement();
    cellspace[localel].next=stack[s];
    stack[s]=localel;
    cellspace[localel].discsize=i;
  }
};



Init (s,n) int s, n;{
  int discctr;
  Makenull(s);
  for ( discctr = n; discctr >= 1; discctr-- )
    Push(discctr,s);
};


int Pop (s) int s;{
  int temp, temp1;
  if ( stack[s] > 0 ){
    temp1 = cellspace[stack[s]].discsize;
    temp = cellspace[stack[s]].next;
    cellspace[stack[s]].next=freelist;
    freelist=stack[s];
    stack[s]=temp;
    return (temp1);
  }
};

Move (s1,s2) int s1, s2;{
  Push(Pop(s1),s2);
  movesdone=movesdone+1;
};

tower(i,j,k) int i,j,k;{
  int other;
  if ( k==1 )
    Move(i,j);
  else {
    other=6-i-j;
    tower(i,other,k-1);
    Move(i,j);
    tower(other,j,k-1);
  }
};



Towers (){ /* Towers */
  int i;
  for ( i=1; i <= maxcells; i++ )
    cellspace[i].next=i-1;
  freelist=maxcells;
  Init(1,14);
  Makenull(2);
  Makenull(3);
  movesdone=0;
  tower(1,2,14);
  if ( movesdone != 16383 );
}; /* Towers */

main()
{
  Towers();
}




