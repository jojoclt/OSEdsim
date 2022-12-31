/*
 * file: testpreempt.c
 */
#include <8051.h>

#include "preemptive.h"

__data __at(0x24) char isConsumer;
__data __at(0x25) char producerNum;

__data __at(0x27) char sharedBuffer[3] = {' ', ' ', ' '};
__data __at(0x2A) char nextProduceAlpha;
__data __at(0x2B) char nextProduceNum;

__data __at(0x3B) char head;
__data __at(0x3C) char tail;
__data __at(0x3D) char mutex;
__data __at(0x3E) char full;
__data __at(0x3F) char empty;

void Producer1(void) {
    nextProduceAlpha = 0;
    while (1) {
        SemaphoreWait(empty);
        SemaphoreWait(mutex);
        __critical {
            sharedBuffer[tail] = 'A' + nextProduceAlpha;
            tail = (tail + 1) % 3;
            nextProduceAlpha = (nextProduceAlpha + 1) % 26;
        }
        SemaphoreSignal(mutex);
        SemaphoreSignal(full);
    }
}
void Producer2(void) {
    nextProduceNum = 0;
    while (1) {
        SemaphoreWait(empty);
        SemaphoreWait(mutex);
        __critical {
            sharedBuffer[tail] = '0' + nextProduceNum;
            tail = (tail + 1) % 3;
            nextProduceNum = (nextProduceNum + 1) % 10;
        }
        SemaphoreSignal(mutex);
        SemaphoreSignal(full);
    }
}

void Consumer(void) {
    TMOD |= 0x20;
    TH1 = (char)-6;
    SCON = 0x50;
    TR1 = 1;

    TI = 1;
    while (1) {
        SemaphoreWait(full);
        SemaphoreWait(mutex);
        while (!TI);
        __critical {
            SBUF = sharedBuffer[head];
            TI = 0;
            head = (head + 1) % 3;
        }
        SemaphoreSignal(mutex);
        SemaphoreSignal(empty);
    }
}

void main(void) {
    SemaphoreCreate(mutex, 1);
    SemaphoreCreate(full, 0);
    SemaphoreCreate(empty, 3);
    sharedBuffer[0] = sharedBuffer[1] = sharedBuffer[2] = ' ';
    head = tail = 0;

    producerNum = 0;
    ThreadCreate(Producer1);
    ThreadCreate(Producer2);
    
    isConsumer = 1;
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
