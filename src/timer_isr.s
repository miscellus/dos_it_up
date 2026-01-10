.intel_syntax noprefix

.section .data

	.globl _TimerTicks
	.align 4
_TimerTicks:
	.long 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.section .text

	.globl _TimerInterruptHandler
	.globl _TimerInterruptHandler_End
_TimerInterruptHandler:
	incd [_TimerTicks]
	iret
_TimerInterruptHandler_End:
