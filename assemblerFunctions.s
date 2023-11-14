		AREA |.text|, CODE, READONLY, ALIGN=2         
		THUMB         
		REQUIRE8         
		PRESERVE8   


		EXPORT  loadResgistersContent
		EXPORT	loadR1Content
		EXPORT	updateR0
		EXPORT	updateR1
		EXPORT	updateR2
		EXPORT	updateR3
		EXPORT	updateR4
		EXPORT	updateR5
		EXPORT	updateR6
		EXPORT	updateR7
		EXPORT	updateR8
		EXPORT	updateR9
		EXPORT	updateR10
		EXPORT	updateR11
		EXPORT	updateR12
			
loadResgistersContent
	push {lr}
	str r0, [r0, #0]
	str r1, [r0, #4]
	str r2, [r0, #8]
	str r3, [r0, #12]
	str r4, [r0, #16]
	str r5, [r0, #20]
	str r6, [r0, #24]
	str r7, [r0, #28]
	str r8, [r0, #32]
	str r9, [r0, #36]
	str r10, [r0, #40]
	str r11, [r0, #44]
	str r12, [r0, #48]
	str r13, [r0, #52]
	str r14, [r0, #56]
	pop {pc}
	bx lr

loadR1Content
	push {lr}
	str r1, [r0, #0]
	pop {pc}
	bx lr
	
updateR0
	push {lr}
	movs r0, r0
	pop {pc}
	bx lr

updateR1
	push {lr}
	movs r1, r0
	pop {pc}
	bx lr
	
updateR2
	push {lr}
	movs r2, r0
	pop {pc}
	bx lr

updateR3
	push {lr}
	movs r3, r0
	pop {pc}
	bx lr
	
updateR4
	push {lr}
	movs r4, r0
	pop {pc}
	bx lr

updateR5
	push {lr}
	movs r5, r0
	pop {pc}
	bx lr
	
updateR6
	push {lr}
	movs r6, r0
	pop {pc}
	bx lr

updateR7
	push {lr}
	movs r7, r0
	pop {pc}
	bx lr

updateR8
	push {lr}
	movs r8, r0
	pop {pc}
	bx lr
	
updateR9
	push {lr}
	movs r9, r0
	pop {pc}
	bx lr

updateR10
	push {lr}
	movs r10, r0
	pop {pc}
	bx lr
	
updateR11
	push {lr}
	movs r11, r0
	pop {pc}
	bx lr

updateR12
	push {lr}
	movs r12, r0
	pop {pc}
	bx lr
	


	
	END
