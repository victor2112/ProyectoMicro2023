#include "stm32f3xx.h" // Device header

#include <string.h>
#include <stdio.h>
#define _OPEN_SYS_ITOA_EXT
#include <stdlib.h>
#include <math.h>
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
unsigned int size_memory = 4;
unsigned long address_mm;
unsigned long data_mm;

// RUN
void runCommand();
void I2C_Write(char slave, char addr, char data);
void I2C_Read(char slave, char addr, char *data);
unsigned long memory_run_address;

// CALL
void callCommand();
unsigned long memory_call_address;

// SEGMENT OUT
#define SLAVE_ADDR 0x40
void I2C_config();
char data_i2c;
uint8_t ticks;
void segmentOut();
char *intNumber = "0";
char *decimalNumber = "0";
char hexNumberDisplay(int num);
char hexDigitDisplay(int num);
char *digitDisplay = "0";
int digitN = 0;
int segmentFlag = 0;
int intNumberi = 0;
int decimalNumberi = 0;
int intNumberINT = 0;
int decimalNumberINT = 0;

// Block Fill
void blockFill();
static char *bf_start;
static char *bf_end;
static char *bf_data;
static char *bf_size;
int size_bf = 1;
unsigned long start_bf;
unsigned long end_bf;
unsigned long data_bf;
static size_t block_size;

int main(void)
{

	USART_config(115200);
	//I2C_config();

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
					// USART_putString("\n\r\n\r token ");
					// USART_putString((char *)&arguments_iterator);
					// USART_putString(" ");
					// USART_putString(args[arguments_iterator]);
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
					blockFill();
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
					segmentOut();
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
	if ((strcmp(args[0], " ") == 0) || (strcmp(args[1], " ") == 0) || (strcmp(args[2], " ") == 0))
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

	USART_putString("\n\r\n\r Memory Modify OK...\n\r\n\r");
}

void blockFill() 
{
	USART_putString("\n\r\n\r Executing Block Fill...\n\r\n\r");

	// Verificar argumentos
	if ((strcmp(args[0], " ") == 0) || (strcmp(args[1], " ") == 0) || (strcmp(args[2], " ") == 0) || (strcmp(args[3], " ") == 0)) {
			USART_putString("Arguments Missing. Command: BF start end data [size]\n\r");
			return;
	}

	bf_start = args[0];
	bf_end = args[1];
	bf_data = args[2];
	bf_size = args[3];
	
	// Tomar el contenido de args[3] y transformarlo
	sscanf(bf_size, "%u", &size_bf);

	// Verificar el tamaño válido (1, 2 o 4 bytes)
	if (size_bf != 1 && size_bf != 2 && size_bf != 4) {
			USART_putString("Incorrect Size. Sizes: 1, 2 o 4 bytes.\n\r");
			return;
	}

	// Obtener la dirección
	start_bf = strtoul(bf_start, &endptr, 16);
	end_bf = strtoul(bf_end, &endptr, 16);
	data_bf = strtoul(bf_data, &endptr, 16);
	
	// Validar rango
	if (start_bf >= end_bf) {
			USART_putString("Invalid Range. Start > End \n\r");
			return;
	}
	
	// Convertir bf_data a int para el memset
	int data_value = strtol(bf_data, NULL, 16);
	
	// Llenar bloque de memoria desde inicio a fin con memset
	block_size = end_bf - start_bf;
	memset((void *)start_bf, data_value, block_size);

	USART_putString("\n\r\n\r Block Fill OK...\n\r\n\r");
}

void runCommand()
{
	USART_putString("\n\r\n\r Executing Run...\n\r\n\r");
	if ((strcmp(args[1], " ") != 0))
	{
		USART_putString("\n\r\n\r Too many arguments\n\r"); // Verificar que no existan argumentos
		return;
	}

	/*run_addr = strtoul("RD", &endptr, 16);
	sprintf(buffer, " RD en hex = 0x%08x", (int)run_addr);
	USART_putString(buffer);
	*/

	memory_run_address = strtoul(strtok(args[0], "0x"), &endptr, 16);
	sprintf(buffer, " Run code in address = 0x%08x", (int)memory_run_address);
	USART_putString(buffer);
	runAddrAssembler(memory_run_address);

	USART_putString("\n\r");
}

void callCommand()
{
	USART_putString("\n\r\n\r Executing Call...\n\r\n\r");
	if ((strcmp(args[1], " ") != 0))
	{
		USART_putString("\n\r\n\r Too many arguments\n\r"); // Verificar que no existan argumentos
		return;
	}

	memory_call_address = strtoul(strtok(args[0], "0x"), &endptr, 16);
	sprintf(buffer, " Call subrutine in address = 0x%08x", (int)memory_call_address);
	USART_putString(buffer);
	callAddrAssembler(memory_call_address);

	USART_putString("\n\r");
}

void I2C_config()
{

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	GPIOB->MODER |= (2 << 12) | (2 << 14);
	GPIOB->AFR[0] |= (4 << 24) | (4 << 28);

	// fbus = 8MHz , fi2c = 100kHz
	I2C1->TIMINGR |= (1 << 28) | (0x4 << 20) | (0x2 << 16) | (0xf << 8) | (0x13 << 0);
	// I2C1->CR1 |= I2C_CR1_TXIE | I2C_CR1_RXIE;
	I2C1->CR1 |= I2C_CR1_PE;

	I2C_Write(SLAVE_ADDR,0x00,0x00); //IODIRA (7-0 AS OUTPUT)
	I2C_Write(SLAVE_ADDR,0x01,0xf0); //IODIRB (7-4 AS INPUTS, 3-0 AS OUTPUT)
	I2C_Write(SLAVE_ADDR,0x0d,0xf0); //PULL UP B (7-4 WITH PULL-UP)
	I2C_Write(SLAVE_ADDR,0x12,0x3f); //PORTA REGISTER (Escribimos cero)
	I2C_Write(SLAVE_ADDR,0x13,0x0f); //PORTB REGISTER (Encendemos Digit 1, 2, 3 y 4)
	
	SysTick_Config(SystemCoreClock/1000);

}


int getDigit(int num, int position) {
    int pow10 = 1;
	int length = 1 + (int)log10(num);
	int i = length - 1;
	
    for (i = length -1;  i > position; --i) {
        pow10 *= 10;
    }
    return (num / pow10) % 10;
}

void SysTick_Handler(void)
{
	ticks++;
	
	if(segmentFlag == 1 ){
		return;
	}
	
	I2C_Write(SLAVE_ADDR,0x13,0x00); //PORTB REGISTER (Apagamos todos los Digit)
	if ((strcmp(intNumber, "0") == 0) && (strcmp(decimalNumber, "0") == 0)) // Colocamos 0 en los 4 digitos
	{
		I2C_Write(SLAVE_ADDR,0x12,0x3f); //PORTA REGISTER (Escribimos cero)
		I2C_Write(SLAVE_ADDR,0x13,0x0f); //PORTB REGISTER (Encendemos Digit 1, 2, 3 y 4)
	} else { // Imprimimos en los 4 digitos del display
		if (digitN == 4) {
			digitN = 0;
		}
		

		if (digitN < intNumberi) { // print de parte entera
			
			int value = getDigit(intNumberINT, digitN); 
			I2C_Write(SLAVE_ADDR,0x12,hexNumberDisplay(value)); //PORTA REGISTER (Escribimos el numero codificado)
			
			//I2C_Write(SLAVE_ADDR,0x12,hexNumberDisplay(&intNumber[digitN])); //PORTA REGISTER (Escribimos el numero codificado)
			if (digitN+1 == intNumberi) { // //PORTA REGISTER (encender el punto decimal)
				I2C_Write(SLAVE_ADDR,0x12,hexNumberDisplay(value) | (1<<7)); //PORTA REGISTER (Escribimos el numero codificado con el punto decimal)
			}
		} else if (digitN < (decimalNumberi + intNumberi) ) { // print de parte decimal
			
			int value = getDigit(decimalNumberINT, digitN-intNumberi); 
			I2C_Write(SLAVE_ADDR,0x12, hexNumberDisplay(value)); //PORTA REGISTER (Escribimos el numero codificado)
			
			
			//I2C_Write(SLAVE_ADDR,0x12,hexNumberDisplay(&decimalNumber[digitN - intNumberi])); //PORTA REGISTER (Escribimos el numero codificado)
		} else { // print de ceros para llenar el display
			I2C_Write(SLAVE_ADDR,0x12,0x5b); //PORTA REGISTER (Escribimos cero)
		}
		//strcpy( digitDisplay, "" );
		//digitDisplay = (char *) digitN+1;
		I2C_Write(SLAVE_ADDR,0x13,hexDigitDisplay(digitN+1)); //PORTB REGISTER (Encendemos Digit correspondiente)	
		digitN++;
	}

	// I2C_Write(SLAVE_ADDR,0x12,(ticks%16)); //PORTA REGISTER
	// I2C_Read(SLAVE_ADDR,0x12,&data_i2c);
}


void I2C_Write(char slave, char addr, char data_i2c)
{
	// AutoEND, Reload, 2 bytes
	I2C1->CR2 = 0x00;
	I2C1->CR2 = I2C_CR2_AUTOEND | (2 << 16) | (slave) | I2C_CR2_START;
	while (!(I2C1->ISR & I2C_ISR_TXIS))
		;
	I2C1->TXDR = addr;
	while (!(I2C1->ISR & I2C_ISR_TXIS))
		;
	I2C1->TXDR = data_i2c;
	while (!(I2C1->ISR & I2C_ISR_TXE))
		;
}

void I2C_Read(char slave, char addr, char *data_i2c)
{
	// AutoEND, Reload, 1 bytes
	I2C1->CR2 = 0x00;
	I2C1->CR2 = I2C_CR2_AUTOEND | (1 << 16) | (slave) | I2C_CR2_START;
	while (!(I2C1->ISR & I2C_ISR_TXIS))
		;
	I2C1->TXDR = addr;
	while (!(I2C1->ISR & I2C_ISR_TXE))
		;
	I2C1->CR2 = 0x00;
	I2C1->CR2 |= I2C_CR2_AUTOEND | (1 << 16) | I2C_CR2_RD_WRN | (slave) | I2C_CR2_START;
	while (!(I2C1->ISR & I2C_ISR_RXNE))
		;
	*data_i2c = I2C1->RXDR;
}

void segmentOut(){
	
	segmentFlag = 1;
	USART_putString("\n\r\n\r Executing Segment Out...\n\r\n\r");
	if ((strcmp(args[1], " ") != 0))
	{
		USART_putString("\n\r\n\r Too many arguments\n\r"); // Verificar que no existan argumentos
		return;
	}
	if ((strcmp(args[0], " ") == 0))
	{
		USART_putString("\n\r\n\r Number parameter is mising\n\r"); // Verificar que no existan argumentos
		return;
	}
	
	intNumber = strtok(args[0], ".");
	decimalNumber = strtok(NULL, "");

	if (strlen(intNumber) == 0)
	{
		intNumber = "0";
	}
	if (strlen(decimalNumber) == 0)
	{
		decimalNumber = "0";
	}

	char subtext[4] = "0";
	if (strlen(intNumber) > 4)
	{

		sprintf(buffer, " Decimal number must be less than 10000\n\r");
		USART_putString(buffer);
		return;
	}
	if (strlen(decimalNumber) > (4 - strlen(intNumber)))
	{
		memcpy(subtext, &decimalNumber[0], 4 - strlen(intNumber));
		if (strlen(subtext) == 0)
		{
			decimalNumber = "0";
		}
		else
		{
			decimalNumber = (char *)&subtext;
		}
	}
	
	char *output;
  intNumberINT = strtol(intNumber, &output, 10);
  decimalNumberINT = strtol(decimalNumber, &output, 10);
	
	for (intNumberi = 0; intNumber[intNumberi] != '\0'; ++intNumberi);
	for (decimalNumberi = 0; decimalNumber[decimalNumberi] != '\0'; ++decimalNumberi);

	sprintf(buffer, " Display decimal number %s.%s", intNumber, decimalNumber);
	USART_putString(buffer);
	//sprintf(buffer, " >>>>>>Display decimal number %d.%d", intNumberINT, decimalNumberINT);
	//USART_putString(buffer);
	USART_putString("\n\r");
	segmentFlag = 0;
		
}

char hexNumberDisplay(int num){
	if (num == 0)
	{
		return 0x3f;
	}
	else if (num == 1)
	{
		return 0x06;
	} 
	else if (num == 2)
	{
		return 0x5b;
	} 
	else if (num == 3)
	{
		return 0x4f;
	} 
	else if (num == 4)
	{
		return 0x66;
	} 
	else if (num == 5)
	{
		return 0x6d;
	}
	else if (num == 6)
	{
		return 0x7c;
	} 
	else if (num == 7)
	{
		return 0x07;
	} 
	else if (num == 8)
	{
		return 0x7f;
	}
	else // #9
	{
		return 0x67;
	}
}

char hexDigitDisplay(int num){
	if (num == 1)
	{
		return 0x08;
	}
	else if (num == 2)
	{
		return 0x04;
	} 
	else if (num == 3)
	{
		return 0x02;
	}
	else // digit 4
	{
		return 0x01;
	}
}
