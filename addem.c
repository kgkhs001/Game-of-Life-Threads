/* addem.c */
#include "addem.h"
void* SendMsg(int iTo, struct msg *pMsg);
void* RecvMsg(int iFrom, struct msg *pMsg);
//declare semaphores
//sem_t ssem, rsem; /* semaphores */
sem_t recSem[MAXTHREADS + 1];
sem_t sendSem[MAXTHREADS + 1];
int holder;
int main(int argc, char *argv[])
{    
    int num_threads;
    
    //main message that we will be storing stuff in
    struct msg mainMsg;
    //right number of inputs
    if(argc < 3){
        printf("Enter ./proj3 [number of threads]\n");
        exit(1);
    }
    //right type of inputs
    if(isNumber(argv[1]) == 0 && isNumber(argv[2]) == 0){
        if(atoi(argv[1]) > MAXTHREADS){
            printf("Can only make up to 10 threads\n");
            num_threads = MAXTHREADS;
            
        }
        else{
            num_threads = atoi(argv[1]);
            
        }
    }
    holder = atoi(argv[2]);
    //semaphore initialization
    for(int i = 0; i <= num_threads; i++){
        if (sem_init(&recSem[i], 0, 0) < 0) {
            perror("sem_init");
            exit(1);
        }

        if (sem_init(&sendSem[i], 0, 1) < 0) {
            perror("sem_init");
            exit(1);
        }
    }

    


    //send a message to all the child threads from the parent thread
    int min = 1;
    int max;
    for(int i = 1; i < num_threads + 1; i++){
        if(i > 1){
            min  = max + 1;
        }
        max = 0;
        max = (min - 1) + (atoi(argv[2])/num_threads);
        int module = atoi(argv[2])%num_threads;
        if(module >= i){
            max += 1;
        }

        mainMsg.iSender = i;
        mainMsg.type = RANGE;
        mainMsg.value1 = min;
        mainMsg.value2 = max;

        SendMsg(i, &mainMsg);
    }


    //declare thread ids
    pthread_t threads[num_threads];
    //create all the child threads
    for(int i = 1; i < num_threads + 1; i++){
        if (pthread_create(&threads[i], NULL, addem, (void *)i) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    //wait for all child threads to terminate
    for(int j = 1; j <= num_threads; j++){
        (void)pthread_join(threads[j], NULL);
    }


    //destory the semaphores
    for(int i = 0; i <= num_threads + 1; i++){
        (void)sem_destroy(&sendSem[i]);
        (void)sem_destroy(&recSem[i]);
    }
    printf("The total for %d to %d using %d threads is %d\n", 1, atoi(argv[2]), num_threads, msgs[0].value1);
}//end of main    



//if send is done before receive-> block send until receive happens
//iTo -> mailbox that you want to send to
//*pMsg -> message that you want to send
void* SendMsg(int iTo, struct msg *pMsg){
    //get the elements from args
    //store the inputted message into the array at index input
    if(iTo == 0){
        sem_wait(&sendSem[iTo]);
        msgs[iTo].iSender =pMsg->iSender;
        msgs[iTo].type = pMsg->type;
        msgs[iTo].value1 += pMsg->value1;
        msgs[iTo].value2 = pMsg->value2;
        sem_post(&recSem[iTo]);        
    }
    else{
        sem_wait(&sendSem[iTo]);
        msgs[iTo].iSender =pMsg->iSender;
        msgs[iTo].type = pMsg->type;
        msgs[iTo].value1 = pMsg->value1;
        msgs[iTo].value2 = pMsg->value2;
        sem_post(&recSem[iTo]);
    }

}


//this is passed an empty struct with an index to a valid mailbox

//iFrom -> this is the mailbox you want to receive from 
//*pMsg -> a pointer to an empty struct that you will fill with the message at the iFrom address
void* RecvMsg(int iFrom, struct msg *pMsg){
    sem_wait(&recSem[iFrom]);
    //set the empty struct to the found values
    pMsg->iSender = msgs[iFrom].iSender;
    pMsg->type = msgs[iFrom].type;
    pMsg->value1 = msgs[iFrom].value1;
    pMsg->value2 = msgs[iFrom].value2;
    sem_post(&sendSem[iFrom]);
}

//function to add all previous values to sum
void* addem(void* id){
   //struct msg msgi;//empty message
    struct msg newMsg;
    RecvMsg((int)id, &newMsg);//want to recieve a message from parent thread this will populate msgi
    
    if((int) id > holder){
    	pthread_exit(NULL);
    }
    
    newMsg.iSender = id;
    newMsg.type = ALLDONE;
    //newMsg.value1 = msgi.value1;
    // newMsg.value2 = msgi.value2;
    
    for(int i = newMsg.value1 + 1; i <= newMsg.value2; i++){
        newMsg.value1 = newMsg.value1 + i;
    }


    //send back to the main thread
    SendMsg(0, &newMsg);
    //receive to the main thread
    RecvMsg(0, &newMsg);
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
//makes an instance of a message struct

