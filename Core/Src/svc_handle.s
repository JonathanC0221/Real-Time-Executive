  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.global SVC_Handler_Main

  	.section  .text.SVC_Handler
  .weak  SVC_Handler
  .type  SVC_Handler, %function
SVC_Handler:
  TST lr, #4
  ITE EQ
  MRSEQ r0, MSP
  MRSNE r0, PSP
  B SVC_Handler_Main

.size  SVC_Handler, .-SVC_Handler

@ .extern stackptr
@ .global SVC_0

@   	.section  .text.SVC_0
@   .weak  SVC_0
@   .type  SVC_0, %function
@ SVC_0:
@ 	LDR r0, =stackptr
@ 	LDR r0, [r0]
@ 	LDMIA r0!, {r4-r7}
@ 	LDM r0!, {r8-r11}
@ 	//LDMIA r0!, {r4-r11}
@ 	MSR PSP, r0
@ 	MOV LR, #0xFFFFFFFD
@ 	BX LR

@ .size  SVC_0, .-SVC_0



