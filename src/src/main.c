/*
 * Game of Life implementation based on
 * http://www.cs.utexas.edu/users/djimenez/utsa/cs1713-3/c/life.txt
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#include <game-of-life.h>
#include "omp.h"
#include "sys/time.h"
int NUM_OF_THREADS = 8;

int main (int argc, char *argv[]) {
  int   *board,Y, *newboard,flag=0,r;
  int task, i,numtasks;
  
  struct timeval startwtime, endwtime;
  
   if (argc != 5) { // Check if the command line arguments are correct 
    printf("Usage: %s N thres disp\n"
	   "where\n"
	   "  N     : size of table (N x N)\n"
	   "  thres : propability of alive cell\n"
           "  t     : number of generations\n"
	   "  disp  : {1: display output, 0: hide output}\n"
           , argv[0]);
    return (1);
  }
   MPI_Init(&argc,&argv);
  MPI_Status Stat;
  
  MPI_Comm_rank(MPI_COMM_WORLD, &task);
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // Input command line arguments
  int N = atoi(argv[1]);        // Array size
  double thres = atof(argv[2]); // Propability of life cell
  int t = atoi(argv[3]);        // Number of generations 
  int disp = atoi(argv[4]);     // Display output?
  
  if(numtasks==4){
	  Y=2*N;
	  N=N/2;
  }
if(numtasks==4 && task == 0) {printf("Size %dx%d with propability: %0.1f%%\n", N*4, Y, thres*100);}
   else if(task == 0) {printf("Size %dx%d with propability: %0.1f%%\n",2*N, N, thres*100);}

  

  gettimeofday (&startwtime, NULL); //count the runtime
  
//check the number of tasks  
if(numtasks>1){  
  board = NULL;
  newboard = NULL;
  
  
//-----------------------FOR 2 TASKS-----------------------------------------------------------
  if(numtasks==2){
	  int b[N];
  board = (int *)malloc(N*N*sizeof(int));

  if (board == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }

  /* second pointer for updated result */
  newboard = (int *)malloc((N)*(N)*sizeof(int));

  if (newboard == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }

  
  generate_table (board, N,N, thres,task);
  
  //Board generated
  
  

  
  
  //send and receive the respective regional cells
  
   if (task == 0) {
		  //send the first row
		  MPI_Send(&board[N], N, MPI_INT, 1, 6, MPI_COMM_WORLD);
		  
		  MPI_Recv(b,N, MPI_INT, 1, 7, MPI_COMM_WORLD, &Stat);
		 
		   Board(N-1,0)=b[N-1];   // fix the first and the
		  Board(N-1,N-1)=b[1];    // last cell of the row
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			  
			  Board(N-1,i)=b[i]; //fix the last row 
			  Board(i,0)=Board(i,N-2); // fix the side of
			  Board(i,N-1)=Board(i,1); //  the matrix
			  
		  }
		 // send  its last row and receive the last row
		 
		  MPI_Send(&board[(N-1)*(N)], N, MPI_INT, 1, 8, MPI_COMM_WORLD);
		 //communication for the first row fixing
		  MPI_Recv(b,N, MPI_INT, 1, 9, MPI_COMM_WORLD, &Stat);
		  
		  //fix the first and last cell of the row
		  Board(0,0)=b[N-1];
		  Board(0,N-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			  Board(0,i)=b[i];//fix the first row
		  }
		  
	 }
	if (task == 1) {
		  //send the first row
		  
		  MPI_Recv(b,N, MPI_INT, 0, 6, MPI_COMM_WORLD, &Stat);
		  
		  MPI_Send(&board[N], N, MPI_INT, 0, 7, MPI_COMM_WORLD);
		  
		  
		  Board(N-1,0)=b[N-1];
		  Board(N-1,N-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			  
			  Board(N-1,i)=b[i];
			  Board(i,0)=Board(i,N-2);
			  Board(i,N-1)=Board(i,1);
		  }
		  //receive the last row send the its last row
		  MPI_Recv(b,N, MPI_INT, 0, 8, MPI_COMM_WORLD, &Stat);
		 
		  MPI_Send(&board[N*(N-1)], N, MPI_INT, 0, 9, MPI_COMM_WORLD);
		  
		  
		  Board(0,0)=b[N-1];
		  Board(0,N-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			  Board(0,i)=b[i];
			  
		  }
	  
	  }
  
  
  
  
//display
  for (i=0; i<t; i++) {
	
	if(task==0){
	 printf("display the %d regeneration\n",i);
	 if (disp) display_table (board, N,N);
	 MPI_Send(&flag, 1, MPI_INT, 1, 10, MPI_COMM_WORLD);
     	
	}
	if(task==1){
	 MPI_Recv(&r,1, MPI_INT, 0, 10, MPI_COMM_WORLD, &Stat);
	 if (disp) display_table (board, N,N);
	}
    
    play (board, newboard, N,N);
	
	//fix the circuit cells for the next play
	if(i<t-1){
	if(numtasks==2){
   if (task == 0) {
		  //send the first row
		  
		  MPI_Send(&board[N], N, MPI_INT, 1, 6, MPI_COMM_WORLD);
		  
		  MPI_Recv(b,N, MPI_INT, 1, 7, MPI_COMM_WORLD, &Stat);
		  //fix the last row (N+1)
		   Board(N-1,0)=b[N-1];
		  Board(N-1,N-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			  
			  Board(N-1,i)=b[i];
			  Board(i,0)=Board(i,N-2);
			  Board(i,N-1)=Board(i,1);
			  
		  }
		 // send  its last row and receive the last row
		 
		  MPI_Send(&board[(N-1)*(N)], N, MPI_INT, 1, 8, MPI_COMM_WORLD);
		 
		  MPI_Recv(b,N, MPI_INT, 1, 9, MPI_COMM_WORLD, &Stat);
		  
		  Board(0,0)=b[N-1];
		  Board(0,N-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			  Board(0,i)=b[i];
		  }
		  
	 }
	if (task == 1) {
		  //send the first row
		  
		  MPI_Recv(b,N, MPI_INT, 0, 6, MPI_COMM_WORLD, &Stat);
		  
		  MPI_Send(&board[N], N, MPI_INT, 0, 7, MPI_COMM_WORLD);
		  
		  
		  Board(N-1,0)=b[N-1];
		  Board(N-1,N-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			  
			  Board(N-1,i)=b[i];
			  Board(i,0)=Board(i,N-2);
			  Board(i,N-1)=Board(i,1);
		  }
		  //receive the last row send the its last row
		  MPI_Recv(b,N, MPI_INT, 0, 8, MPI_COMM_WORLD, &Stat);
		 
		  MPI_Send(&board[N*(N-1)], N, MPI_INT, 0, 9, MPI_COMM_WORLD);
		  
		  
		  Board(0,0)=b[N-1];
		  Board(0,N-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			  Board(0,i)=b[i];
			  
		  }
	  
	  }
  }
  
    }
  }
 
  }
//-----------------------END----------------------------------------------------------------  


//-----------------------FOR 4 TASKS-----------------------------------------------------------
if(numtasks==4){
	
	
	int b[Y];
	
	
	N+=2; Y+=2;
	board = (int *)malloc(N*Y*sizeof(int));
   if (board == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }
   /* second pointer for updated result */
   newboard = (int *)malloc((N)*(Y)*sizeof(int));

   if (newboard == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }
   generate_table (board, N,Y, thres,task);
   
	
  
   
   if(task==0){
	   //communicate with #3
	    MPI_Send(&board[Y], Y, MPI_INT, 3, 6, MPI_COMM_WORLD);
		MPI_Recv(b,Y, MPI_INT, 3, 6, MPI_COMM_WORLD, &Stat);
		//fix the first row
		Board(0,0)=b[Y-1];
		  Board(0,Y-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<Y-1;i++){
			  Board(0,i)=b[i];
		  }
	   //communicate with #1
	   MPI_Send(&board[(N-1)*Y], Y, MPI_INT, 1, 5, MPI_COMM_WORLD);
	   MPI_Recv(b,Y, MPI_INT, 1, 5, MPI_COMM_WORLD, &Stat);
	   
	   
	   //fix the last row
	   Board(N-1,0)=b[Y-1];
		 Board(N-1,Y-1)=b[1];
		 
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			 Board(i,0)=Board(i,Y-2);
		     Board(i,Y-1)=Board(i,1);}
		  #pragma omp parallel for	 
		  for(int j=1;j<Y-1;j++){
		       Board(N-1,i)=b[i];}
			 
			  
		  
   }
   if(task==1){
	   //communicate with #2
	   MPI_Send(&board[(N-1)*Y], Y, MPI_INT, 2, 5, MPI_COMM_WORLD);
	   MPI_Recv(b,Y, MPI_INT, 2, 5, MPI_COMM_WORLD, &Stat);
	   //fix the last row
	   Board(N-1,0)=b[Y-1];
		 Board(N-1,Y-1)=b[1];
		 
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			 Board(i,0)=Board(i,Y-2);
		  Board(i,Y-1)=Board(i,1);}
		  #pragma omp parallel for
		 for(int j=1;j<Y-1;j++){
		       Board(N-1,i)=b[i];}
			 
			  
		  
		//communicate with #0
         MPI_Recv(b,Y, MPI_INT, 0, 5, MPI_COMM_WORLD, &Stat);
	     MPI_Send(&board[Y], Y, MPI_INT, 0, 5, MPI_COMM_WORLD);
	    //fix the first row
		  Board(0,0)=b[Y-1];
		  Board(0,Y-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<Y-1;i++){
			  Board(0,i)=b[i];
		  }
   }
   if(task==2){
	   //communicate with #1
	   MPI_Recv(b,Y, MPI_INT, 1, 5, MPI_COMM_WORLD, &Stat);
	   MPI_Send(&board[Y], Y, MPI_INT, 1, 5, MPI_COMM_WORLD);
	   //fix the first row
		Board(0,0)=b[Y-1];
		  Board(0,Y-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<Y-1;i++){
			  Board(0,i)=b[i];
		  }
		//communicate with #3
         MPI_Send(&board[(N-1)*Y], Y, MPI_INT, 3, 5, MPI_COMM_WORLD);
	     MPI_Recv(b,Y, MPI_INT, 3, 5, MPI_COMM_WORLD, &Stat);
	     //fix the last row
	   Board(N-1,0)=b[Y-1];
		 Board(N-1,Y-1)=b[1];
		 
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			 Board(i,0)=Board(i,Y-2);
		  Board(i,Y-1)=Board(i,1);}
			 for(int j=1;j<Y-1;j++){
		       Board(N-1,i)=b[i];}
			 
			  
		  
	   
   }
   if(task==3){
	   //communicate with #0
	    MPI_Recv(b,Y, MPI_INT, 0, 6, MPI_COMM_WORLD, &Stat);
		MPI_Send(&board[(N-1)*Y], Y, MPI_INT, 0, 6, MPI_COMM_WORLD);
		 //fix the last row
	   Board(N-1,0)=b[Y-1];
		 Board(N-1,Y-1)=b[1];
		 
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			 Board(i,0)=Board(i,Y-2);
		  Board(i,Y-1)=Board(i,1);}
			 for(int j=1;j<Y-1;j++){
		       Board(N-1,i)=b[i];}
			 
			  
		  
	  //communicate with #2
	     MPI_Recv(b,Y, MPI_INT, 2, 5, MPI_COMM_WORLD, &Stat);
	     MPI_Send(&board[Y], Y, MPI_INT, 2, 5, MPI_COMM_WORLD);
	       //fix the first row
		Board(0,0)=b[Y-1];
		  Board(0,Y-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<Y-1;i++){
			  Board(0,i)=b[i];
		  }
		
   }
   MPI_Barrier(MPI_COMM_WORLD);

//DISPLAY
for(i=0;i<t;i++){
	
	
	
	
	if(task==0){
	 printf("display the %d regeneration\n",i);
	 if (disp) display_table (board, N,Y);
	 MPI_Send(&flag, 1, MPI_INT, 1, 10, MPI_COMM_WORLD);
	}
	if(task==1){
	 MPI_Recv(&r,1, MPI_INT, 0, 10, MPI_COMM_WORLD, &Stat);
	 if (disp) display_table (board, N,Y);
	 MPI_Send(&flag, 1, MPI_INT, 2, 10, MPI_COMM_WORLD);
	}
	if(task==2){
	 MPI_Recv(&r,1, MPI_INT, 1, 10, MPI_COMM_WORLD, &Stat);	
	 if (disp) display_table (board, N,Y);
	 MPI_Send(&flag, 1, MPI_INT, 3, 10, MPI_COMM_WORLD);
	}
	if(task==3){
	 MPI_Recv(&r,1, MPI_INT, 2, 10, MPI_COMM_WORLD, &Stat);
	 if (disp) display_table (board, N,Y);
	 
	
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	play (board, newboard, N,Y);
	
//fix the circuit cells for the next play
	
 if(i<t-1){
    if(task==0){
	   //communicate with #3
	    MPI_Send(&board[Y], Y, MPI_INT, 3, 6, MPI_COMM_WORLD);
		MPI_Recv(b,Y, MPI_INT, 3, 6, MPI_COMM_WORLD, &Stat);
		//fix the first row
		Board(0,0)=b[Y-1];
		  Board(0,Y-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<Y-1;i++){
			  Board(0,i)=b[i];
		  }
	   //communicate with #1
	   MPI_Send(&board[(N-1)*Y], Y, MPI_INT, 1, 5, MPI_COMM_WORLD);
	   MPI_Recv(b,Y, MPI_INT, 1, 5, MPI_COMM_WORLD, &Stat);
	   
	   
	   //fix the last row
	   Board(N-1,0)=b[Y-1];
		 Board(N-1,Y-1)=b[1];
		 
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			 Board(i,0)=Board(i,Y-2);
		     Board(i,Y-1)=Board(i,1);}
		  #pragma omp parallel for	 
		  for(int j=1;j<Y-1;j++){
		       Board(N-1,i)=b[i];}
			 
			  
		  
   }
   if(task==1){
	   //communicate with #2
	   MPI_Send(&board[(N-1)*Y], Y, MPI_INT, 2, 5, MPI_COMM_WORLD);
	   MPI_Recv(b,Y, MPI_INT, 2, 5, MPI_COMM_WORLD, &Stat);
	   //fix the last row
	   Board(N-1,0)=b[Y-1];
		 Board(N-1,Y-1)=b[1];
		 
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			 Board(i,0)=Board(i,Y-2);
		  Board(i,Y-1)=Board(i,1);}
		  #pragma omp parallel for
		 for(int j=1;j<Y-1;j++){
		       Board(N-1,i)=b[i];}
			 
			  
		  
		//communicate with #0
         MPI_Recv(b,Y, MPI_INT, 0, 5, MPI_COMM_WORLD, &Stat);
	     MPI_Send(&board[Y], Y, MPI_INT, 0, 5, MPI_COMM_WORLD);
	    //fix the first row
		  Board(0,0)=b[Y-1];
		  Board(0,Y-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<Y-1;i++){
			  Board(0,i)=b[i];
		  }
   }
   if(task==2){
	   //communicate with #1
	   MPI_Recv(b,Y, MPI_INT, 1, 5, MPI_COMM_WORLD, &Stat);
	   MPI_Send(&board[Y], Y, MPI_INT, 1, 5, MPI_COMM_WORLD);
	   //fix the first row
		Board(0,0)=b[Y-1];
		  Board(0,Y-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<Y-1;i++){
			  Board(0,i)=b[i];
		  }
		//communicate with #3
         MPI_Send(&board[(N-1)*Y], Y, MPI_INT, 3, 5, MPI_COMM_WORLD);
	     MPI_Recv(b,Y, MPI_INT, 3, 5, MPI_COMM_WORLD, &Stat);
	     //fix the last row
	   Board(N-1,0)=b[Y-1];
		 Board(N-1,Y-1)=b[1];
		 
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			 Board(i,0)=Board(i,Y-2);
		  Board(i,Y-1)=Board(i,1);}
			 for(int j=1;j<Y-1;j++){
		       Board(N-1,i)=b[i];}
			 
			  
		  
	   
   }
   if(task==3){
	   //communicate with #0
	    MPI_Recv(b,Y, MPI_INT, 0, 6, MPI_COMM_WORLD, &Stat);
		MPI_Send(&board[(N-1)*Y], Y, MPI_INT, 0, 6, MPI_COMM_WORLD);
		 //fix the last row
	   Board(N-1,0)=b[Y-1];
		 Board(N-1,Y-1)=b[1];
		 
		  #pragma omp parallel for
		  for(int i=1;i<N-1;i++){
			 Board(i,0)=Board(i,Y-2);
		  Board(i,Y-1)=Board(i,1);}
			 for(int j=1;j<Y-1;j++){
		       Board(N-1,i)=b[i];}
			 
			  
		  
	  //communicate with #2
	     MPI_Recv(b,Y, MPI_INT, 2, 5, MPI_COMM_WORLD, &Stat);
	     MPI_Send(&board[Y], Y, MPI_INT, 2, 5, MPI_COMM_WORLD);
	       //fix the first row
		Board(0,0)=b[Y-1];
		  Board(0,Y-1)=b[1];
		  #pragma omp parallel for
		  for(int i=1;i<Y-1;i++){
			  Board(0,i)=b[i];
		  }
		
   }
 
	}
	MPI_Barrier(MPI_COMM_WORLD);
}
//-----------------------END----------------------------------------------------------------  
  
   
}
 
 
 


}
else{
	
	
  board = NULL;
  newboard = NULL;
  
  board = (int *)malloc(N*N*sizeof(int));

  if (board == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }

  /* second pointer for updated result */
  newboard = (int *)malloc(N*N*sizeof(int));

  if (newboard == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }

 generate_table (board, N,N, thres,0);
  

//DISPLAY
  for (i=0; i<t; i++) {
	  printf("display the %d generate\n",i);
    if (disp) display_table (board, N,N);
	
    play (board, newboard, N,N);    
  }
  printf("Game finished after %d generations.\n", t);
  
}
  
	MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday (&endwtime, NULL);
//print the runtime
	double init_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    printf("Pogramm runtime            : %fs\n", init_time);
	MPI_Finalize();
	printf("Game finished after %d generations.\n", t);
}