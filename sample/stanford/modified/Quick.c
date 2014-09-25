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


    /* Bubble, Quick */
#define sortelements     5000

int seed;

    /* Bubble, Quick */
int sortlist[sortelements+1],
    biggest, littlest,
    top;

Initrand () {
  seed = 74755;
};

int Rand () {
  seed = (seed * 1309 + 13849) & 65535;
  return( seed );
};

    /* Sorts an array using quicksort */
Initarr() {
  int i, temp;
  Initrand();
  biggest = 0; littlest = 0;
  for ( i = 1; i <= sortelements; i++ ) {
    temp = Rand();
    sortlist[i] = temp - (temp/100000)*100000 - 50000;
    if ( sortlist[i] > biggest ) biggest = sortlist[i];
    else if ( sortlist[i] < littlest ) littlest = sortlist[i];
  };
};

Quicksort( a,l,r) int a[], l, r;/* quicksort the array A from start to finish */{
  int i,j,x,w;

  i=l; j=r;
  x=a[(l+r) / 2];
  do {
    while ( a[i]<x ) i = i+1;
    while ( x<a[j] ) j = j-1;
      if ( i<=j ) {
        w = a[i];
        a[i] = a[j];
        a[j] = w;
        i = i+1;    j= j-1;
      }
  } 
  while ( i<=j );
    if ( l <j ) Quicksort(a,l,j);
    if ( i<r ) Quicksort(a,i,r);
};



Quick () {
  Initarr();
  Quicksort(sortlist,1,sortelements);
  if ( (sortlist[1] != littlest) || (sortlist[sortelements] != biggest) );
};

main()
{
  Quick();
}
