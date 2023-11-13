#include "stm32f3xx.h"                  // Device header

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char buffer[64];
char error_flag = 0;

// USART con interrupciones 
volatile char data;
void USART_config(uint32_t baudrate);
void USART_Send(char c);
void USART_putString(char * string);
char command[64];
char *character;
char *instruction;
char *args[] = {" ", " ", " ", " "};
char *token;
int characters_iterator = 0;

// Register Display
void registerDisplay();
extern void loadResgistersContent(uint32_t *registers);
#define REGISTERS_NUM 15
static uint32_t registers[REGISTERS_NUM];	

// Register Modify
void registerModify();
char *register_modify;
char *register_content;
extern void updateLoadResgister(int register_modify, uint32_t register_content);

int main(void){
	
	USART_config(115200);
	
	USART_putString("\n\r               Microprocesadores 2023                  \n\r");
	USART_putString("                 Programa Monitor                  \n\r");
	USART_putString("                   Victor Amado                  \n\r");
	USART_putString("                  Cruz Ambrocio                  \n\r");
	USART_putString("                 Christian Osorio                  \n\r");
	USART_putString("                  Celeste Batres                  \n\r\n\r");
	USART_putString(">> ");
	
 	while(1){
	}
}


	
void USART2_IRQHandler(void){
	if (USART2->ISR & USART_ISR_RXNE) {
	
		data = USART2->RDR;
		if (data != '\r') {// Esperamos hasta obtener un enter es decir una linea
			command[characters_iterator] = USART2->RDR;
			character = &command[characters_iterator];
			USART_putString( character ); // Se escribe en pantalla lo que se esta recibiendo
			characters_iterator++;
		} else {
			command[characters_iterator] = '\0';
			instruction = strtok(command, " ");
			token = instruction;
			int arguments_iterator = 0;
			while( token != NULL) {
				token = strtok(NULL, " ");
				if (token != NULL) {
					args[arguments_iterator] = token;
					USART_putString("\n\r\n\r token ");
					USART_putString((char *) &arguments_iterator);
					USART_putString(" ");
					USART_putString(token);
					arguments_iterator++;
				}
				
				if (arguments_iterator>4){ // Verificar que no se reciban mas de 4 argumentos
					USART_putString("\n\r\n\r Too many arguments\n\r");
					error_flag = 1;
					token = NULL;
				}
			}
			
			
			if (error_flag!=1){
				if ((strcmp(instruction, "RD") == 0)) {
					registerDisplay();
				} else if ((strcmp(instruction, "RM") == 0)) {
					registerModify();
				} else {
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
			characters_iterator=0; // reiniciamos el iterador de la linea de comando
			error_flag = 0;
		}
		USART2->ISR |= USART_ISR_RXNE;
	}
}


void USART_config(uint32_t baudrate){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;						//Clock Enbale GPIOA
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					//Clock Enbale USART2

	GPIOA->MODER |= (0x02<<4) | (0x02<<6);				//Alternate Function PA2 & PA15
	GPIOA->AFR[0] |= (0x07<<8);									  //PA2 as TX2
	GPIOA->AFR[0] |= (0x07<<12);									//PA15 as RX2
	
	USART2->BRR = (uint32_t)(SystemCoreClock/baudrate);  //round( 8MHz/115200)
	USART2->CR1 |= USART_CR1_TE + USART_CR1_RE;					// Habiliar recepcion y transmision
	USART2->CR1 |= USART_CR1_RXNEIE;										// Interrupci?n recepcion
	USART2->CR1 |= USART_CR1_UE;												// Habilitar modulo UART (puerto serial)
	
	NVIC_EnableIRQ(USART2_IRQn);
}

void USART_Send(char c){
	while(!(USART2->ISR & USART_ISR_TC));
	USART2->TDR = c;
}

void USART_putString(char * string){
	while(*string){
		USART_Send(*string);
		string++;
	}
}

void registerDisplay(){
	USART_putString("\n\r\n\r Executing Register Display...\n\r\n\r");
	if ((strcmp(args[0], " ") != 0)) {
		USART_putString("\n\r\n\r Too many arguments\n\r"); // Verificar que no existan argumentos
	} else {
		loadResgistersContent(registers);
		int i = 0;
		for (i = 0; i <= REGISTERS_NUM; i++){
			sprintf(buffer, " R%d = 0x%08x", i, registers[i]);
			USART_putString(buffer);
			USART_putString("\n\r");
		}
	}
	USART_putString("\n\r");
}

void registerModify(){
	USART_putString("\n\r\n\r Executing Register Modify...\n\r\n\r");
	if ((strcmp(args[2], " ") != 0)) { // Verificar que no existan mas de 2 argumentos
		USART_putString("\n\r\n\r Too many arguments\n\r");
	} else {
		register_modify = strtok(args[0], "R");
		register_content = strtok(args[1], "0x");
		sprintf(buffer, " R%s = 0x%s", register_modify, register_content);
		//sprintf(buffer, " R%s = 0x %08x", register_modify, register_content);
		USART_putString(buffer);
		
		
		
		loadResgistersContent(registers);
		sprintf(buffer, " R%s = 0x%08x", register_modify, registers[(int) register_modify]);
		USART_putString(buffer);
		
		/*
		register_modify = (uint32_t) args[0];
		updateLoadResgister(register_modify, register_content);
		sprintf(buffer, " %s = 0x%08x", args[0], register_content);
		USART_putString(buffer);
		
		*/
		
		
	}
	USART_putString("\n\r");
}

