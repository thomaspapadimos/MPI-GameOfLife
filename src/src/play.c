#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include<omp.h>
#define chunk 20
#include <game-of-life.h>

void play (int *board, int *newboard, int N,int Y) {
  /*
    (copied this from some web page, hence the English spellings...)

    1.STASIS : If, for a given cell, the number of on neighbours is 
    exactly two, the cell maintains its status quo into the next 
    generation. If the cell is on, it stays on, if it is off, it stays off.

    2.GROWTH : If the number of on neighbours is exactly three, the cell 
    will be on in the next generation. This is regardless of the cell's
    current state.

    3.DEATH : If the number of on neighbours is 0, 1, 4-8, the cell will 
    be off in the next generation.
  */
  
  
  int   i, j, a;

  /* for each cell, apply the rules of Life */
#pragma omp  for schedule (static, chunk)
  for (i=0; i<Y; i++){
    for (j=0; j<N; j++) {
      a = adjacent_to (board, i, j, N);
      if (a == 2) NewBoard(i,j) = Board(i,j);
      if (a == 3) NewBoard(i,j) = 1;
      if (a < 2) NewBoard(i,j) = 0;
      if (a > 3) NewBoard(i,j) = 0;
    }
  }
#pragma omp barrier
  /* copy the new board back into the old board */
 omp_set_num_threads(NUM_OF_THREADS);
 #pragma omp  for schedule (static, chunk)
  for (i=0; i<Y; i++)
    for (j=0; j<N; j++) {
      Board(i,j) = NewBoard(i,j);
    }


	}