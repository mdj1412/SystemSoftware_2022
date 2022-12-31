#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "init.h"
#include "thread.h"
#include "scheduler.h"

#include <linux/sched.h>

#define GNU_SOURCE

ThreadQueue ReadyQueue;
ThreadQueue WaitingQueue;
ThreadTblEnt pThreadTbEnt[MAX_THREAD_NUM];
Thread* pCurrentThread; // Running ������ Thread�� ����Ű�� ����

// thread ������ Size (Bytes)
int STACK_SIZE = 1024 * 64;
int TRUE = 1; int FALSE = 0;


void check_queue(void) {
    printf("\n\n=====================================\n");
    Thread *temp;

    for (int i = 0; i < 6; i++) {
        if (pThreadTbEnt[i].pThread == NULL) { printf("NULL\n"); continue; }
        printf("%d, ", pThreadTbEnt[i].pThread->pid);
        if (pThreadTbEnt[i].pThread->status == THREAD_STATUS_RUN) { printf("run\n"); }
        if (pThreadTbEnt[i].pThread->status == THREAD_STATUS_READY) { printf("ready\n"); }
        if (pThreadTbEnt[i].pThread->status == THREAD_STATUS_WAIT) { printf("wait\n"); }
        if (pThreadTbEnt[i].pThread->status == THREAD_STATUS_ZOMBIE) { printf("zombie\n"); }
    }
    if (pCurrentThread == NULL) {
        printf("current thread is null\n");
    }
    else {
        printf("current thread : %d\n", pCurrentThread->pid);
    }

    temp = WaitingQueue.pHead;
    printf("current waiting queue : ");
    for(;temp!=NULL;temp=temp->pNext){
        printf(" %d",temp->pid);
        if(temp->status != 2)
        {
            printf("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[Not Waiting Queue\n");
            //assert(0);
        }
    }
    printf("\t %d\n", WaitingQueue.queueCount);


    temp = ReadyQueue.pHead;
    printf("current ready queue : ");
    for(;temp!=NULL;temp=temp->pNext){
        printf(" %d",temp->pid);
        if(temp->status != 1)
        {
            printf("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[Not Ready Queue\n");
            //assert(0);
        }
    }
    printf("\t %d\n", ReadyQueue.queueCount);





    printf("=====================================\n\n");
}

































int thread_create(thread_t *thread, thread_attr_t *attr, void *(*start_routine)(void*), void *arg)
{
    // 1. Call clone() : clone() ���� ���ο� child thread ����
    char* pStack = (char *)malloc(STACK_SIZE);   // Stack ���� �Ҵ�
    pid_t pid = clone(start_routine, (char *)pStack + STACK_SIZE, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, arg);

    //printf("thread_create() start : pid : %d\n", pid);

    // Parent Thread ���� ����
    if (pid != 0) {

        // 2. Call kill(pid, SIGSTOP) : ������ child thread�� ���� ��Ŵ
        kill(pid, SIGSTOP);


        // 3. Allocate TCB (thread control block)
        Thread* tcb = malloc(sizeof(Thread));


        // 4. Initialize TCB through child thread info : TCB �ʱ�ȭ
        // 5. Set child thread status to ready
        tcb->stackSize = STACK_SIZE;
        tcb->stackAddr = (char*)pStack + STACK_SIZE;
        tcb->status = THREAD_STATUS_READY;
        tcb->pid = pid;     // Linux thread id
        tcb->cpu_time = 2;
        tcb->pNext = NULL;


        // 6. Move TCB to ready queue
        if (ReadyQueue.queueCount == 0) {
            ReadyQueue.pHead = tcb;
            ReadyQueue.pTail = tcb;
            tcb->pNext = NULL;
            tcb->pPrev = NULL;
        }
        else {
            ReadyQueue.pTail->pNext = tcb;
            tcb->pPrev = ReadyQueue.pTail;
            ReadyQueue.pTail = tcb;
        }
        ReadyQueue.queueCount++;


        // 7. ���ο� TCB�� Thread Table �� ����
        int index = 0;
        while (1) {
            if (pThreadTbEnt[index].bUsed == TRUE) {
                pThreadTbEnt[index].bUsed = FALSE;
                pThreadTbEnt[index].pThread = tcb;

                // �߿� !!!
                *thread = index;
                break;
            }
            index++;

            // ���� ������ Thread ���� �ʰ�
            if (index == MAX_THREAD_NUM) {
                printf("thread_create() : ���� ������ �ִ� Thread�� ������ �Ѱ���ϴ�.\n");
            }
        }
    }
}








// Ready Queue or Waiting Queue ---> Waiting Queue
// ���� : thread id�� Thread Table�� index ��
int thread_suspend(thread_t tid)
{
    //printf("start thread_suspend() : %d, %d\n", getpid(), tid);

    // 0. thread table entry���� thread pointer ��������
    Thread* tcb = pThreadTbEnt[tid].pThread;


    // 1. tid�� Ready Queue �� ���� ���
    if (tcb->status == THREAD_STATUS_READY) {

        //printf("\t\t\t\t\t\t\tReady Queue -> Waiting Queue %d\n", tid);
        //check_queue();


        // A. Ready Queue�� ����Ǿ� �ִ� tid ����

        // Remove tid in Ready Queue
        if (ReadyQueue.pHead == tcb) {
            if (ReadyQueue.pHead == ReadyQueue.pTail) {
                ReadyQueue.pTail = NULL;
                ReadyQueue.pHead = NULL;
            }
            else {
                ReadyQueue.pHead = tcb->pNext;
                ReadyQueue.pHead->pPrev = NULL;
                tcb->pNext = NULL;
                tcb->pPrev = NULL;
            }
        }
        else if (ReadyQueue.pTail == tcb) {
            ReadyQueue.pTail = tcb->pPrev;
            ReadyQueue.pTail->pNext = NULL;
            tcb->pPrev = NULL;
            tcb->pNext = NULL;
        }
        else {
            tcb->pNext->pPrev = tcb->pPrev;
            tcb->pPrev->pNext = tcb->pNext;
            tcb->pPrev = NULL;
            tcb->pNext = NULL;
        }

        
        // �߿� !!!
        ReadyQueue.queueCount--;


        // B. Waiting Queue�� Tail�� ����
        if (WaitingQueue.queueCount == 0) {
            WaitingQueue.pHead = tcb;
            WaitingQueue.pTail = tcb;
            tcb->pPrev = NULL;
            tcb->pNext = NULL;
            WaitingQueue.queueCount++;
        }
        else {
            tcb->pPrev = WaitingQueue.pTail;
            WaitingQueue.pTail->pNext = tcb;
            WaitingQueue.pTail = tcb;  
            tcb->pNext = NULL;
            WaitingQueue.queueCount++;
        }

        tcb->status = THREAD_STATUS_WAIT;

        //printf("Change status wait : thread_suspend() \n");
        //check_queue();
        
        return 0;
    }


    // 2. tid�� Waiting Queue �� ���� ���
    else if (tcb->status == THREAD_STATUS_WAIT) {
        // A. Waiting Queue�� ����Ǿ� �ִ� tid ����

        // Tail �� ���� ���
        if (WaitingQueue.pHead == tcb) {
            if (WaitingQueue.pHead == WaitingQueue.pTail) {
                WaitingQueue.pTail = NULL;
                WaitingQueue.pHead = NULL;
            }
            else {
                WaitingQueue.pHead = tcb->pNext;
                WaitingQueue.pHead->pPrev = NULL;
                tcb->pPrev = NULL;
            }
        }
        else if (WaitingQueue.pTail == tcb) {
            WaitingQueue.pTail = tcb->pPrev;
            WaitingQueue.pTail->pNext = NULL;
            tcb->pNext = NULL;
        }
        else {
            tcb->pNext->pPrev = tcb->pPrev;
            tcb->pPrev->pNext = tcb->pNext;
            tcb->pPrev = NULL;
            tcb->pNext = NULL;
        }

        // B. Waiting Queue�� Tail�� ����
        if (WaitingQueue.queueCount == 0) {
            WaitingQueue.pHead = tcb;
            WaitingQueue.pTail = tcb;
            tcb->pPrev = NULL;
            tcb->pNext = NULL;
        }
        else {
            tcb->pPrev = WaitingQueue.pTail;
            WaitingQueue.pTail->pNext = tcb;
            WaitingQueue.pTail = tcb;
        }

        tcb->pNext = NULL;
        WaitingQueue.queueCount++;

        return 0;
    }


    // 3. �� �� �ƴ� ���
    else { 
        printf("thread_suspend() : ready queue, waitiing queue �� �� �ƴ�\n");
        return -1; 
    }
}























int thread_cancel(thread_t tid)
{

//    printf("thread_cancel() start \n");
//    check_queue();


    // 0. thread table entry���� thread pointer ��������
    Thread* tcb = pThreadTbEnt[tid].pThread;

    // 1. Kill(tpid, SIGKILL)
    kill(tcb->pid, SIGKILL);

    // 2. tid�� Ready Queue �� ���� ���
    if (tcb->status == THREAD_STATUS_READY) {
        // A. Remove TCB from ready list
        // A. Ready Queue�� ����Ǿ� �ִ� tid ����

        // Remove tid in Ready Queue
        if (ReadyQueue.queueCount == 0) {
            printf("thread_resume() : Problem 1\n");
        }
        else if (ReadyQueue.queueCount == 1) {
            if (ReadyQueue.pHead != tcb) {
                printf("thread_resume() : Problem 2\n");
            }
            ReadyQueue.pHead = NULL;
            ReadyQueue.pTail = NULL;
        }
        else {
            if (ReadyQueue.pHead == tcb) {
                ReadyQueue.pHead = ReadyQueue.pHead->pNext;
                ReadyQueue.pHead->pPrev = NULL;
            }
            else if (ReadyQueue.pTail == tcb) {
                ReadyQueue.pTail = ReadyQueue.pTail->pPrev;
                ReadyQueue.pTail->pNext = NULL;
            }
            else {
                tcb->pPrev->pNext = tcb->pNext;
                tcb->pNext->pPrev = tcb->pPrev;
            }
        }
        
        // �߿� !!!
        ReadyQueue.queueCount--;


        // B. tid�� thread table entry �ʱ�ȭ
        pThreadTbEnt[tid].bUsed = TRUE;
        pThreadTbEnt[tid].pThread = NULL;

        // C. Deallocate target thread TCB
        free(tcb);


 //       printf("thread_cancel() Finish \n");
//        check_queue();
        return 0;
    }


    // 3. tid�� Waiting Queue �� ���� ���
    else if (tcb->status == THREAD_STATUS_WAIT) {
        // A. Remove TCB from waiting list
        // A. Waiting Queue�� ����Ǿ� �ִ� tid ����

        // Remove tid in Waiting Queue
        if (WaitingQueue.queueCount == 0) {
            printf("thread_resume() : Problem 1\n");
        }
        else if (WaitingQueue.queueCount == 1) {
            if (WaitingQueue.pHead != tcb) {
                printf("thread_resume() : Problem 2\n");
            }
            WaitingQueue.pHead = NULL;
            WaitingQueue.pTail = NULL;
        }
        else {
            if (WaitingQueue.pHead == tcb) {
                WaitingQueue.pHead = WaitingQueue.pHead->pNext;
                WaitingQueue.pHead->pPrev = NULL;
            }
            else if (WaitingQueue.pTail == tcb) {
                WaitingQueue.pTail = WaitingQueue.pTail->pPrev;
                WaitingQueue.pTail->pNext = NULL;
            }
            else {
                tcb->pPrev->pNext = tcb->pNext;
                tcb->pNext->pPrev = tcb->pPrev;
            }
        }
        
        // �߿� !!!
        WaitingQueue.queueCount--;


        // B. tid�� thread table entry �ʱ�ȭ
        pThreadTbEnt[tid].bUsed = TRUE;
        pThreadTbEnt[tid].pThread = NULL;

        // C. Deallocate target thread TCB
        free(tcb);

        return 0;
    }

    else if (tcb->status == THREAD_STATUS_ZOMBIE) {
        //printf("thread_cancel() : zombie : %d\n", tcb->pid);

        pThreadTbEnt[tid].bUsed = TRUE;
        pThreadTbEnt[tid].pThread = NULL;

        free(tcb);

        //check_queue();
        //printf("Finish thread_cancel() \n");
        return 0;
    }


    // 4. �� �� �ƴ� ���
    else { 
        printf("thread_cancel() : ready queue, waitiing queue �� �� �ƴ�\n");
        return -1; 
    }
}




















// Ready Queue(?) or Waiting Queue ---> Ready Queue
int thread_resume(thread_t tid)
{
    //printf("start thread_resume() : %d\n", getpid());

    // 0. thread table entry���� thread pointer ��������
    Thread* tcb = pThreadTbEnt[tid].pThread;

    if (tcb == NULL) { printf("thread_resume() : null \n"); }


    // 1. ready ���¶�� ???
    if (tcb->status == THREAD_STATUS_READY) {
        printf("thread_resume() : ready -> ready ...? \n");
        return 0;
    }


    


    // 2. �׷���, ������ waiting ���¶�� �����ϴ� ��...
    if (tcb->status == THREAD_STATUS_WAIT) {
        // A. Set target thread status to ready
        tcb->status = THREAD_STATUS_READY;

        // ???
        //kill(getpid(), SIGSTOP);


        // B. Move TCB from waiting queue to ready queue
        // a) Waiting Queue�� ����Ǿ� �ִ� tid ����

        // Remove tid in Waiting Queue
        if (WaitingQueue.queueCount == 0) {
            printf("thread_resume() : Problem 1\n");
        }
        else if (WaitingQueue.queueCount == 1) {
            if (WaitingQueue.pHead != tcb) {
                printf("thread_resume() : Problem 2\n");
            }
            WaitingQueue.pHead = NULL;
            WaitingQueue.pTail = NULL;
        }
        else {
            if (WaitingQueue.pHead == tcb) {
                WaitingQueue.pHead = WaitingQueue.pHead->pNext;
                WaitingQueue.pHead->pPrev = NULL;
            }
            else if (WaitingQueue.pTail == tcb) {
                WaitingQueue.pTail = WaitingQueue.pTail->pPrev;
                WaitingQueue.pTail->pNext = NULL;
            }
            else {
                tcb->pPrev->pNext = tcb->pNext;
                tcb->pNext->pPrev = tcb->pPrev;
            }
            tcb->pNext = NULL;
            tcb->pPrev = NULL;
        }
        
        // �߿� !!!
        WaitingQueue.queueCount--;


        // B. Ready Queue�� Tail�� ����
        if (ReadyQueue.queueCount == 0) {
            ReadyQueue.pHead = tcb;
            ReadyQueue.pTail = tcb;
            tcb->pPrev = NULL;
            tcb->pNext = NULL;
        }
        // else if (ReadyQueue.queueCount == 1) {
        //     ReadyQueue.pHead->pNext = tcb;
        //     tcb->pPrev = ReadyQueue.pHead;
        //     ReadyQueue.pTail = tcb;
        // }
        else {
            ReadyQueue.pTail->pNext = tcb;
            tcb->pPrev = ReadyQueue.pTail;
            ReadyQueue.pTail = tcb;
        }

        // �߿� !!!
        ReadyQueue.queueCount++;


        //kill(getpid(), SIGSTOP);

        //printf("Finish thread_resume() : ready queue : %d, waiting queue : %d\n", ReadyQueue.queueCount, WaitingQueue.queueCount);
        //check_queue();
        return 0;
    }
    

    // 3. ready queue, waiting queue �� �� �ƴ� ��, 
    else {
        printf("thread_resume() : ready queue, waitiing queue �� �� �ƴ�\n");
        return -1; 
    }
}








































// ���� : thread table entry���� index�� ��ȯ
thread_t thread_self(void)
{
    // 1. ���� ó�� : pCurrentThread == NULL
    if (pCurrentThread == NULL) {
        printf("thread_self() : pCurrentThread == NULL \n");
        return -1;
    }


    // 2. thread table entry���� ã��
    int index = 0;
    while (1) {
        if (pThreadTbEnt[index].pThread == pCurrentThread) {
            return index;
        }
        index++;

        // �������� �ʴ� ���
        if (index == MAX_THREAD_NUM) {
            printf("thread_self() : ���� ������ �ִ� Thread�� ������ �Ѱ���ϴ�.\n");
            return -1;
        }
    }
}





















// ���� : thread index
int thread_join(thread_t tid)
{
    //printf("start thread_join() \n");
    
    pid_t parent_pid = getpid();

    int parent_idx = -1;

    int j=0;
    if(pThreadTbEnt[tid].bUsed == TRUE)
    {
        return 0;
    }

    // 0. find thread table index
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        if (pThreadTbEnt[i].bUsed == 1) { j++; continue; }
        else if (pThreadTbEnt[i].pThread->pid == parent_pid) {
            parent_idx = i;
        }
    }
    // 해당 pid가 없는 경우
    if (parent_idx == -1) { 
        printf("signal_handler() : Prblem 1 : SIGCHLD %d, %d\n ", j, parent_pid);
    }
    Thread* parent_tcb = pThreadTbEnt[parent_idx].pThread;

    if (parent_tcb == NULL) { printf("nonono\n"); }


    // 0.
    Thread* tcb = pCurrentThread;
    int curpid; int newpid;

    // 1. Set this current thread status to waiting
    // printf("Change THREAD_STATUS_WAIT at thread_join() : %d\n", getpid());
    if (pCurrentThread->status != THREAD_STATUS_ZOMBIE) {
        pCurrentThread->status = THREAD_STATUS_WAIT;

        //printf("Change status wait : thread_join()\n");
        
        //check_queue();
    }


    // 2. Move this thread's TCB to waiting queue
    if (WaitingQueue.queueCount == 0) {
        WaitingQueue.pHead = tcb;
        WaitingQueue.pTail = tcb;
        tcb->pPrev = NULL;
        tcb->pNext = NULL;
    }
    else if (WaitingQueue.queueCount == 1) {
        WaitingQueue.pHead->pNext = tcb;
        tcb->pPrev = WaitingQueue.pHead;
        WaitingQueue.pTail = tcb;
    }
    else {
        WaitingQueue.pTail->pNext = tcb;
        tcb->pPrev = WaitingQueue.pTail;
        WaitingQueue.pTail = tcb;
    }
    WaitingQueue.queueCount++;



    // 3. Select new thread to run on CPU
    //    Remove new thread's TCB from ready queue
    //    Set new thread status to running
    /*
    if (ReadyQueue.queueCount == 0) {
        pCurrentThread = NULL;
    }
    else if (ReadyQueue.queueCount == 1) {
        pCurrentThread = ReadyQueue.pHead;
        pCurrentThread->status = THREAD_STATUS_RUN;

        kill(pCurrentThread->pid, SIGCONT);

        ReadyQueue.pHead = NULL;
        ReadyQueue.pTail = NULL;
        ReadyQueue.queueCount--;
    }
    else {
        pCurrentThread = ReadyQueue.pHead;
        pCurrentThread->status = THREAD_STATUS_RUN;

        kill(pCurrentThread->pid, SIGCONT);

        ReadyQueue.pHead = pCurrentThread->pNext;
        ReadyQueue.pHead->pPrev = NULL;
        ReadyQueue.queueCount--;

        pCurrentThread->pNext = NULL;
        pCurrentThread->pPrev = NULL;
    }
    */
    


    // 4. Context switching to the new thread


    // 5. SIGCHLD
    //pause();


    // 6. Remove child's TCB from thread table
    //    deallocate child's TCB
    //check_queue();
    
    while(1) {

        //printf("\t\t\t\tjoin() : check same child thread tid : %d\n", tid);
        
        
        Thread* tmp = pThreadTbEnt[tid].pThread;
        if (pThreadTbEnt[tid].pThread->status == THREAD_STATUS_ZOMBIE) {
            //printf("\t\t\t\tjoin() : same child thread\n");

  
            // 1. move this thread's TCB into ready queue
            // 2. Set thread status to ready

            //printf("before resume() \n");
            //printf("parent index : %d\n", parent_idx);
            // A. Set target thread status to ready
            tcb->status = THREAD_STATUS_READY;

            // ???
            //kill(getpid(), SIGSTOP);


            // B. Move TCB from waiting queue to ready queue
            // a) Waiting Queue�� ����Ǿ� �ִ� tid ����

            tcb = pThreadTbEnt[parent_idx].pThread;

            // Remove tid in Waiting Queue
            if (WaitingQueue.queueCount == 0) {
                printf("thread_resume() : Problem 1\n");
            }
            else if (WaitingQueue.queueCount == 1) {
                if (WaitingQueue.pHead != tcb) {
                    printf("thread_resume() : Problem 2\n");
                }
                WaitingQueue.pHead = NULL;
                WaitingQueue.pTail = NULL;
            }
            else {
                if (WaitingQueue.pHead == tcb) {
                    WaitingQueue.pHead = WaitingQueue.pHead->pNext;
                    WaitingQueue.pHead->pPrev = NULL;
                }
                else if (WaitingQueue.pTail == tcb) {
                    WaitingQueue.pTail = WaitingQueue.pTail->pPrev;
                    WaitingQueue.pTail->pNext = NULL;
                }
                else {
                    tcb->pPrev->pNext = tcb->pNext;
                    tcb->pNext->pPrev = tcb->pPrev;
                }
                tcb->pNext = NULL;
                tcb->pPrev = NULL;
            }
            
            // �߿� !!!
            WaitingQueue.queueCount--;


            // B. Ready Queue�� Tail�� ����
            if (ReadyQueue.queueCount == 0) {
                ReadyQueue.pHead = tcb;
                ReadyQueue.pTail = tcb;
                tcb->pPrev = NULL;
                tcb->pNext = NULL;
            }
            // else if (ReadyQueue.queueCount == 1) {
            //     ReadyQueue.pHead->pNext = tcb;
            //     tcb->pPrev = ReadyQueue.pHead;
            //     ReadyQueue.pTail = tcb;
            // }
            else {
                ReadyQueue.pTail->pNext = tcb;
                tcb->pPrev = ReadyQueue.pTail;
                ReadyQueue.pTail = tcb;
            }

            // �߿� !!!
            ReadyQueue.queueCount++;


//            kill(getpid(), SIGSTOP);


            thread_cancel(tid);


            //printf("finish thread_join() \n");
            break;
        }

        // ������
        else {
            //printf("\t\t\t\tjoin() : different child thread \n");
            __ContextSwitch(getpid(), ReadyQueue.pHead->pid);
            pause();

            //kill(parent_pid, SIGSTOP);

        }
    }

}





























int thread_cputime(void)
{
    // 1. ���� ó�� : pCurrentThread == NULL
    if (pCurrentThread == NULL) {
        printf("thread_self() : pCurrentThread == NULL \n");
        return -1;
    }


    return pCurrentThread->cpu_time;
}




int zombie = 1;

void thread_exit(void)
{
    //printf("\nchange zombie : %d, num of zombie : %d ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\n", pCurrentThread->pid, zombie);

    zombie++;
    pCurrentThread->status = THREAD_STATUS_ZOMBIE;

    //check_queue();
}