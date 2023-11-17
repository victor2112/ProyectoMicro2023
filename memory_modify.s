        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN start_ma;
        EXTERN return_ma;
        EXPORT modifyMemoryAss

; Función para modificar la memoria
modifyMemoryAss
    push { lr }                  ; Guarda el LR

    ldr r0, =start_ma            ; Carga start_ma en r0
    ldr r0, [r0]                 ; Carga el valor start_ma en r0
    ldr r1, =return_ma           ; Carga return_ma en r1
    ldr r1, [r1]                 ; Carga el valor return_ma en r1
    strb r1, [r0]                ; Guarda el valor de r1 en la dirección de memoria apuntada por r0

    bx lr                        ; Retonar a la dirección almacenada en LR
    pop { pc }                   ; Restaurar PC

	END
