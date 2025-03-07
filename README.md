# Foodoku Puzzle - Inter-Process Communication

## Overview

This project implements a multi-process Foodoku puzzle game using pipes for inter-process communication. The game renders a Foodoku puzzle (similar to Sudoku) where each 3×3 block is managed by a separate process and displayed in its own xterm window.

## Game Description

The Foodoku puzzle consists of a 9×9 grid divided into nine 3×3 blocks. The objective is to fill each empty cell with digits 1-9 such that:

No digit repeats in any row

No digit repeats in any column

No digit repeats in any 3×3 block

## Implementation Details

### This implementation uses:

**Coordinator Process** – Manages the game and user input.

**Nine Child Processes** – Each handling a separate 3×3 block.

**Pipes** – Used for communication between processes.

**Xterm Windows** – Each block is displayed in its own window.

## Files

**coordinator.c** – Main coordinator process that handles user input and game coordination.

**block.c** – Code for each block process that manages a 3×3 region.

**boardgen.c** – Puzzle generator (provided in the assignment).

**Makefile** – For compiling the project.

## Game Commands

### User Commands (Entered in the Coordinator Window)

**h** – Display help message with block and cell numbering.

**n** – Start a new puzzle.

**p b c d** – Place digit d in cell c of block b.

**s** – Show the complete solution.

**q** – Quit the game.

## Block Commands (Internal Communication)

**n** – Receive a new block from coordinator.

**p c d** – Place digit d in cell c.

**r i d** – Row-check request for digit d in row i.

**c j d** – Column-check request for digit d in column j.

**q** – Quit command from coordinator.

## Error Handling

**The game handles four types of errors:**

**Invalid Input Values** – Checked by the coordinator.

**Read-Only Cells** – Attempting to modify cells from the original puzzle.

**Block Conflicts** – Same digit appearing twice in a block.

**Row and Column Conflicts** – Same digit appearing twice in a row or column.


## Process Communication

The coordinator forks nine child processes, one for each 3×3 block.

Each child process launches an xterm window positioned to match its location in the Foodoku grid.

Pipes are used for bidirectional communication between the coordinator and blocks.

Blocks communicate with four neighbors (two in the same row and two in the same column).

printf/scanf are used for pipe communication.

## Visual Layout

The nine xterm windows are arranged in a **3×3 grid** to visually represent the Foodoku puzzle. Each window displays its corresponding 3×3 block and any error messages related to that block.
