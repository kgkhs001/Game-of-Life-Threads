
/* addem.c */
#include "header.h"
//THIS IS THE MAIN FUNCTION
int main(int argc, char *argv[])
{    
    int num_threads;
    struct msg mainMsg;
    
    /*INPUT CHECKS*/
    //right number of inputs
    if(argc < 4){
        printf("Enter ./proj3 [number of threads] [FILE NAME] [NUM GENS] OPTIONAL: [PRINT] [INPUT]\n");
        exit(1);
    }

    //right type of inputs
    if(isNumber(argv[1]) == 0){
        if(atoi(argv[1]) > MAXTHREADS){
            printf("Can only make up to 10 threads\n");
            num_threads = MAXTHREADS;
        }
        else{
            num_threads = atoi(argv[1]);
        }
    }
    
    char* file = argv[2];
    file2 = fopen(file, "r");
    int num_cols = cols(file);
    int num_rows = rows(file);
    g_cols = num_cols;
    g_rows = num_rows;


    if(num_cols > MAXGRID){
        printf("restting to 40x40 matrix\n");
        num_cols = 40;
    }

    if(num_rows > MAXGRID){
        printf("restting to 40x40 matrix\n");
        num_rows = 40;
    }

    if(isNumber(argv[3]) == 0){
        num_gens = atoi(argv[3]);
        if(num_gens < 1){
            printf("Must have at least one gen\n");
            exit(1);
        }
    }
    if(argc == 5){
        if(strcmp(argv[4], "n") != 0 && strcmp(argv[4], "y") != 0){
            printf("Invalid input\n");
            exit(1);
        }
        else{
            print = argv[4];
        }
    }

    if(argc == 6){
        if(strcmp(argv[4], "n") != 0 && strcmp(argv[4], "y") != 0){
            printf("Invalid input\n");
            exit(1);
        }
        if(strcmp(argv[5], "n") != 0 && strcmp(argv[5], "y") != 0){
            printf("Invalid input\n");
            exit(1);
        }
        else{
            print = argv[4];
            input = argv[4];
        }
    }

    odd = malloc(num_rows * sizeof(int*));
    for(int i= 0; i < num_rows; i++){
        odd[i] = malloc(num_cols * sizeof(int));
    }

    //allocate to even
    even = malloc(num_rows * sizeof(int*));
    for(int j = 0; j < num_rows; j++){
        even[j] = malloc(num_cols * sizeof(int));
    }

    fill(even, file2);
    printf("Generation 0:\n");
    print_mat(even, g_rows, g_cols);
    //semaphore initialization
    //START WHILE
    int its = 0;
    while(its < 1){
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

        if(num_threads > g_rows){
            num_threads = g_rows;
        }

        
        
        pthread_t threads[num_threads];
        //create all the child threads
        for(int i = 1; i <= num_threads; i++){
            if (pthread_create(&threads[i], NULL, addem, (void *)i) != 0) {
                perror("pthread_create");
                exit(1);
            }
        }

        //printf("Initialized Sems\n");
        //send a message to all the child threads from the parent thread
        int min = 0;
        int max = 0;
        for(int i = 1; i < num_threads + 1; i++){
            if(i > 1){
                min = max + 1;
            }
            max = 0;
            max = (min - 1) + (num_rows / num_threads);
            int module = num_rows % num_threads;
            
            if(module >= i){
                max += 1;
            }


            if(max >= g_rows){
                max = g_rows -1;
            }
            mainMsg.iSender = 0;
            mainMsg.type = RANGE;
            mainMsg.value1 = min;
            mainMsg.value2 = max;
            
            SendMsg(i, &mainMsg);
            //printf("RANGE MESSAGE SENT TO %d\n", i);
        }
        int genc1 = 0;
        for(int genc = 1; genc <= num_gens; genc++){
            for(int i = 1; i <= num_threads; i++){
            mainMsg.type = GO;
            mainMsg.iSender = 0;
            SendMsg(i, &mainMsg);
            }

            for(int i = 1; i <= num_threads; i++){
                RecvMsg(0, &mainMsg);
                if(mainMsg.type != GENDONE){
                    printf("PARENT: EXPECTED TYPE GENDONE");
                }
            }
            if((all_dead(even) == 0 && all_dead(odd) == 0) || all_equal(even, odd) == 0){
                goto printer;
            }

            if(strcmp(print, "y") == 0){
                //wait for keyboard input
                if(strcmp(input, "y") == 0){
                    printf("waiting for input...\n");
                    getchar();
                }
                //print 
                printf("Generation %d: \n", genc);
                if(genc % 2 == 0){
                    print_mat(even, g_rows, g_cols);
                }
                else{
                    print_mat(odd, g_rows, g_cols);
                }
            }
            genc1 = genc;
        }
        printer:
            printf("The game ends after %d iterations with: \n", genc1);
            if(genc1 % 2 == 0){
                print_mat(even, g_rows, g_cols);
            }
            else{
                print_mat(odd, g_rows, g_cols);
            }


        //cleanup
        for(int j = 1; j <= num_threads; j++){
            (void)pthread_join(threads[j], NULL);
        }

        //destory the semaphores
        for(int i = 0; i < num_threads + 1; i++){
            (void)sem_destroy(&sendSem[i]);
            (void)sem_destroy(&recSem[i]);
        }

        its++;
    }
    //END WHILE

    //print final here

    fclose(file2);
}//end of main    
