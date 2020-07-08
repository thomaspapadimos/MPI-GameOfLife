# Serial to Parallel 

- In this work, a serial program converted to parallel with the MPI method. This program is the well-known Game of Life game.
- In this program, a Board is created in which the cells will have the value 0 or 1 (live or dead). With the right controls and rules, each cell in each repetition changes (or does not) its state.
- So, the goal was to make a program that would do the same job, but in parallel. The same work will be done but with more than one processor, resulting in faster performance. Each processor will manage its own board and at the end the total will be displayed.

