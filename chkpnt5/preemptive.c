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

#define bitshift(arg) \
__asm \
	mov a, r7 \
	push a \
__endasm; \
arg; \
__asm \
	pop a \
	mov r7, a \
__endasm;
#define SAVESTATE \
   { __asm \
		push ACC \
		push B \
		push DPL \
		push DPH \
		push PSW \
   	__endasm; \
   } \
   /* savedSP[currentThread] = SP; */ \
   { __asm \
		mov a, r0 \
		push a \
		mov a, _currentThread \
		add a, #_savedSP \
		mov r0, a \
		mov @r0, _SP \
		pop a \
		mov r0, a \
	__endasm; \
   }

#define RESTORESTATE \
   /* SP = savedSP[currentThread]; */ \
   { __asm \
		mov a, r1 \
		push a \
		mov a,_currentThread \
		add a,#_savedSP \
		mov r1,a \
		mov _SP,@r1 \
		pop a \
		mov r1, a \
   __endasm; \
   } \
   { __asm \
		pop PSW \
		pop DPH \
		pop DPL \
		pop B \
		pop ACC \
	__endasm; \
   }

 /* 
  * we declare main() as an extern so we can reference its symbol
  * when creating a thread for it.
  */

extern void main(void);

/*
 * Bootstrap is jumped to by the startup code to make the thread for
 * main, and restore its context so the thread can run.
 */

void Bootstrap(void) {
	TMOD = 0;  // timer 0 mode 0
    IE = 0x82;  // enable timer 0 interrupt; keep consumer polling
                // EA  -  ET2  ES  ET1  EX1  ET0  EX0
    TR0 = 1; // set bit TR0 to start running timer 0

	bitmap = 0x0;
	timer = 0;
	park[0] = park[1] = ' ';
    delayed[0] = delayed[1] = delayed[2] = delayed[3] = 0xFF;
    arrived[0] = arrived[1] = arrived[2] = arrived[3] = arrived[4] = 0xFF;
    departed[0] = departed[1] = departed[2] = departed[3] = departed[4] = 0xFF;
	SemaphoreCreate(count, 2);
    SemaphoreCreate(threads, 4);

	currentThread = ThreadCreate(main);

	RESTORESTATE;
}

/*
 * ThreadCreate() creates a thread data structure so it is ready
 * to be restored (context switched in).
 * The function pointer itself should take no argument and should
 * return no argument.
 */
ThreadID ThreadCreate(FunctionPtr fp) {
	SemaphoreWait(threads);
	EA = 0;

	for (createdThread = 0; createdThread < MAXTHREADS; createdThread++) {
		tmp = bitmap & (1 << createdThread);
		if (tmp) continue;
		break;
	}

	bitmap |= (1 << createdThread);

	startStack = ((createdThread+3) << 4) | 0x0F;

	oldSP = SP;
	SP = startStack;

	tmp = createdThread << 3;
	__asm
		push DPL
		push DPH
		MOV A, #0x0
		push A
		push A
		push A
		push A
		push _tmp
	__endasm;

	/* savedSP[createdThread] = SP; */ 
	__asm 
		mov a, r0 
		push a 
		mov a, _createdThread 
		add a, #_savedSP 
		mov r0, a 
		mov @r0, _SP 
		pop a
		mov r0, a
	__endasm; 

	SP = oldSP;
	EA = 1;
	return createdThread;        
}



/*
 * this is called by a running thread to yield control to another
 * thread.  ThreadYield() saves the context of the current
 * running thread, picks another thread (and set the current thread
 * ID to it), if any, and then restores its state.
 */

void ThreadYield(void) {
	__critical {

		SAVESTATE;
		do {

			currentThread = (currentThread+1) % MAXTHREADS;
			tmp = bitmap & (1 << currentThread);
			if (tmp)
				break;
					
					
		} while (1);
		RESTORESTATE;
	}
}


/*
 * ThreadExit() is called by the thread's own code to terminate
 * itself.  It will never return; instead, it switches context
 * to another thread.
 */
void ThreadExit(void) {
	__critical {
		bitmap = bitmap ^ (1 << currentThread);
		SemaphoreSignal(threads);
	}
	EA = 0;
	do {
		currentThread = (currentThread+1) % MAXTHREADS;
		if (currentThread == 0 && bitmap) timer++;
		tmp = bitmap & (1 << currentThread);
		if (tmp)
			break;
				
				
	} while (1);
	RESTORESTATE;

	EA = 1;
}
// it doesnt use ar6 and ar7 ??? lol
void myTimer0Handler(void) {
	EA = 0;
	SAVESTATE;
	// for (i = 0; i < 4; i++) {
	// 	if (delayed[i]) delayed[i]--;
	// }
	do {
		currentThread = (currentThread+1) % MAXTHREADS;
		if (currentThread == 0) timer++;
		tmp = bitmap & (1 << currentThread);
		if (tmp)
			break;
				
				
	} while (1);
	RESTORESTATE;

	EA = 1;
	__asm
	reti
	__endasm;
}
// void delay(unsigned char d) {
// 	// delayed[currentThread] = d;
// 	tmp = d;
// 	__asm 
// 		mov a, r0 
// 		push a 
// 		mov a, _createdThread 
// 		add a, #_delayed 
// 		mov r0, a 
// 		mov @r0, _tmp 
// 		pop a
// 		mov r0, a
// 	__endasm; 
// }
// unsigned char now(void) {
// 	return timer;
// }