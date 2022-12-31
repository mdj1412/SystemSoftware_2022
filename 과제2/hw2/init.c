#include "init.h"
#include "thread.h"
#include "scheduler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>





void signal_handler(int signum)
{
    int curpid; int newpid;


    if (signum == SIGALRM) {
        // printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tSIGALRM !!! : %d Start\n", getpid());
        if (ReadyQueue.queueCount == 0) {
            // printf("skip\n");
            return ;
        }

        // Running State 가 없을 경우
        if (pCurrentThread == NULL) {
            // // printf("SIGALRM case : 1 !!!\n");

            if (ReadyQueue.queueCount == 0) {
                // // printf("XXXXXXXXXX\n");
                return ;
            }
     
            curpid = -1;
            newpid = ReadyQueue.pHead->pid; 

            //// printf("before switch : NULL -> %d \n", newpid);
            __ContextSwitch(curpid, newpid);  
        }

        // 일반적인 경우, 
        else {
            // printf("SIGALRM case : 2 !!! : end %d thread\n", pCurrentThread->pid);
            

            curpid = pCurrentThread->pid;


            // ready queue is empty, so continue current thread
            if (ReadyQueue.queueCount == 0) {
                return;
            }

            newpid = ReadyQueue.pHead->pid;

            //// printf("before switch : %d -> %d \n", curpid, newpid);
            __ContextSwitch(curpid, newpid);

            // printf("end %d thread and start %d thread\n", curpid, newpid);
        }


//        printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tSIGALRM !!! : %d Finish\n", getpid());
//        check_queue();
    }


    else if (signum == SIGCHLD) {
    //     printf("\nSIGCHLD !!! : %d\n", pCurrentThread->pid);
        
        //check_queue();
        if (getpid() != pCurrentThread->pid) {
 //           printf("asdfasdfasdfasdfasdfasdfasdfsadfdsaf\n");
            pCurrentThread = NULL;
        }

        // printf("finish signal child\n");
    }   
}



void Init(void)
{
    ThreadQueue* tmp; 

    // 1. Create Ready Queue
    tmp = (ThreadQueue *)malloc(sizeof(ThreadQueue));
    ReadyQueue = *tmp;
    ReadyQueue.pHead = NULL;
    ReadyQueue.pTail = NULL;
    ReadyQueue.queueCount = 0;

    // 2. Create Waiting Queue
    tmp = (ThreadQueue *)malloc(sizeof(ThreadQueue));
    WaitingQueue = *tmp;
    WaitingQueue.pHead = NULL;
    WaitingQueue.pTail = NULL;
    WaitingQueue.queueCount = 0;

    // 3. Initialized Thread Table Entry
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        pThreadTbEnt[i].bUsed = 1;
        pThreadTbEnt[i].pThread = NULL;
    }
    
    // 4. Initialized pCurrentThread
    pCurrentThread = NULL;






    struct sigaction sa;

    // 1. SIGALRM 설정
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sigaction(SIGALRM, &sa, NULL);

    // 2. SIGCHLD 설정
    sa.sa_flags = SA_NOCLDSTOP;
    //sigaction(SIGCHLD, &sa, NULL);
    signal(SIGCHLD, SIG_IGN);   
    signal(SIGUSR1, signal_handler);    
}