/*
 * file: preemptive.h
 *
 * this is the include file for the preemptive multithreading
 * package.  It is to be compiled by SDCC and targets the EdSim51 as
 * the target architecture.
 *
 * CS 3423 Fall 2018
 */

#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__

#define MAXTHREADS 4  /* not including the scheduler */
/* the scheduler does not take up a thread of its own */

#define CNAME(s) _ ## s
#define CLABEL(l) l ## $

#define SemaphoreCreate(s, n) *&s=n

#define SemaphoreWaitBody(s, label) \
{ __asm \
    CLABEL(label):  \
        mov a, CNAME(s) \
        /* equal 0 and less than*/ \
        jz CLABEL(label)  \
        jb ACC.7, CLABEL(label) \
        dec  CNAME(s) \
__endasm; \
}

#define SemaphoreWait(s) SemaphoreWaitBody(s, __COUNTER__)

#define SemaphoreSignal(s) \
{ __asm \
    inc CNAME(s) \
__endasm; \
} 


typedef char ThreadID;
typedef void (*FunctionPtr)(void);

ThreadID ThreadCreate(FunctionPtr);
void ThreadYield(void);
void ThreadExit(void);

#endif // __PREEMPTIVE_H__
