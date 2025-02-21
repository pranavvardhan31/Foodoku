#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "boardgen.c"  // Include the provided board generator

#define BLOCK_COUNT 9
#define BLOCK_SIZE 3
#define BOARD_SIZE 9

// Structure to hold pipe file descriptors
typedef struct {
    int read_fd;
    int write_fd;
} PipeFDs;

// Get neighbor block numbers for a given block
void get_neighbors(int i, int* row_neighbors, int* col_neighbors) {
    if(i%3 == 0){
        row_neighbors[0] = i+1;
        row_neighbors[1] = i+2;
    }
    else if(i%3 == 1){
        row_neighbors[0] = i-1;
        row_neighbors[1] = i+1;
    }
    else if(i%3 == 2){ 
        row_neighbors[0] = i-2;
        row_neighbors[1] = i-1;
    }

    if(i/3 == 0){
        col_neighbors[0] = i+3;
        col_neighbors[1] = i+6;
    }
    else if(i/3 == 1){
        col_neighbors[0] = i-3;
        col_neighbors[1] = i+3;
    }
    else if(i/3 == 2){
        col_neighbors[0] = i-6;
        col_neighbors[1] = i-3;
    }
}

void print_help() {
    printf("\nCommands Supported\n");
    printf("\tn             Start new game\n");
    printf("\tp b c d       Place digit d in cell c of block b\n");
    printf("\ts             Show solution\n");
    printf("\th             Enter *h* for help\n");
    printf("\tq             Quit\n\n");
    printf("Nmbering scheme for blocks and cells\n");
    printf("\t+---+---+---+\n");
    printf("\t| 0 | 1 | 2 |\n");
    printf("\t+---+---+---+\n");
    printf("\t| 3 | 4 | 5 |\n");
    printf("\t+---+---+---+\n");
    printf("\t| 6 | 7 | 8 |\n");
    printf("\t+---+---+---+\n");
}

// Launch xterm for a block with appropriate geometry
void launch_block(int blockno, int pipe_in, int pipe_out, 
                    int rn1_out, int rn2_out, int cn1_out, int cn2_out) {
    char geometry[32];  // Only one geometry variable now
    int x = (blockno % 3) * 220 + 80;
    int y = (blockno / 3) * 270 + 80;
    sprintf(geometry, "16x9+%d+%d", x, y);  // Using the correct values

    char blockname[32];
    sprintf(blockname, "Block %d", blockno);

    char block_number[4];
    sprintf(block_number,"%d",blockno);

    char pipe_in_str[8], pipe_out_str[8];
    char rn1_str[8], rn2_str[8], cn1_str[8], cn2_str[8];
    sprintf(pipe_in_str, "%d", pipe_in);
    sprintf(pipe_out_str, "%d", pipe_out);
    sprintf(rn1_str, "%d", rn1_out);
    sprintf(rn2_str, "%d", rn2_out);
    sprintf(cn1_str, "%d", cn1_out);
    sprintf(cn2_str, "%d", cn2_out);

    execlp("xterm", "xterm",
           "-T", blockname,
           "-fa", "Monospace",
           "-fs", "16",
           "-fg", "white",
           "-geometry", geometry,
           "-bg", "#331100",
           "-e", "./block",
           block_number,
           pipe_in_str, pipe_out_str,
           rn1_str, rn2_str,
           cn1_str, cn2_str,
           NULL);

    perror("execlp failed");
    exit(1);
}

int main() {
    PipeFDs pipes[BLOCK_COUNT];
    pid_t pids[BLOCK_COUNT];
    int A[BOARD_SIZE][BOARD_SIZE];  // Current puzzle
    int S[BOARD_SIZE][BOARD_SIZE];  // Solution
    char command;
    int running = 1;
    
    // Create pipes for each block
    for (int i = 0; i < BLOCK_COUNT; i++) {
        int fds[2];
        if (pipe(fds) < 0) {
            perror("pipe creation failed");
            exit(1);
        }
        pipes[i].read_fd = fds[0];
        pipes[i].write_fd = fds[1];
    }
    
    // Fork child processes for each block
    for (int block = 0; block < BLOCK_COUNT; block++) {
        pids[block] = fork();
        
        if (pids[block] < 0) {
            perror("fork failed");
            exit(1);
        }
        
        if (pids[block] == 0) {  // Child process
            // Get neighbor information
            int row_neighbors[2], col_neighbors[2];
            get_neighbors(block, row_neighbors, col_neighbors);
            
            // Close unused pipe ends
            for (int i = 0; i < BLOCK_COUNT; i++) {
                if (i != block) {
                    close(pipes[i].read_fd);
                }
                if (!((i == block) || 
                      (i == row_neighbors[0]) || 
                      (i == row_neighbors[1]) || 
                      (i == col_neighbors[0]) || 
                      (i == col_neighbors[1]))) {
                    close(pipes[i].write_fd);
                }
            }
            
            // Launch xterm with block process
            launch_block(block,
                        pipes[block].read_fd,
                        pipes[block].write_fd,
                        row_neighbors[0] >= 0 ? pipes[row_neighbors[0]].write_fd : -1,
                        row_neighbors[1] >= 0 ? pipes[row_neighbors[1]].write_fd : -1,
                        col_neighbors[0] >= 0 ? pipes[col_neighbors[0]].write_fd : -1,
                        col_neighbors[1] >= 0 ? pipes[col_neighbors[1]].write_fd : -1);
        }
    }

    print_help();
    
    // Parent process: Handle user commands
    while (running) {
        printf("\nFoodoku > ");
        scanf(" %c", &command);
        
        switch (command) {
            case 'h':
                print_help();
                break;
                
            case 'n': {
                // Generate new puzzle
                newboard(A, S);
                
                // Send blocks to processes
                for (int block = 0; block < BLOCK_COUNT; block++) {
                    int row = (block / 3) * 3;
                    int col = (block % 3) * 3;
                    
                    // Redirect stdout to block's pipe
                    int saved_stdout = dup(STDOUT_FILENO);
                    dup2(pipes[block].write_fd, STDOUT_FILENO);
                    
                    // Send command and block data
                    printf("n");
                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                            printf(" %d", A[row + i][col + j]);
                        }
                    }
                    printf("\n");
                    fflush(stdout);
                    
                    // Restore stdout
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);
                }
                break;
            }
                
            case 'p': {
                int block, cell, digit;
                scanf("%d %d %d", &block, &cell, &digit);
                
                // Validate input
                if (block < 0 || block >= BLOCK_COUNT ||
                    cell < 0 || cell >= BLOCK_COUNT ||
                    digit < 1 || digit > BLOCK_COUNT) {
                    printf("Invalid input values\n");
                    break;
                }
                
                // Send command to appropriate block
                int saved_stdout = dup(STDOUT_FILENO);
                dup2(pipes[block].write_fd, STDOUT_FILENO);
                printf("p %d %d\n", cell, digit);
                fflush(stdout);
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
                break;
            }
                
            case 's': {
                // Send solution blocks to processes
                for (int block = 0; block < BLOCK_COUNT; block++) {
                    int row = (block / 3) * 3;
                    int col = (block % 3) * 3;
                    
                    int saved_stdout = dup(STDOUT_FILENO);
                    dup2(pipes[block].write_fd, STDOUT_FILENO);
                    
                    printf("n");
                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                            printf(" %d", S[row + i][col + j]);
                        }
                    }
                    printf("\n");
                    fflush(stdout);
                    
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);
                }
                break;
            }
                
            case 'q':
                // Send quit command to all blocks
                for (int block = 0; block < BLOCK_COUNT; block++) {
                    int saved_stdout = dup(STDOUT_FILENO);
                    dup2(pipes[block].write_fd, STDOUT_FILENO);
                    printf("q\n");
                    fflush(stdout);
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);
                }
                
                // Wait for all children to exit
                for (int block = 0; block < BLOCK_COUNT; block++) {
                    wait(NULL);
                }
                
                running = 0;
                break;
                
            default:
                printf("Unknown command. Type 'h' for help.\n");
        }
    }
    
    return 0;
}