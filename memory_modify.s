        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN start_ma;
        EXTERN return_ma;
		EXTERN size_memory
        EXPORT modifyMemoryAss

; Funcion para modifiar la memoria
modifyMemoryAss
    PUSH {lr}                 ; Guarda el LR

    LDR R3, =size_memory      ; Carga la dirección de size_memory en R3
    LDR R3, [R3]              ; Carga el valor de size_memory en R3

    LDR R0, =start_ma       ; Carga la dirección en R0
    LDR R0, [R0]              ; Carga el valor de la dirección en R0
    LDR R1, =return_ma          ; Carga el dato en R1
    LDR R1, [R1]              ; Carga el valor del dato en R1

    ; Lógica para escribir en la memoria según el tamaño requerido
    CMP R3, #1                ; Compara el tamaño requerido
    BEQ write_byte            ; Si es 1, escribe un byte
    CMP R3, #2                ; Compara si es 2
    BEQ write_halfword        ; Si es 2, escribe una halfword
    CMP R3, #4                ; Compara si es 4
    BEQ write_word            ; Si es 4, escribe una palabra

    ; Agrega lógica para otros tamaños si es necesario

write_byte
    STRB R1, [R0]             ; Escribe un byte en la dirección
    B exit                    ; Salta a la salida

write_halfword
    STRH R1, [R0]             ; Escribe una halfword en la dirección
    B exit                    ; Salta a la salida

write_word
    STR R1, [R0]              ; Escribe una palabra en la dirección
    B exit                    ; Salta a la salida

exit
    POP {pc}                  ; Restaura PC y sale de la función

    END
