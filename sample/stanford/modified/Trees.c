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


#define  nil             0
#define  false           0
#define  true            1
    /* Bubble, Quick */
#define sortelements     5000
   /* tree */
struct node {
  struct node *left,*right;
  int val;
};

int    seed;

    /* tree */
struct node *tree;

    /* Bubble, Quick */
int sortlist[sortelements+1],
    biggest, littlest,
    top;

Initrand (){
  seed = 74755;
};

int Rand (){
  seed = (seed * 1309 + 13849) & 65535;
  return( seed );
};


    /* Sorts an array using treesort */
tInitarr(){
  int i, temp;
  Initrand();
  biggest = 0; littlest = 0;
  for ( i = 1; i <= sortelements; i++ )
  {
    temp = Rand();
    sortlist[i] = temp - (temp/100000)*100000 - 50000;
    if ( sortlist[i] > biggest ) biggest = sortlist[i];
    else if ( sortlist[i] < littlest ) littlest = sortlist[i];
  };
};


CreateNode (t,n) struct node **t; int n;{
  *t = (struct node *)malloc(sizeof(struct node)); 
  (*t)->left = nil; (*t)->right = nil;
  (*t)->val = n;
};


        /* insert n into tree */
Insert(n, t) int n; struct node *t;{
  if ( n > t->val ) 
    if ( t->left == nil ) CreateNode(&t->left,n);
    else Insert(n,t->left);
  else if ( n < t->val )
    if ( t->right == nil ) CreateNode(&t->right,n);
    else Insert(n,t->right);
};

    /* check by inorder traversal */
int Checktree(p) struct node *p;{
  int result;
  result = true;
  if ( p->left != nil ) 
    if ( p->left->val <= p->val ) result=false;
    else result = Checktree(p->left) && result;
  if ( p->right != nil )
    if ( p->right->val >= p->val ) result = false;
    else result = Checktree(p->right) && result;
  return( result);
}; /* checktree */

Trees(){
  int i;
  tInitarr();
  tree = (struct node *)malloc(sizeof(struct node)); 
  tree->left = nil; tree->right=nil; tree->val=sortlist[1];
  for ( i = 2; i <= sortelements; i++ ) Insert(sortlist[i],tree);
  if ( ! Checktree(tree) );
};

main(){
  Trees();
}




