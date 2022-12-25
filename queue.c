#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"

void queueInitializer(Queue* q, int buffers){
    q->current = 0;
    q->max = buffers;
    q->cData = (Data*) malloc(sizeof(Data) * (buffers + 1)); //And array of client data
    for(int i = 0; i < q->max; i++){
        q->cData[i].priority = 0;
    }
}
void enqueue(Queue* q, Data d, char* sched){
    q->cData[q->current] = d;
    if(!strcmp(sched, "HPSC")){
        printf("Using the Static first Scheduler");
        for(int i = 0; i < q->current; i++){
            if(q->cData[i].priority == 0 || q->cData->priority == 5){
                Data temp = q->cData[i];
                q->cData[i] = q->cData[q->current];
                q->cData[q->current] = temp;
                break;
            }
        }
    } else if (!strcmp(sched, "HPDC")){
        printf("Using the dynamic first Scheduler");
        for(int i = 0; i < q->current; i++){
            if(q->cData[i].priority == 0 || q->cData->priority == 2){
                Data temp = q->cData[i];
                q->cData[i] = q->cData[q->current];
                q->cData[q->current] = temp;
                break;
            }
        }
    }
    q->current++;

    printf("\n");
}
Data dequeue(Queue* q){
    Data ret = q->cData[0];
    for(int i = 0; i < q->current - 1; i++){
        q->cData[i] = q->cData[i+1];
    }
    q->current--;
    //Resetting Priority
    for(int i = q->current; i < q->max; i++){
        q->cData[i].priority = 0;
    }
    return ret;
}
