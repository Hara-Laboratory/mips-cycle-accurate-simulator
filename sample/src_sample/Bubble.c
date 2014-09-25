/*  This is a suite of benchmarks that are relatively short, both in program
    size and execution time.  It requires no input, and prints the execution
    time for each program, using the system- dependent routine Getclock,
    below, to find out the current CPU time.  It does a rudimentary check to
    make sure each program gets the right output.  These programs were
    gathered by John Hennessy and modified by Peter Nye. */

#include <sys/types.h>
#include <sys/times.h>
#include <stdio.h>

#define  false           0
#define  true            1

    /* Bubble, Quick */
#define sortelements     5000
#define srtelements      500


    /* global */
int seed;


    /* Bubble, Quick */
int sortlist[sortelements+1], biggest, littlest, top;


Initrand (){
  seed = 74755;
};

int Rand (){
  seed = (seed * 1309 + 13849) & 65535;
  return( seed );
};

bInitarr(){
  int i, temp;
  Initrand();
  biggest = 0; littlest = 0;
  for ( i = 1; i <= srtelements; i++ ){
    temp = Rand();
    sortlist[i] = temp - (temp/100000)*100000 - 50000;
    if ( sortlist[i] > biggest ) biggest = sortlist[i];
    else if ( sortlist[i] < littlest ) littlest = sortlist[i];
  };
};

Bubble(){
  int i, j;
  bInitarr();
  top=srtelements;

  while ( top>1 ) {
    i=1;
    while ( i<top ) {
      if ( sortlist[i] > sortlist[i+1] ) {
        j = sortlist[i];
        sortlist[i] = sortlist[i+1];
        sortlist[i+1] = j;
      };
      i=i+1;
    };
    top=top-1;
  };
  if ( (sortlist[1] != littlest) || (sortlist[srtelements] != biggest) );
};





main()
{
  Bubble();
}




