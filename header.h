//libs to use
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>

//macros and structs
#define MAXTHREADS 10
#define RANGE 1
#define ALLDONE 2
#define GO 3
#define GENDONE 4 // Generation Done
#define MAXGRID 40

//STRUCTS TO USE
struct msg {
    int iSender; /* sender of the message (0 .. number-of-threads) */
    int type; /* its type */
    int value1; /* first value */
    int value2; /* second value */
};

//array of mailboxes
struct msg msgs[MAXTHREADS + 1];
//IMPORTANT VARIABLES
//semaphore arrays
sem_t recSem[MAXTHREADS + 1];
sem_t sendSem[MAXTHREADS + 1];
//OTHER GLOBAL VARIABLES
FILE *file2;
int num_gens;
int gen_c;//what gen are we on?
char buf[50];
char* tok;
char* print = "n";
char* input = "n";
int g_rows, g_cols;
int** boards[3];
//the arrays we are using 
int** odd; 
int** even;


//FUNCTION DECLARATIONS
void print_mat(int** board, int rowsa, int colsa);
void playing(int start, int rows, int cols, int** prev, int** nextArr);

/*FUCNTIONS THAT YOU ARE GOING TO USE*/

//if send is done before receive-> block send until receive happens
//iTo -> mailbox that you want to send to
//*pMsg -> message that you want to send
void* SendMsg(int iTo, struct msg *pMsg){
    //get the elements from args
    
    sem_wait(&sendSem[iTo]);
    //printf("Sent\n");
    msgs[iTo].iSender =pMsg->iSender;
    msgs[iTo].type = pMsg->type;
    msgs[iTo].value1 = pMsg->value1;
    msgs[iTo].value2 = pMsg->value2;
    sem_post(&recSem[iTo]);        
}


//this is passed an empty struct with an index to a valid mailbox

//iFrom -> this is the mailbox you want to receive from 
//*pMsg -> a pointer to an empty struct that you will fill with the message at the iFrom address
void* RecvMsg(int iFrom, struct msg *pMsg){
    
    sem_wait(&recSem[iFrom]);
    //printf("received\n");
    //set the empty struct to the found values
    pMsg->iSender = msgs[iFrom].iSender;
    pMsg->type = msgs[iFrom].type;
    pMsg->value1 = msgs[iFrom].value1;
    pMsg->value2 = msgs[iFrom].value2;
    sem_post(&sendSem[iFrom]);
}

//function to add all previous values to sum
void* addem(void* id1){
    int id = (int) id1;
    struct msg empty;
    //RECEIVE MESSAGE TYPE RANGE
    RecvMsg(id, &empty);
    if(empty.type != RANGE){
        printf("CHILD: EXPECTED TYPE RANGE\n");
    }

    int start = empty.value1;
    int end = empty.value2;
    int odd_ct;
    for(int i = 1; i <= num_gens; i++){
        struct msg go_msg;
        RecvMsg(id, &go_msg);
        if(go_msg.type != GO){
            printf("CHILD: EXPECTED TYPE GO\n");
        }

        if(i % 2 == 1){//if this generation is odd
            playing(start, end, g_cols, even, odd);
        }

        else{
            playing(start, end, g_cols, odd, even);
        }

        //send gendone message here
        struct msg some;
        some.iSender = id;
        some.type = GENDONE;
        SendMsg(0, &some);
        //printf("GENDONE SENT from %d\n", id);
    }
}


//Helper function to tell if an input is number
//returns 1 if it is not and 0 if it is
int isNumber(char *var){
    char num[strlen(var) + 1];
    strcpy(num, var);
    int boolean = 0;
    int j;
    while(j < strlen(var) && boolean == 0){
        if(num[j] >= '0' && num[j] <= '9'){
            j++;
        }
        else{
            boolean = 1;
        }
    }
    return boolean;
}

//gets the number of rows from the file

int rows(char* file){
    int fd;
    if ((fd = open (file, O_RDONLY)) < 0) { //O_RDONLY creates a read only file
        perror("Could not open file");
        exit (1);
    }

    char buf[5000];
    int bytes = read(fd, buf, 5000);//read everything into the buffer
    int count = 1;//default you have one row

    for(int i = 0; i < bytes; i++){
        if(buf[i] == '\n'){
            count += 1;
        }
    }

    close(fd);
    //printf("rows is %d\n", count);
    return count;
}


//gets the number of columns from the file

int cols(char* file){
    int fd;
    if ((fd = open (file, O_RDONLY)) < 0) { //O_RDONLY creates a read only file
        perror("Could not open file");
        exit (1);
    }
    char buf[100];
    int bytes = read(fd, buf, 100);//read everything into the buffer
    //all the columns should be the same size so every line should have the same number of columns in it
    int count = 0;//default you have one column
    for(int i = 0; buf[i] != '\n' && i < bytes; i++){
        if(buf[i] == '1' || buf[i] == '0'){
            count += 1;
        }
    }

    close(fd);
    //printf("cols is %d\n", count);
    return count;
}


//filling the initial board up
void fill(int ** board, FILE* file){
    int col = 0;
    int row = 0;
    
    //while(fgets(input, sizeof(input), file) != NULL){
    for(char c = getc(file); c != EOF; c = getc(file)){
        if(c == '1'){
            board[row][col] = 1;
            col++;
        }
        if(c == '0'){
            board[row][col] = 0;
            col++;
        }
        if(c == '\n'){
            row++;
            col = 0;
        }
    }
    
}//end of fill


//print the inputted matrix
void print_mat(int** board, int rowsa, int colsa){
    for(int i = 0; i < rowsa; i++){
        if(i > 0){
            printf("\n");
        }
        for(int j = 0; j < colsa; j++){
            printf("%d ", board[i][j]);
        }
    }
    printf("\n");
}

//this will return the number of sorrounding alive of that specific cell
int neighbors(int** board, int row, int col, int cols_t, int rows_t){
    //for(int i = 0; i < cols; i++){
        int count_alive = 0; //keep count of all the alive ones
        //to the right
        if(col < cols_t - 1){
            //printf("right\n");
            if(board[row][col + 1] == 1){
                count_alive++;
            }
            //printf("count alive %d\n", count_alive);
        }

        //to the left
        if(col > 0){
            if(board[row][col - 1] == 1){
                //printf("left\n");
                count_alive++;
            }
            //printf("count alive %d\n", count_alive);
        }

        //above
        if(row > 0){
            if(board[row - 1][col] == 1){
                //printf("above\n");
                count_alive++;
            }
            //printf("count alive %d\n", count_alive);
        }

        //below
        if(row < rows_t -1){
            if(board[row + 1][col] == 1){
                //printf("below\n");
                count_alive++;
            }
            //printf("count alive %d\n", count_alive);
        }

        //above to the right
        if(row > 0 && col < cols_t -1){
            if(board[row - 1][col + 1] == 1){
                //printf("above to the right\n");
                count_alive++;
            }
            //printf("count alive %d\n", count_alive);
        }

        //above to the left
        if(row > 0 && col > 0){
            if(board[row - 1][col - 1] == 1){
                //printf("above to the left\n");
                count_alive++;
            }
            //printf("count alive %d\n", count_alive);
        }

        //below to the right
        if(row < rows_t-1 && col < cols_t-1){
            if(board[row + 1][col + 1] == 1){
                //printf("below to the right\n");
                count_alive++;
            }
            //printf("count alive %d\n", count_alive);
        }

        //below to the left
        if(row < rows_t-1 && col > 0){
            if(board[row + 1][col - 1] == 1){
                //printf("below to the left\n");
                count_alive++;
            }
            //printf("count alive %d\n", count_alive);
        }
        return count_alive;
    //}//end of for
}//end of play_by_row


void playing(int start, int rows, int cols, int** prev, int** nextArr){
    //allocate memory for the array
    int hold;
    //allocate the new array
    if(start == rows){
        int i = start;
        for(int k = 0; k < cols; k++){
            hold = neighbors(prev, i, k, g_cols, g_rows);
            //SURVIVAL
            if(prev[i][k] == 1){
                if(hold > 3 || hold < 2){
                    nextArr[i][k] = 0;
                }
                else{
                    nextArr[i][k] = 1;
                }
            }//end of if alive

            if(prev[i][k] == 0){
                if(hold == 3){
                    nextArr[i][k] = 1;
                }
                else{
                    nextArr[i][k] = 0;
                }
            }//end of dead
        }//end inner for
    }
    //this will go through each row column by column and adjust accordingly in the new matrix
    for(int i = start; i <= rows; i++){
        for(int k = 0; k < cols; k++){
            hold = neighbors(prev, i, k, g_cols, g_rows);
            //SURVIVAL
            if(prev[i][k] == 1){
                if(hold > 3 || hold < 2){
                    nextArr[i][k] = 0;
                }
                else{
                    nextArr[i][k] = 1;
                }
            }//end of if alive

            if(prev[i][k] == 0){
                if(hold == 3){
                    nextArr[i][k] = 1;
                }
                else{
                    nextArr[i][k] = 0;
                }
            }//end of dead
        }//end inner for
    }//end outer for
}//end playing

int all_dead(int** board){
    for(int i = 0; i < g_rows; i++){
        for(int j = 0; j < g_cols; j++){
            if(board[i][j] == 1){
                return 1;
            }
        }
    }
    return 0;//this means that all the cells are dead
}


int all_equal(int** board, int** board2){
    for(int i = 0; i < g_rows; i++){
        for(int j = 0; j < g_cols; j++){
            if(board[i][j] != board2[i][j]){
                return 1;
            }
        }
    }
    return 0;//this means that all the cells are dead
}