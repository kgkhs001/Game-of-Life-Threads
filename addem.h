//libs to use
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

//macros and structs
#define MAXTHREADS 10
#define RANGE 1
#define ALLDONE 2
int isNumber(char *var);
//struct msg msg_gen(int index);

void* addem(void* id);
/* gcc -o pcthreads pcthreads.c -lpthread */

struct msg {
    int iSender; /* sender of the message (0 .. number-of-threads) */
    int type; /* its type */
    int value1; /* first value */
    int value2; /* second value */
};

// struct args {
//     int mailbox;
//     struct msg *message;
// };

struct msg msgs[MAXTHREADS + 1];

