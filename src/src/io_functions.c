#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <game-of-life.h>

/* print the life board */

void print (int *board, int N,int Y) {
  int   i, j;

  /* for each row */
  for (j=1; j<N-1; j++) {

    /* print each column position... */
    for (i=1; i<(Y-1); i++) {
      printf ("%c", Board(i,j) ? 'x' : ' ');
    }

    /* followed by a carriage return */
    printf ("\n");
  }
}



/* display the table with delay and clear console */

void display_table(int *board, int N,int Y) {
  print (board, N,Y);
  usleep(100000);  
  /* clear the screen using VT100 escape codes */
  
}
