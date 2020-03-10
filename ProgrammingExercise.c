#include "NU32.h"
#include <stdio.h>
#include <math.h>

unsigned int priority = 0; // This defines whether NU32_LED1 or NU32_LED2 is blinking faster
						   // 0 = NU32_LED1, 1 = NU32_LED2
volatile int InterruptFreq; // Initially, Timer2 will interrupt more quickly

char warning_msg[100] = {};

void __ISR(_TIMER_2_VECTOR, IPL6SRS) Timer2ISR(void) {		
	sprintf(warning_msg, "In Interrupt 1\r\n");
	NU32_WriteUART3(warning_msg);
	
	if (priority == 0) {
		LATFINV = 0x1;
	}
	else {
		LATFINV = 0x2;
	}	
	IFS0bits.T2IF = 0; // Clear flag for Timer2
}

void __ISR(_TIMER_4_VECTOR, IPL5SOFT) Timer4ISR(void) {
	sprintf(warning_msg, "In Interrupt 2\r\n");
	NU32_WriteUART3(warning_msg);
	if (priority == 0) {
		LATFINV = 0x2;
	}
	else {
		LATFINV = 0x1;
	}	
	IFS0bits.T4IF = 0; // Clear flag for Timer4
}

int main(void) {
	NU32_Startup(); // Cache on, prefetch on, interrupts on, LED/button init, UART init
	NU32_LED1 = 0;
	NU32_LED2 = 0;
	
	char message[100];
	
	sprintf(message, "Enter a frequency to blink LED2:\r\n");
	NU32_WriteUART3(message);
		
	NU32_ReadUART3(message, 100);
	sscanf(message, "%d", &InterruptFreq);
		
	sprintf(message, "You entered: %d\r\n", InterruptFreq);
	NU32_WriteUART3(message);
	
	__builtin_disable_interrupts();
	
	//INTCON = 0b1<<12;
	
	// Timer2 (Fixed frequency)
	T2CONbits.T32 = 1; // Initialize 32-bit timer
	PR2 = 80000000; // 80,000,000 ticks = (PR1 + 1) * 1 <-- Prescalar
	TMR2 = 0; // Initialize count to 0
	T2CONbits.TCKPS	 = 0; // Prescalar of 1
	T2CONbits.SIDL = 0; // Continue operation when device is in idle mode
	T2CONbits.TGATE = 0;
	T2CONbits.ON = 1; // Turn on Timer2
	IPC3bits.T3IP = 6; // Priority level 6 (SRS)
	IPC3bits.T3IS = 0; // Don't care about the subpriority level
	IFS0bits.T3IF = 0; // Clear flag for Timer2
	IEC0bits.T3IE = 1; // Enable Timer2 interrupt
	
	// Timer4 (User frequency)
	T4CONbits.T32 = 1; // Initialize 32-bit timer
	PR4 = (80000000 / InterruptFreq) - 1; // Calculate the desired frequency
	TMR4 = 0;
	T4CONbits.TCKPS = 0;
	T4CONbits.SIDL = 0;
	T4CONbits.TGATE = 0;
	T4CONbits.ON = 1;
	IPC5bits.T5IP = 5;
	IPC5bits.T5IS = 0;
	IFS0bits.T5IF = 0;
	IEC0bits.T5IE = 1;
	
	__builtin_enable_interrupts();
	
	while(1) {
		sprintf(message, "Enter a frequency to blink LED2:\r\n");
		NU32_WriteUART3(message);
		
		NU32_ReadUART3(message, 100);
		sscanf(message, "%d", &InterruptFreq);
	}
	return 0;
}