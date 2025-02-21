#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 3
#define CELL_COUNT 9

// Block state
int block_num;
int A[BLOCK_SIZE][BLOCK_SIZE];  // Original puzzle block
int B[BLOCK_SIZE][BLOCK_SIZE];  // Current state
int pipe_in, pipe_out;
int row_neighbor1_out, row_neighbor2_out;
int col_neighbor1_out, col_neighbor2_out;

void draw_block() {
    printf("\033[H\033[2J");  // Clear screen
    printf("\n");
    printf(" +---+---+---+\n");
    for (int i = 0; i < BLOCK_SIZE; i++) {
        printf(" |");
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (B[i][j] == 0) {
                printf("   |");
            } else {
                printf(" %d |", B[i][j]);
            }
        }
        printf("\n +---+---+---+\n");
    }
    
    fflush(stdout);

}

void show_error(const char* message) {
    printf("%s\n", message);
    fflush(stdout);
    sleep(2);  // Show error for 2 seconds
    draw_block();
}

// Check if placing digit d in row i causes conflict with row neighbors
int check_row_conflicts(int i, int d, int pipe_out) {
    int saved_stdout = dup(STDOUT_FILENO);
    
    // Check with first row neighbor
    dup2(row_neighbor1_out, STDOUT_FILENO);
    printf("r %d %d %d\n", i, d, pipe_out);
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);

    int response1;
    scanf("%d", &response1);
    
    dup2(row_neighbor2_out, STDOUT_FILENO);
    printf("r %d %d %d\n", i, d, pipe_out); 
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);

    int response2;
    scanf("%d", &response2);

    if(response1 !=0 || response2 !=0){
        draw_block();
        close(saved_stdout);
        fflush(stdout);
        close(saved_stdout);
        return 1;
    }

    return 0; //no row conflict
}

// Handle request to check digit d in row i
void handle_row_check(int i, int d, int requester_fd) {
    int saved_stdout = dup(STDOUT_FILENO);
    for(int j=0; j<BLOCK_SIZE; j++){
        if(B[i][j] == d){
            dup2(requester_fd, STDOUT_FILENO);
            printf("1\n");
            fflush(stdout);
            // Restore stdout
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
            return;
        }
    }
    dup2(requester_fd, STDOUT_FILENO);
    printf("0\n");
    fflush(stdout);
    // Restore stdout
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    return;
}


// Check if placing digit d in column j causes conflict with column neighbors
int check_col_conflicts(int j, int d, int requester_fd) {
    int saved_stdout = dup(STDOUT_FILENO);
    
    // Check with first column neighbor
    dup2(col_neighbor1_out, STDOUT_FILENO);
    printf("c %d %d %d\n", j, d, requester_fd);
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);

    int response1;
    scanf("%d", &response1);
    
    // Check with second column neighbor
    dup2(col_neighbor2_out, STDOUT_FILENO);
    printf("c %d %d %d\n", j, d, requester_fd);
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);

    int response2;
    scanf("%d", &response2);
    
    if(response1 !=0 || response2 !=0){
        draw_block();
        close(saved_stdout);
        fflush(stdout);
        close(saved_stdout);
        return 1;
    }

    return 0; //no coloumn conflict
}



// Handle request to check digit d in column j
void handle_col_check(int j, int d, int requester_fd) {
    int saved_stdout = dup(STDOUT_FILENO);
    for(int i=0; i<BLOCK_SIZE; i++){
        if(B[i][j] == d){
            dup2(requester_fd, STDOUT_FILENO);
            printf("1\n");
            fflush(stdout);
            // Restore stdout
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
            return;
        }
    }
    dup2(requester_fd, STDOUT_FILENO);
    printf("0\n");
    fflush(stdout);
    // Restore stdout
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    return;
}

// Check for block conflicts
int check_block_conflict(int d) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (B[i][j] == d) {
                return 1;  // Conflict found
            }
        }
    }
    return 0;  // No conflict
}

int main(int argc, char* argv[]) {
    if (argc != 8) {
        fprintf(stderr, "Usage: %s block_num pipe_in pipe_out rn1_out rn2_out cn1_out cn2_out\n", argv[0]);
        exit(1);
    }
    
    // Parse command line arguments
    block_num = atoi(argv[1]);
    pipe_in = atoi(argv[2]);
    pipe_out = atoi(argv[3]);
    row_neighbor1_out = atoi(argv[4]);
    row_neighbor2_out = atoi(argv[5]);
    col_neighbor1_out = atoi(argv[6]);
    col_neighbor2_out = atoi(argv[7]);
    

    // Redirect stdin to pipe input
    if (dup2(pipe_in, STDIN_FILENO) < 0) {
        perror("dup2 failed");
        exit(1);
    }
    
    // Initialize arrays
    memset(A, 0, sizeof(A));
    memset(B, 0, sizeof(B));
    
    // Main command loop
    char command;
    int running = 1;
    
    while (running && scanf(" %c", &command) == 1) {
        switch (command) {
            case 'n': {
                // Receive new block values
                for (int i = 0; i < BLOCK_SIZE; i++) {
                    for (int j = 0; j < BLOCK_SIZE; j++) {
                        scanf("%d", &A[i][j]);
                        B[i][j] = A[i][j];
                    }
                }
                draw_block();
                break;
            }
                
            case 'p': {
                int cell, digit;
                scanf("%d %d", &cell, &digit);
                int row = cell / BLOCK_SIZE;
                int col = cell % BLOCK_SIZE;
                
                // Check if trying to modify original puzzle cell
                if (A[row][col] != 0) {
                    show_error("Read-only cell");
                    break;
                }
                
                // Check for block conflict
                if (check_block_conflict(digit)) {
                    show_error("Block conflict");
                    break;
                }
                
                // Check for row conflicts
                if (check_row_conflicts(row, digit, pipe_out)) {
                    show_error("Row conflict");
                    break;
                }
                
                // Check for column conflicts
                if (check_col_conflicts(col, digit, pipe_out)) {
                    show_error("Column conflict");
                    break;
                }
                
                // If all checks pass, update the cell
                B[row][col] = digit;
                draw_block();
                break;
            }
                
            case 'r': {
                int row, digit, requester_fd; 
                scanf("%d %d %d", &row, &digit, &requester_fd);
                handle_row_check(row, digit, requester_fd);
                break;
            }
                
            case 'c': {
                int col, digit, requester_fd;
                scanf("%d %d %d", &col, &digit, &requester_fd);
                handle_col_check(col, digit, requester_fd);
                break;
            }
                
            case 'q':
                printf("Bye...\n");
                fflush(stdout);
                sleep(1);
                running = 0;
                break;
        }
    }
    
    return 0;
}

