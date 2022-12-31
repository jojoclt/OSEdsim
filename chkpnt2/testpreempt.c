/*
 * file: testpreempt.c
 */
#include <8051.h>

#include "preemptive.h"

__data __at(0x3A) char bufferEmpty;
__data __at(0x3B) char sharedBuffer;
__data __at(0x3C) char nextProduce;

void Producer(void) {
    nextProduce = 'A';
    while (1) {
        if (bufferEmpty) {
            __critical {
                sharedBuffer = nextProduce++;
                if (nextProduce > 'Z') nextProduce = 'A';
                bufferEmpty = !bufferEmpty;
            }
        }
    }
}

void Consumer(void) {
    TMOD |= 0x20;
    TH1 = (char)-6;
    SCON = 0x50;
    TR1 = 1;

    TI = 1;
    while (1) {
        if (!bufferEmpty) {
            while (!TI);
            __critical {
                SBUF = sharedBuffer;
                TI = 0;
                bufferEmpty = !bufferEmpty;
            }
        }
    }
}

void main(void) {
    bufferEmpty = 1;
    sharedBuffer = ' ';
    ThreadCreate(Producer);
    Consumer();
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
