;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 4 starter file
; March 25, 2016




        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler


SysTick_Handler                ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
    ;YOU IMPLEMENT THIS (same as Lab 3)
	PUSH	{R4-R11}	; 3) Save remaining regs r4-11
	LDR		R0, =RunPt	; 4) R0=pointer to RunPt, old thread
	LDR		R1, [R0]	;    R1 = RunPt
	STR		SP, [R1]	; 5) Save SP into TCB
	
	push {LR}
	BL Scheduler	;not sure if I need to pop anything for this as I overwrite everything anyway
	POP {LR}
	LDR R0, =RunPt
	LDR R1, [R0]			;getting the new stack pointer after running periodic tasks
	
	;LDR		R1, [R1,#4]	; 6) R1 = RunPt->next
	;STR		R1, [R0]	;    RunPt = R1
	
	LDR		SP, [R1]	; 7) new thread SP; SP = RunPt->sp;
	POP		{R4-R11}	; 8) restore regs r4-11
    CPSIE   I                  ; 9) tasks run with interrupts enabled
    BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR

StartOS
    ;YOU IMPLEMENT THIS (same as Lab 3)
    CPSIE   I                  ; Enable interrupts at processor level
    LDR R0, =RunPt				; grab the pointer
	LDR R1, [R0]				;get pointer to the thread pointer
	LDR SP, [R1]				;insert the thread pointer into SP
	POP {R4-R11}				;start poping register from stack to registers
	POP {R0-R3}					;pop R0, R3
	POP {R12}					;pop R12
	ADD SP, SP, #4				;purge initial LR from preset stack in stack data
	POP {LR}					;set LR to the first task
	ADD SP, SP, #4				;purge PSR from stack
	BX      LR                 ; start first thread

    ALIGN
    END
