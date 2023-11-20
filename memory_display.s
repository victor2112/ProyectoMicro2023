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
    cmp r4, #1       ; Comprobar el tamaño de lectura
    beq readByte     ; Leer un byte si r4 es 1
    cmp r4, #2       ; Comprobar si se lee la mitad de una palabra
    beq readHalf     ; Leer la mitad de una palabra si r4 es 2
    cmp r4, #4       ; Comprobar si se lee una palabra completa
    beq readWord     ; Leer una palabra si r4 es 4
    b endLoopCopy

readByte
    ldrb r5, [r1, r3]
    strb r5, [r0, r3]
    add r1, #1
    add r3, #1
    b loopCopy

readHalf
    ldrh r5, [r1, r3]
    strh r5, [r0, r3]
    add r1, #2
    add r3, #2
    b loopCopy

readWord
    ldr r5, [r1, r3]
    str r5, [r0, r3]
    add r1, #4
    add r3, #4
    b loopCopy

endLoopCopy
    pop { pc }

    END