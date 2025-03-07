Inter-process Communication using Foodoku Puzzle
This project implements a multi-process Foodoku puzzle game using pipes for inter-process communication. The game renders a Foodoku puzzle (similar to Sudoku) where each 3×3 block is managed by a separate process and displayed in its own xterm window.
Overview
The Foodoku puzzle is a 9×9 grid divided into nine 3×3 blocks. The objective is to fill each empty cell with digits 1-9 such that no digit repeats in any row, column, or 3×3 block.
This implementation uses:

A coordinator process that manages the game
Nine child processes, each handling one 3×3 block
Pipes for communication between processes
xterm windows for displaying each block

Files

coordinator.c - Main coordinator process that handles user input and coordinates the game
block.c - Code for each block process that manages a 3×3 region
boardgen.c - Puzzle generator (provided in the assignment)
makefile - For compiling the project

Game Commands
User Commands (entered in the coordinator window)

h - Display help message with block and cell numbering
n - Start a new puzzle
p b c d - Place digit d in cell c of block b
s - Show the complete solution
q - Quit the game

Block Commands (internal communication)

n - Receive a new block from coordinator
p c d - Place digit d in cell c
r i d - Row-check request for digit d in row i
c j d - Column-check request for digit d in column j
q - Quit command from coordinator

Error Handling
The game handles four types of errors:

Invalid input values (checked by coordinator)
Attempting to modify cells from the original puzzle (Read-only cell)
Block conflicts (same digit appears twice in a block)
Row and column conflicts (same digit appears twice in a row or column)

Compilation and Execution
To compile:
Copymake
To run:
Copymake run
or
Copy./coordinator
To clean:
Copymake clean
Implementation Details

The coordinator forks nine child processes, one for each 3×3 block.
Each child process launches an xterm window positioned to match its location in the Foodoku grid.
Pipes are used for bidirectional communication between the coordinator and blocks and between neighboring blocks.
High-level I/O functions (printf/scanf) are used for pipe communication.
Each block communicates with four neighbors (two in the same row and two in the same column).

Visual Layout
The nine xterm windows are arranged in a 3×3 grid to visually represent the Foodoku puzzle. Each window displays its corresponding 3×3 block and any error messages related to that block.
