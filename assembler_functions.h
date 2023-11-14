#ifndef ASSEMBLER_FUNCTIONS_H
#define ASSEMBLER_FUNCTIONS_H

#include <stdint.h>

extern void memoryDisplayAss(uint32_t *memory, uint32_t start, uint32_t end);
extern void updateR0(unsigned long register_content);
extern void updateR1(unsigned long register_content);
extern void updateR2(unsigned long register_content);
extern void updateR3(unsigned long register_content);
extern void updateR4(unsigned long register_content);
extern void updateR5(unsigned long register_content);
extern void updateR6(unsigned long register_content);
extern void updateR7(unsigned long register_content);
extern void updateR8(unsigned long register_content);
extern void updateR9(unsigned long register_content);
extern void updateR10(unsigned long register_content);
extern void updateR11(unsigned long register_content);
extern void updateR12(unsigned long register_content);

#endif