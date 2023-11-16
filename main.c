#include "stm32f3xx.h" // Device header

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "usart_utils.h"
#include "utils.h"
#include "commands.h"

#include "assembler_functions.h"
#include "call.h"
#include "memory_modify.h"
#include "memory_display.h"

uint32_t start_ma;
uint32_t return_ma;

char buffer[MAX_COMMAND_LENGTH];
char error_flag = 0;

// USART con interrupciones
volatile char data;
char command[MAX_COMMAND_LENGTH];
char *character;
char *instruction;
char *args[] = {" ", " ", " ", " "};
char *token;
int characters_iterator = 0;
int arguments_iterator = 0;

// Register Display
void registerDisplay();
static uint32_t registers[REGISTERS_NUM];

// Register Modify
void registerModify();
char *register_number;
unsigned long register_content;
static char *endptr;

// Memory Display
void memoryDisplay();
static char *start;
static char *end;
unsigned long start_memory_display;
unsigned long end_memory_display;
static uint32_t memory_range_blocks;
uint32_t *display_memory;
char arr_display_memory[64];

// Memory Modify
void memoryModify();
static char *md_addr;
static char *md_data;
static char *md_size;
unsigned int size_memory = 0;
unsigned long address_mm;
unsigned long data_mm;

// RUN
void runCommand();
unsigned long run_addr;

// CALL
void callCommand();
unsigned long memory_call_address;

// Block Fill
void blockFill();
static char *bf_start;
static char *bf_end;
static char *bf_data;
static char *bf_size;
int size_bf = 0;

int main(void)
{

	USART_config(115200);

	USART_putString("\n\r               Microprocesadores 2023                  \n\r");
	USART_putString("                     Programa Monitor                      \n\r");
	USART_putString("                      Victor Amado                         \n\r");
	USART_putString("                      Cruz Ambrocio                        \n\r");
	USART_putString("                     Christian Osorio                      \n\r");
	USART_putString("                      Celeste Batres                   \n\r\n\r");
	USART_putString(">> ");

	while (1)
	{
	}
}

void USART2_IRQHandler(void)
{
	if (USART2->ISR & USART_ISR_RXNE)
	{

		data = USART2->RDR;
		if (data != '\r')
		{ // Esperamos hasta obtener un enter es decir una linea
			command[characters_iterator] = USART2->RDR;
			character = &command[characters_iterator];
			USART_putString(character); // Se escribe en pantalla lo que se esta recibiendo
			characters_iterator++;
		}
		else
		{
			command[characters_iterator] = '\0';
			instruction = strtok(command, " ");
			token = instruction;
			arguments_iterator = 0;
			while (token != NULL)
			{
				token = strtok(NULL, " ");
				if (token != NULL)
				{
					args[arguments_iterator] = token;
					USART_putString("\n\r\n\r token ");
					USART_putString((char *)&arguments_iterator);
					USART_putString(" ");
					USART_putString(args[arguments_iterator]);
					arguments_iterator++;
				}

				if (arguments_iterator > 4)
				{ // Verificar que no se reciban mas de 4 argumentos
					USART_putString("\n\r\n\r Too many arguments\n\r");
					error_flag = 1;
					token = NULL;
				}
			}

			if (error_flag != 1)
			{
				if ((strcmp(instruction, REGISTER_DISPLAY_COMMAND) == 0))
				{
					registerDisplay();
				}
				else if ((strcmp(instruction, REGISTER_MODIFY_COMMAND) == 0))
				{
					registerModify();
				}
				else if ((strcmp(instruction, MEMORY_DISPLAY_COMMAND) == 0))
				{
					memoryDisplay();
				}
				else if ((strcmp(instruction, MEMORY_MODIFY_COMMAND) == 0))
				{
					memoryModify();
				}
				else if ((strcmp(instruction, BLOCK_FILL_COMMAND) == 0))
				{
					// TODO
				}
				else if ((strcmp(instruction, RUN_COMMAND) == 0))
				{
					runCommand();
				}
				else if ((strcmp(instruction, CALL_COMMAND) == 0))
				{
					callCommand();
				}
				else if ((strcmp(instruction, IOMAP_COMMAND) == 0))
				{
					// TODO
				}
				else if ((strcmp(instruction, IOUNMAP_COMMAND) == 0))
				{
					// TODO
				}
				else if ((strcmp(instruction, SEGMENT_OUT_COMMAND) == 0))
				{
					// TODO
				}
				else if ((strcmp(instruction, LCD_PRINT_COMMAND) == 0))
				{
					// TODO
				}
				else
				{
					USART_putString("\n\r\n\r");
					USART_putString(instruction);
				}
			}

			USART_putString("\n\r");
			USART_putString(">> ");
			memset(command, 0, 64); // lipiamos la variable de la linea de comando
			args[0] = " ";
			args[1] = " ";
			args[2] = " ";
			args[3] = " ";
			characters_iterator = 0; // reiniciamos el iterador de la linea de comando
			error_flag = 0;
		}
		USART2->ISR |= USART_ISR_RXNE;
	}
}

void USART_config(uint32_t baudrate)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	  // Clock Enbale GPIOA
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Clock Enbale USART2

	GPIOA->MODER |= (0x02 << 4) | (0x02 << 6); // Alternate Function PA2 & PA15
	GPIOA->AFR[0] |= (0x07 << 8);			   // PA2 as TX2
	GPIOA->AFR[0] |= (0x07 << 12);			   // PA15 as RX2

	USART2->BRR = (uint32_t)(SystemCoreClock / baudrate); // round( 8MHz/115200)
	USART2->CR1 |= USART_CR1_TE + USART_CR1_RE;			  // Habiliar recepcion y transmision
	USART2->CR1 |= USART_CR1_RXNEIE;					  // Interrupci?n recepcion
	USART2->CR1 |= USART_CR1_UE;						  // Habilitar modulo UART (puerto serial)

	NVIC_EnableIRQ(USART2_IRQn);
}

void USART_Send(char c)
{
	while (!(USART2->ISR & USART_ISR_TC))
		;
	USART2->TDR = c;
}

void USART_putString(char *string)
{
	while (*string)
	{
		USART_Send(*string);
		string++;
	}
}

void registerDisplay()
{
	USART_putString("\n\r\n\r Executing Register Display...\n\r\n\r");
	if ((strcmp(args[0], " ") != 0))
	{
		USART_putString("\n\r\n\r Too many arguments\n\r"); // Verificar que no existan argumentos
	}
	else
	{
		loadResgistersContent(registers);
		int i = 0;
		for (i = 0; i < REGISTERS_NUM; i++)
		{
			sprintf(buffer, " R%d = 0x%08x", i, registers[i]);
			USART_putString(buffer);
			USART_putString("\n\r");
		}
	}
	USART_putString("\n\r");
}

void runCommand()
{
	USART_putString("\n\r\n\r Executing Run...\n\r\n\r");
	if ((strcmp(args[1], " ") != 0))
	{
		USART_putString("\n\r\n\r Too many arguments\n\r"); // Verificar que no existan argumentos
	}
	run_addr = strtoul("RD", &endptr, 16);
	sprintf(buffer, " RD en hex = 0x%08x", (int)run_addr);
	USART_putString(buffer);

	USART_putString("\n\r");
}

void registerModify()
{
	USART_putString("\n\r\n\r Executing Register Modify...\n\r\n\r");
	if ((strcmp(args[2], " ") != 0))
	{ // Verificar que no existan mas de 2 argumentos
		USART_putString("\n\r\n\r Too many arguments\n\r");
	}
	else
	{
		register_number = strtok(args[0], "R");
		register_content = strtoul(strtok(args[1], "0x"), &endptr, 16);

		sprintf(buffer, " R%s = 0x%08x", register_number, (int)register_content);
		USART_putString(buffer);

		if ((strcmp(register_number, "0") == 0))
		{
			updateR0(register_content);
		}
		else if ((strcmp(register_number, "1") == 0))
		{
			updateR1(register_content);
		}
		else if ((strcmp(register_number, "2") == 0))
		{
			updateR2(register_content);
		}
		else if ((strcmp(register_number, "3") == 0))
		{
			updateR3(register_content);
		}
		else if ((strcmp(register_number, "4") == 0))
		{
			updateR4(register_content);
		}
		else if ((strcmp(register_number, "5") == 0))
		{
			updateR5(register_content);
		}
		else if ((strcmp(register_number, "6") == 0))
		{
			updateR6(register_content);
		}
		else if ((strcmp(register_number, "7") == 0))
		{
			updateR7(register_content);
		}
		else if ((strcmp(register_number, "8") == 0))
		{
			updateR8(register_content);
		}
		else if ((strcmp(register_number, "9") == 0))
		{
			updateR9(register_content);
		}
		else if ((strcmp(register_number, "10") == 0))
		{
			updateR10(register_content);
		}
		else if ((strcmp(register_number, "11") == 0))
		{
			updateR11(register_content);
		}
		else if ((strcmp(register_number, "12") == 0))
		{
			updateR12(register_content);
		}
		else
		{
			USART_putString("\n\r\n\r Register number invalid\n\r");
		}
	}
	USART_putString("\n\r");
}

void memoryDisplay(void)
{
	USART_putString("\n\r\n\r Executing Memory Display...\n\r\n\r");
	if ((strcmp(args[2], " ") != 0))
	{ // Verificar que no existan mas de 2 argumentos
		USART_putString("\n\r\n\r Too many arguments\n\r");
		return;
	}

	start = args[0];
	end = args[1];

	// Si no se especifican start y end, usar el rango predeterminado
	if ((strcmp(start, " ") == 0) || (strcmp(end, " ") == 0))
	{
		start = DEFAULT_START_ADDRESS;
		end = DEFAULT_END_ADDRESS;
	}

	USART_putString("Start Address: ");
	USART_putString(start);
	USART_putString("\n\r");
	USART_putString("End Address: ");
	USART_putString(end);
	USART_putString("\n\r\n\r");

	// Convertir HEX a sin signo
	start_memory_display = strtoul(start, &endptr, 16);
	end_memory_display = strtoul(end, &endptr, 16);

	// Calcular espacio necesario para guardar el rango de direcciones. Bloques de 4 bytes
	memory_range_blocks = ((end_memory_display - start_memory_display) / 4) + 1;

	// Reservar memoria
	display_memory = (uint32_t *)malloc(sizeof(uint32_t) * memory_range_blocks);

	if (display_memory == NULL)
	{
		USART_putString("Error: Memory allocation failed\n\r");
		return;
	}

	// Llenar el arreglo con datos de la memoria
	memoryDisplayAss(display_memory, start_memory_display, end_memory_display);

	// Print de contenido de la memoria
	uint32_t addr;
	for (addr = start_memory_display; addr <= end_memory_display; addr += 4)
	{
		uint32_t value = *((uint32_t *)addr);

		sprintf(arr_display_memory, "Address 0x%08x: 0x%08x\n\r", addr, value);
		USART_putString(arr_display_memory);
	}

	// Liberar memoria
	free(display_memory);
}

void memoryModify()
{
	USART_putString("\n\r\n\r Executing Memory Modify...\n\r\n\r");

	// Verificar argumentos
	if (args[0] == NULL || args[1] == NULL || args[2] == NULL)
	{
		USART_putString("Arguments Missing. Command: MM addr data [size]\n\r");
		return;
	}

	md_addr = args[0];
	md_data = args[1];
	md_size = args[2];

	// Tomar el contenido de args[2] y transformarlo
	sscanf(md_size, "%u", &size_memory);

	// Verificar el tamaño válido (1, 2 o 4 bytes)
	if (size_memory != 1 && size_memory != 2 && size_memory != 4)
	{
		USART_putString("Incorrect Size. Sizes: 1, 2 o 4 bytes.\n\r");
		return;
	}

	// Obtener la dirección
	address_mm = strtoul(md_addr, &endptr, 16);
	data_mm = strtoul(md_data, &endptr, 16);

	// Send and Return Memory for Assembler
	start_ma = address_mm;
	return_ma = data_mm;

	// Modificar la memoria en ensamblador
	modifyMemoryAss(address_mm, data_mm);
}

void blockFill()
{
	USART_putString("\n\r\n\r Executing Block Fill...\n\r\n\r");

	// Verificar argumentos
	if (args[0] == NULL || args[1] == NULL || args[2] == NULL || args[3] == NULL)
	{
		USART_putString("Arguments Missing. Command: BF start end data [size]\n\r");
		return;
	}

	bf_start = strtoul(args[0], &endptr, 16);
	bf_end = strtoul(args[1], &endptr, 16);
	bf_data = (unsigned char)strtoul(args[2], &endptr, 16);
	bf_size = args[3];

	sscanf(bf_size, "%d", &size_bf);

	// Verificar el tamaño válido (1, 2 o 4 bytes)
	if (size_bf != 1 && size_bf != 2 && size_bf != 4)
	{
		USART_putString("Incorrect Size. Sizes: 1, 2 o 4 bytes.\n\r");
		return;
	}

	// Validar rango
	if (bf_start >= bf_end)
	{
		USART_putString("Invalid Range. start to eq\n\r");
		return;
	}

	// Implementar memset: Llenar bloque de memoria desde inicio a fin
	size_t block_size = bf_end - bf_start;
	memset((void *)bf_start, bf_data, block_size);
}

void callCommand()
{
	USART_putString("\n\r\n\r Executing Call...\n\r\n\r");

	memory_call_address = strtoul(strtok(args[0], "0x"), &endptr, 16);
	;

	sprintf(buffer, " Run command in address = 0x%08x", (int)memory_call_address);
	USART_putString(buffer);

	callAddrAssembler(memory_call_address);

	USART_putString("\n\r");
}
