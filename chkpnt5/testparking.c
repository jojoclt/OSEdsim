/*
 * file: testpreempt.c
 */
#include <8051.h>

#include "preemptive.h"

__data __at (0x20) char savedSP[MAXTHREADS];
__data __at (0x24) char bitmap;
__data __at (0x25) ThreadID currentThread;

__data __at (0x26) ThreadID createdThread;
__data __at (0x27) char startStack;
__data __at (0x28) char oldSP;
__data __at (0x29) char tmp;
__data __at (0x2A) char i;
__data __at (0x2B) char parklot;
__data __at (0x2C) char threads;
__data __at (0x2D) char count; // Semaphores
__data __at (0x2E) char park[2];

__data __at (0x30) char delayed[4];

__data __at (0x35) char arrived[5];
__data __at (0x3A) char departed[5];
__data __at (0x3F) char timer;


MakeCar(0, 2)
MakeCar(1, 3)
MakeCar(2, 3)
MakeCar(3, 1)
MakeCar(4, 2)

void lol() {
    while(1);
}
void printer() {
    TMOD |= 0x20;
    TH1 = (char)-6;
    SCON = 0x50;
    TR1 = 1;

    TI = 1;
    while(departed[0] == 0xFF || departed[1] == 0xFF || departed[2] == 0xFF || departed[3] == 0xFF || departed[4] == 0xFF);
    __critical {
        
        MakePrinter(0)
        MakePrinter(1)
        MakePrinter(2)
        MakePrinter(3)
        MakePrinter(4)
        ThreadExit();
    }
}

void main(void) {

    ThreadCreate(Car0);

    ThreadCreate(Car1);

    ThreadCreate(Car2);

    ThreadCreate(Car3);

    ThreadCreate(Car4);
    
    printer();
}

void _sdcc_gsinit_startup(void) {
    __asm 
        ljmp _Bootstrap
    __endasm;
}

void _mcs51_genRAMCLEAR(void) {}
void _mcs51_genXINIT(void) {}

void timer0_ISR(void) __interrupt(1) {
    __asm
        ljmp _myTimer0Handler
    __endasm;
}
