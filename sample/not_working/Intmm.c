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

    /* Intmm, Mm */
#define rowsize          40
int seed;
    /* Intmm, Mm */
int ima[rowsize+1][rowsize+1], imb[rowsize+1][rowsize+1], imr[rowsize+1][rowsize+1];

Initrand () {
  seed = 74755;
};


int Rand () {
  seed = (seed * 1309 + 13849) & 65535;
  return( seed );
};

Initmatrix ( m ) int m[rowsize+1][rowsize+1]; {
  int temp, i, j;
  for ( i = 1; i <= rowsize; i++ )
    for ( j = 1; j <= rowsize; j++ ) {
      temp = Rand();
      m[i][j] = temp - (temp/120)*120 - 60;
    }
};

Innerproduct( result,a,b, row,column) int *result,a[rowsize+1][rowsize+1],b[rowsize+1][rowsize+1],row,column;/* computes the inner product of A[row,*] and B[*,column] */{
  int i;
  *result = 0;
  for(i = 1; i <= rowsize; i++ )*result = *result+a[row][i]*b[i][column];
};


Intmm () {
  int i, j;
  Initrand();
  Initmatrix (ima);
  Initmatrix (imb);
  for ( i = 1; i <= rowsize; i++ )
    for ( j = 1; j <= rowsize; j++ ) Innerproduct(&imr[i][j],ima,imb,i,j);
};


main()
{
  Intmm();
}




