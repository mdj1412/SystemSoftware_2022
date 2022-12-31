#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thread.h"
#include "semaphore.h"

SemaphoreTblEnt pSemaphoreTblEnt[MAX_SEMAPHORE_NUM];

int thread_sem_open(char* name, int count)
{
    //printf("start : thread_sem_open \n");
    // First, check same name
    for (int i = 0; i < MAX_SEMAPHORE_NUM; i++) {
        
        // Empty Semaphore Control Block
        if (pSemaphoreTblEnt[i].bUsed == 1) {
            continue;
        }

        // Check same name
        else {
            if (strcmp(pSemaphoreTblEnt[i].name, name) == 0) {
                return i;
            }
        }
    }


    // When the same name does not exist ...
    for (int i = 0; i < MAX_SEMAPHORE_NUM; i++) {

        // Find Empty Semaphore Control Block
        if (pSemaphoreTblEnt[i].bUsed == 1) {
            // Make Semaphore Control Block
            Semaphore* scb = malloc(sizeof(Semaphore));

            scb->count = count;
            scb->waitingQueue.queueCount = 0;
            scb->waitingQueue.pHead = NULL;
            scb->waitingQueue.pTail = NULL;

            pSemaphoreTblEnt[i].bUsed = 0;
            strcpy(pSemaphoreTblEnt[i].name, name);
            pSemaphoreTblEnt[i].pSemaphore = scb;
            return i;
        }
    }

}








int thread_sem_wait(int semid)
{
    //printf("start : thread_sem_wait \n");

    // Bring current Semaphore Control Block
    Semaphore* scb = pSemaphoreTblEnt[semid].pSemaphore;

    // Bring current Thread Control Block
    int tid = thread_self();
    Thread* tcb = pThreadTbEnt[tid].pThread;

    // Execute Thread
    if (scb->count != 0) {
        scb->count--;

        //printf("scb->count : %d\n", scb->count);
        //printf("end : thread_sem_wait \n");
        return 0;
    }

    // Acquire
    else if (scb->count == 0) {

        // Moved to Blocked state and thread waiting list
        tcb->status = THREAD_STATUS_WAIT;
        if (scb->waitingQueue.queueCount == 0) {
            scb->waitingQueue.pHead = tcb;
            scb->waitingQueue.pTail = tcb;
            tcb->pPrev = NULL;
            tcb->pNext = NULL;
        }
        else {
            scb->waitingQueue.pTail->pNext = tcb;
            tcb->pPrev = scb->waitingQueue.pTail;
            tcb->pNext = NULL;
            scb->waitingQueue.pTail = tcb;
        }
        scb->waitingQueue.queueCount++;


        pCurrentThread = NULL;


        while (1) {

            //check_queue();
            //printf("%d\n", getpid());
            //printf("%d \n", tcb->pid);



      

            //kill(tcb->pid, SIGALRM);
            



            kill(tcb->pid, SIGSTOP);

            if (scb->count != 0) break;

            /*
            else {
                tcb->status = THREAD_STATUS_WAIT;
                if (scb->waitingQueue.queueCount == 0) {
                    scb->waitingQueue.pHead = tcb;
                    scb->waitingQueue.pTail = tcb;
                    tcb->pPrev = NULL;
                    tcb->pNext = NULL;
                }
                else {
                    scb->waitingQueue.pHead->pPrev = tcb;
                    tcb->pNext = scb->waitingQueue.pHead;

                    tcb->pPrev = NULL;
                    scb->waitingQueue.pHead = tcb;
                }
                scb->waitingQueue.queueCount++;


                pCurrentThread = NULL;
            }

            */



        }
   

        scb->count--;

        //printf("scb->count : %d\n", scb->count);
        //printf("end : thread_sem_wait : %d\n", thread_self());
        return 0;
    }

    else {
        printf("thread_sem_wait : \n");
        return -1;
    }
}

int thread_sem_post(int semid)
{
    //printf("start : thread_sem_post \n");

    // Bring current Semaphore Control Block
    Semaphore* scb = pSemaphoreTblEnt[semid].pSemaphore;

    //printf("\t%d\n", scb->waitingQueue.queueCount);
    if (scb->waitingQueue.queueCount != 0) {

        //printf("aaaaaaaaaaaaaaaaaaaaaaaaa\n");

        // Remove Semaphore Waiting Queue
        Thread* tcb = scb->waitingQueue.pHead;

        if (scb->waitingQueue.queueCount == 1) {
            scb->waitingQueue.pHead = NULL;
            scb->waitingQueue.pTail = NULL;
        }
        else {
            scb->waitingQueue.pHead = tcb->pNext;
            scb->waitingQueue.pHead->pPrev = NULL;
        }
        scb->waitingQueue.queueCount--;

        // Insert to Ready Queue
        if (ReadyQueue.queueCount == 0) {
            ReadyQueue.pHead = tcb;
            ReadyQueue.pTail = tcb;
        }
        else {
            ReadyQueue.pTail->pNext = tcb;
            tcb->pPrev = ReadyQueue.pTail;
            tcb->pNext = NULL;
            ReadyQueue.pTail = tcb;
        }
        ReadyQueue.queueCount++;


        scb->count++;
        



        /*

        if (scb->count == 1) {

            // Bring current Thread Control Block
            int tid = thread_self();
            Thread* tcb = pThreadTbEnt[tid].pThread;

            // Moved to Blocked state and thread waiting list
            tcb->status = THREAD_STATUS_WAIT;
            if (scb->waitingQueue.queueCount == 0) {
                scb->waitingQueue.pHead = tcb;
                scb->waitingQueue.pTail = tcb;
                tcb->pPrev = NULL;
                tcb->pNext = NULL;
            }
            else {
                scb->waitingQueue.pTail->pNext = tcb;
                tcb->pPrev = scb->waitingQueue.pTail;
                tcb->pNext = NULL;
                scb->waitingQueue.pTail = tcb;
            }
            scb->waitingQueue.queueCount++;

            pCurrentThread = NULL;

            printf("scb->count : %d\n", scb->count);
            printf("end : thread_sem_post \n");

            kill(tcb->pid, SIGSTOP);
        }

        */

        

        return 0;
    }
    else {
        //printf("scb waiting queue is empty\n");

        scb->count++;

        //printf("scb->count : %d\n", scb->count);
        //printf("end : thread_sem_post \n");

        return 0;
    }
}







int thread_sem_close(int semid)
{
    //printf("start : thread_sem_close \n");

    pSemaphoreTblEnt[semid].bUsed = 0;
    strcpy(pSemaphoreTblEnt[semid].name, "\0");
    free(pSemaphoreTblEnt[semid].pSemaphore);
    pSemaphoreTblEnt[semid].pSemaphore = NULL;

    return 0;
}

