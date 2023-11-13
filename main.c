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

void USART1_IRQHandler(void){
	if (USART1->ISR & USART_ISR_RXNE) {
	
		data = USART1->RDR;
		if (data != '\r') {// Esperamos hasta obtener un enter es decir una linea
			command[characters_iterator] = USART1->RDR;
			character = &command[characters_iterator];
			USART_putString( character );
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
					USART_putString("\n\r\n\r token");
					USART_putString(token);
					arguments_iterator++;
				}
				
				if (arguments_iterator>4){
					USART_putString("\n\r\n\r Too many arguments\n\r");
					error_flag = 1;
					token = NULL;
				}
			}
			if (error_flag!=1){
				if ((strcmp(instruction, "RD") == 0)) {
					registerDisplay();
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
		USART1->ISR |= USART_ISR_RXNE;
	}
}


void USART_config(uint32_t baudrate){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;						//Clock Enbale GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;					//Clock Enbale USART1

	GPIOA->MODER |= (0x02<<2*9) | (0x02<<2*10);				//Alternate Function PA_9 & PA_10
	//GPIOA->AFR[0] |= (0x07<<8);							//PA_9  as TX2
	GPIOA->AFR[1] |= (0x07<<4) | (0x07<<8) ;				//PA_10 as RX2
	
	USART1->BRR = (uint32_t)(SystemCoreClock/baudrate);  	//round( 8MHz/115200)
	USART1->CR1 |= USART_CR1_TE + USART_CR1_RE;				// Enable reception y transmission
	USART1->CR1 |= USART_CR1_RXNEIE;						// Reception interruption
	USART1->CR1 |= USART_CR1_UE;							// enable port UART (serial port)
	
	NVIC_EnableIRQ(USART1_IRQn);
}

void USART_Send(char c){
	while(!(USART1->ISR & USART_ISR_TC));
	USART1->TDR = c;
}

void USART_putString(char * string){
	while(*string){
		USART_Send(*string);
		string++;
	}
}

void registerDisplay(){
	if ((strcmp(args[0], " ") != 0)) {
		USART_putString("\n\r\n\r Too many arguments\n\r");
	} else {
		USART_putString("\n\r\n\r Executing Register Display...\n\r\n\r");
		loadResgistersContent(registers);
		int i = 0;
		for (i = 0; i <= REGISTERS_NUM; i++){
			sprintf(buffer, " R%d = 0x%08x", i, registers[i]);
			USART_putString(buffer);
			USART_putString("\n\r");
		}
	}
}