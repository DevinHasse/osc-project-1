//tanhSinhCalculator.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

typedef struct message {
    clock_t cpuElapsed;
    double sum;
} msg;

// Finds sum of tanh(0 / numCalculations) + tanh(1 / numCalculations) + ... + tanh((numCalculations - 1) / numCalculations)
// Prints time needed to complete calculation when using a specified number of child programs
int main() {    
    int numCalculations;
    int numChildren;
    int tanhOrSinh = 0;

    // Get user input for numCalculations
    printf("How many calculations will be done (1 to 1000000000): ");
    scanf("%d", &numCalculations);
    
    // Get user input for numChildren
    printf("How many child processes will be made (1 to 8): ");
    scanf("%d", &numChildren);

     // Get user input for tanh or sinh calculations
    printf("Do you want to calculate tanh or sinh tangents? (0 = tanh, 1 = sinh): ");
    scanf("%d", &tanhOrSinh);
    
    int cid = -1;                       // Child ID: -1 denotes parent, 0+ denotes specific child
    pid_t pid[numChildren];             // Program ID: The actual process id
    int pipeMap[numChildren * 2][2];    // Pipe Map: Used to map pipe inputs and outputs

    clock_t cpuStart = clock();
    
    struct timespec wallStart;
    clock_gettime(CLOCK_MONOTONIC,&wallStart);
    
    // Create pipes
    for(int i = 0; i < numChildren * 2; i++) {
        pipe(pipeMap[i]);
    }

    // Create children
    for(int i = 0; i < numChildren; i++) {  
        // Parent path
        if(cid < 0) {
            pid[i] = fork();        
        }
        
        // Child path
        if(pid[i] == 0)    {
            cid = i;
            break;
        }
    }

    // Child Path    
    if(cid >= 0) {    
        cpuStart = clock();
        msg m;
        double sum = 0;

        for(int i = cid; i < numCalculations; i += numChildren) {

            // Calculate tanh if user decided
            if (tanhOrSinh == 0){
                sum += tanh((double) i / numCalculations);
            }
            // Calculate sinh if user decided
            else if (tanhOrSinh == 1) {
                sum += sinh((double) i / numCalculations);
            }
        }

        m.sum = sum;
        m.cpuElapsed = clock() - cpuStart;

        write(pipeMap[cid + numChildren][1], &m, sizeof(msg));

        close(pipeMap[cid][0]);
        close(pipeMap[cid+numChildren][1]);
    }

    // Parent path
    else {    
        double sum = 0.0;
        clock_t cpuElapsedSum = 0;
        msg m;

        for(int i = 0; i < numChildren; i++) {
            read(pipeMap[i+numChildren][0], &m, sizeof(msg));
            sum += m.sum;
            cpuElapsedSum += m.cpuElapsed;
            close(pipeMap[i][1]);
            close(pipeMap[i+numChildren][0]);
        }

        struct timespec wallEnd;
        clock_gettime(CLOCK_MONOTONIC, &wallEnd);
        unsigned long wallElapsed = (wallEnd.tv_sec - wallStart.tv_sec) * 1000000000 + (wallEnd.tv_nsec - wallStart.tv_nsec);

        // Display tanh sum if user decided
        if (tanhOrSinh == 0){
            printf("TanH Sum: %f\n", sum);

        }
        // Display sinh sum if user decided
        else if (tanhOrSinh == 1) {
            printf("SinH Sum: %f\n", sum);

        }
        printf("CPU Time (ms): %lu\n", (unsigned long) (cpuElapsedSum * 1000) / CLOCKS_PER_SEC);
        printf("Wall Clock Time (ms): %lu\n\n", (wallElapsed)/1000000);
    }
}
