#include <8051.h>

#include "preemptive.h"

__data __at (0x30) char savedSP[MAXTHREADS];
__data __at (0x34) int bitmap;
__data __at (0x35) ThreadID currentThread;

__data __at (0x20) ThreadID createdThread;
__data __at (0x21) char startStack;
__data __at (0x22) char oldSP;
__data __at (0x23) int tmp;

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
	EA = 0;

	if (bitmap == (1 << MAXTHREADS) - 1) return -1;

	for (createdThread = 0; createdThread < MAXTHREADS; createdThread++) {
		__asm
			push 6
			push 7
		__endasm;
		tmp = bitmap & (1 << createdThread);
		__asm
			pop 7
			pop 6
		__endasm;
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

			__asm
				push 6
				push 7
			__endasm;
			tmp = bitmap & (1 << currentThread);
			__asm
				pop 7
				pop 6
			__endasm;
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

		/*
			* clear the bit for the current thread from the
			* bit mask, decrement thread count (if any),
			* and set current thread to another valid ID.
			* Q: What happens if there are no more valid threads?
			*/
		RESTORESTATE;
	}
}

void myTimer0Handler(void) {
	EA = 0;
	SAVESTATE;
		do {

			currentThread = (currentThread+1) % MAXTHREADS;

			__asm
				push 6
				push 7
			__endasm;
			tmp = bitmap & (1 << currentThread);
			__asm
				pop 7
				pop 6
			__endasm;
			if (tmp)
				break;
					
					
		} while (1);
		RESTORESTATE;
	EA = 1;
	__asm
	reti
	__endasm;
}