#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#include <omp.h>
#define chunk 20
#include <game-of-life.h>

/* set everthing to zero */


/* generate random table */

void generate_table (int *board, int N,int Y, float threshold, int task) {

  int   i, j;
  int counter = 0;
  
  srand(time(NULL)+task); //different count for each processor
omp_set_num_threads(NUM_OF_THREADS);
#pragma omp  for schedule (static, chunk)
  for (j=0; j<N; j++) {
	  

    for (i=0; i<Y; i++) {
      Board(i,j) = ( (float)rand() / (float)RAND_MAX ) < threshold;
      counter += Board(i,j);
    }
	
	
  }
  
}

