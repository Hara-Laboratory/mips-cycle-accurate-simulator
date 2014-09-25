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

Try(i, q, a, b, c, x) int i, *q, a[], b[], c[], x[]; {
  int j;
  j = 0;
  *q = false;
  while ( (! *q) && (j != 8) ) { 
    j = j + 1;
    *q = false;
    if ( b[j] && a[i+j] && c[i-j+7] ) { 
      x[i] = j;
      b[j] = false;
      a[i+j] = false;
      c[i-j+7] = false;
      if ( i < 8 ) { 
        Try(i+1,q,a,b,c,x);
        if (!*q) { 
          b[j] = true;
          a[i+j] = true;
          c[i-j+7] = true;
        }
      }
      else *q = true;
    }
  }
};
        
Doit () {
  int i,q;
  int a[9], b[17], c[15], x[9];
  i = 0 - 7;
  while ( i <= 16 ) {
    if ( (i >= 1) && (i <= 8) ) a[i] = true;
    if ( i >= 2 ) b[i] = true;
    if ( i <= 7 ) c[i+7] = true;
    i = i + 1;
  };

  Try(1, &q, b, a, c, x);
  if (!q);
};


Queens () {
  int i;
  for ( i = 1; i <= 1; i++ ) Doit();
};

main()
{
  Queens();
}
