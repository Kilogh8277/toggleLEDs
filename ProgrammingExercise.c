#include "NU32.h"
#include <stdio.h>

#define INTERRUPTPERIOD_1kHz 80000 // When PBCLK hits 40,000, 1 ms will have passed
#define PR1_PERIOD 312499 // What PR1 must be set to if it is higher priority

unsigned int priority = 0; // This defines whether NU32_LED1 or NU32_LED2 is blinking faster
						   // 0 = NU32_LED1, 1 = NU32_LED2
volatile unsigned int InterruptPeriod; // Initially, Timer2 will interrupt more quickly

void __ISR(_TIMER_1_VECTOR, IPL6SRS) Timer1ISR(void) {
	if (priority == 0) {
		LATFINV = 0x1;
	}
	else {
		LATFINV = 0x2;
	}	
	TMR1 = 0;
}

void __ISR(_CORE_TIMER_VECTOR, IPL5SOFT) CoreTimerISR(void) {
	if (priority == 0) {
		LATFINV = 0x2;
		_CP0_SET_COUNT(0);	
		_CP0_SET_COMPARE(InterruptPeriod);
	}
	else {
		LATFINV = 0x1;
		_CP0_SET_COUNT(0);
		_CP0_SET_COMPARE(INTERRUPTPERIOD_1kHz);
	}	
	// Setup the conditions for interrupt again
	
}

int main(void) {
	NU32_Startup(); // Cache on, prefetch on, interrupts on, LED/button init, UART init
	char message[100];
	
	sprintf(message, "Enter an interrupt period:\r\n");
	NU32_WriteUART3(message);
		
	NU32_ReadUART3(message, 100);
	sscanf(message, "%d", &InterruptPeriod);
		
	sprintf(message, "You entered: %d\r\n", InterruptPeriod);
	NU32_WriteUART3(message);
	
	__builtin_disable_interrupts();
	
	// Timer1 (Fixed frequency)
	PR1 = 312499; // 80,000,000 ticks = (PR1 + 1) * 256 <-- Prescalar
	TMR1 = 0; // Initialize count to 0
	T1CONbits.TCKPS	 = 3; // Prescalar of 256
	T1CONbits.SIDL = 0; // Continue operation when device is in idle mode
	T1CONbits.TGATE = 0;
	T1CONbits.TCS = 0; // Use the PBCLK as input for Timer1
	T1CONbits.ON = 1; // Turn on Timer1
	IPC1bits.T1IP = 6; // Priority level 6 (SRS)
	IPC1bits.T1IS = 0; // Don't care about the subpriority level
	IFS0bits.T1IF = 0; // Clear flag for Timer1
	IEC0bits.T1IE = 1; // Enable Timer1 interrupt
	
	// CoreTimer (User frequency)
	_CP0_SET_COMPARE(InterruptPeriod);
	IEC0 |= 0b1; // Enable the interrupt (Interrupt Enable Controller)
	IFS0 &= 0xFFFFFFFE; // Set the flag status as 0
	IPC0 |= 0b101 << 2; // Set the priority as 5, bits 4:2
	IPC0 |= 0b00; // Set sub-priority as 0, bits 1:0
	
	__builtin_enable_interrupts();
	
	while(1) {
		sprintf(message, "Enter an interrupt period:\r\n");
		NU32_WriteUART3(message);
		
		NU32_ReadUART3(message, 100);
		sscanf(message, "%d", &InterruptPeriod);
		
		sprintf(message, "You entered: %d\r\n", InterruptPeriod);
		NU32_WriteUART3(message);
	}
	return 0;
}