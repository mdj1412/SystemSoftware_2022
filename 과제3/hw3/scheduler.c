
#include "init.h"
#include "thread.h"
#include "semaphore.h"
#include "scheduler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>










// 주의 : 인자 -> Linux thread pid
void __ContextSwitch(int curpid, int newpid)
{

    //printf("__ContextSwitch() : start !\n");
    //check_queue();

    // 1. Running State 무조건 있는 경우
    if (curpid != -1) {
        if (pCurrentThread == NULL)
        {
            printf("error curpid is no NULL but pCurThread is NULL\n");
        }
        if (ReadyQueue.queueCount == 0) {
            printf("__ContextSwitch() : not change\n");
            return ;
        }



        if (pCurrentThread->status != THREAD_STATUS_ZOMBIE) {
            // A. CPU에서 실행 중인 thread를 정지시킴



            if (pCurrentThread->status == THREAD_STATUS_READY) {}




            // B. 기존에 있던 pCurrentThread를 Ready Queue로 이동
            else if (ReadyQueue.queueCount == 0)
            {
                printf("Ready Queue is 0\n");
            }
            else
            {
                kill(curpid, SIGSTOP);
                pCurrentThread->status = THREAD_STATUS_READY;
                pCurrentThread->pPrev = ReadyQueue.pTail;
                pCurrentThread->pNext = NULL;
                ReadyQueue.pTail->pNext = pCurrentThread;
                ReadyQueue.pTail = pCurrentThread;

                ReadyQueue.queueCount++;
            }
            pCurrentThread->cpu_time += 2;
        }
    }



    // 2. Ready Queue에 있는거 Running 상태로 바꾸기

    // 3. Ready Queue 설정
    if (ReadyQueue.queueCount == 0) {
        printf("Ready Queue is 0\n");
        return ;
    }
    else if (ReadyQueue.queueCount == 1) {
        pCurrentThread = ReadyQueue.pHead;
        pCurrentThread->status = THREAD_STATUS_RUN;
        ReadyQueue.pHead = NULL;
        ReadyQueue.pTail = NULL;
    }
    else {
        pCurrentThread = ReadyQueue.pHead;
        pCurrentThread->status = THREAD_STATUS_RUN;
        ReadyQueue.pHead = ReadyQueue.pHead->pNext;
        ReadyQueue.pHead->pPrev = NULL;
        ReadyQueue.pTail->pNext = NULL;
    }
    ReadyQueue.queueCount--;


    pCurrentThread->pNext = NULL;
    pCurrentThread->pPrev = NULL;


    // 4. 새롭게 실행할 thread를 실행
    kill(newpid, SIGCONT);
    kill(newpid, SIGUSR1);

    //check_queue();
    //printf("__contextswitching() : end !\n");
}




/* [ struct itimerval timer; ]
struct itimerval {
    struct timeval it_interval;
    struct timeval it_value;
};
struct timeval it_interval;
: 타이머의 간격 정보 저장
: it_interval의 값이 0 이면 다음 번에 타이머가 만료될 때 타이머 기능이 멈춘다.
struct timeval it_value;
: 타이머가 만료될 때까지 남은 시간이 저장
: it_value의 값이 0 이면 타이머 기능이 멈춘다.

struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
} */




void RunScheduler(void)
{
    struct itimerval it;

    /*
    struct sigaction sa;

    // 1. SIGALRM 설정
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sigaction(SIGALRM, &sa, NULL);

    // 2. SIGCHLD 설정
    sa.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    */

    // 3. timer 설정
    it.it_interval.tv_sec = 1;
    it.it_interval.tv_usec = 999 * 1000;
    it.it_value.tv_sec = 1;
    it.it_value.tv_usec = 999 * 1000;



    // 4. Ready Queue로 부터 Running State 설정
    pCurrentThread = ReadyQueue.pHead;
    ReadyQueue.pHead = ReadyQueue.pHead->pNext;
    pCurrentThread->pPrev = NULL;
    pCurrentThread->pNext = NULL;
    pCurrentThread->status = THREAD_STATUS_RUN;
    ReadyQueue.queueCount--;


    // 5. Set Timer
    setitimer(ITIMER_REAL, &it, NULL);


    // 6. pCurrentThread Running State로 시작p
    kill(pCurrentThread->pid, SIGCONT);


}