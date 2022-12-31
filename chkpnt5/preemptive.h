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
#define CFUNC(f, l) f ## l

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

void call(FunctionPtr);

// void delay(unsigned char n);
// unsigned char now(void);

#define now() timer
#define delay(x) \
switch (currentThread) { \
    case 0: delayed[0] = now() + x; break; \
    case 1: delayed[1] = now() + x; break; \
    case 2: delayed[2] = now() + x; break; \
    case 3: delayed[3] = now() + x; break; \
}

#define isDELAY \
if (currentThread == 0) while(delayed[0] > timer) {EA=1;}\
else if (currentThread == 1) while(delayed[1] > timer) {EA = 1;}\
else if (currentThread == 2) while(delayed[2] > timer) {EA = 1;}\
else if (currentThread == 3) while(delayed[3] > timer) {EA = 1;} 

#define MakeCar(num, d) \
void Car##num() { \
    SemaphoreWait(count); \
    EA = 0; \
        if (park[0] == ' ') {park[0] = num; parklot |= 0 << num;} \
        else if (park[1] == ' ') {park[1] = num; parklot |= 1 << num;} \
        arrived[num] = now(); \
        delay(d); \
    isDELAY; \
    __critical { \
        if (park[0] == num) park[0] = ' '; \
        else if (park[1] == num) park[1] = ' '; \
        departed[num] = now(); \
        SemaphoreSignal(count); \
    } \
    ThreadExit(); \
} \

#define MakePrinter(cc) \
EA = 0; \
while(!TI);\
TI = 0;\
SBUF = 'C';\
while(!TI);\
TI = 0;\
SBUF = '0' + cc;\
while(!TI);\
TI = 0;\
SBUF = ':';\
while(!TI);\
TI = 0;\
SBUF = ' ';\
while(!TI);\
TI = 0;\
SBUF = '0'+arrived[cc];\
while(!TI);\
TI = 0;\
SBUF = '-';\
while(!TI);\
TI = 0;\
SBUF = '0'+departed[cc];\
while(!TI);\
TI = 0;\
SBUF = '@';\
while(!TI);\
TI = 0;\
SBUF = '0'+((parklot>>cc)&1);\
while(!TI);\
TI = 0;\
SBUF = '\n'; \
EA = 1;

#endif // __PREEMPTIVE_H__
