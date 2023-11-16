	AREA |.text|, CODE, READONLY, ALIGN=2
	THUMB
	REQUIRE8
	PRESERVE8

    EXPORT memoryDisplayAss;

memoryDisplayAss
	push { lr }
	movs r3, #0
	b loopCopy

loopCopy
	ldr r4, [r1, r3]
	str r4, [r0, r3]
	add r1, #4
	add r3, #4
	cmp r1, r2
	ble loopCopy
	pop { pc }

	END