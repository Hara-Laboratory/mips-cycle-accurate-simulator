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

    /* Perm */
#define    permrange 10

    /* Perm */
int permarray[permrange+1];
int pctr;

    /* Permutation program, heavily recursive, written by Denny Brown. */

Swap ( a,b ) int *a, *b; {
  int t;
  t = *a;  *a = *b;  *b = t;
};


Initialize (){
  int i;
  for ( i = 1; i <= 7; i++ ) {
    permarray[i]=i-1;
  };
};


Permute (n) int n;{   /* permute */
  int k;
  pctr = pctr + 1;
  if ( n!=1 ) {
    Permute(n-1);
    for ( k = n-1; k >= 1; k-- ) {
      Swap(&permarray[n],&permarray[k]);
      Permute(n-1);
      Swap(&permarray[n],&permarray[k]);
    };
  };
}/* permute */;


Perm (){   /* Perm */
  int i;
  pctr = 0;
  for ( i = 1; i <= 5; i++ ) {
    Initialize();
    Permute(7);
  };
  if ( pctr != 43300 );
}     /* Perm */;


main()
{
  Perm();
}




