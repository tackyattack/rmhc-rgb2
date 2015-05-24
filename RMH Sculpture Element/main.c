#include <msp430.h>
#include "driverlib.h"

/*
 * main.c
 */

void sendByte(unsigned char byte);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    CS_setDCOFreq( CS_DCORSEL_1, CS_DCOFSEL_3 );

	
    P6DIR |= 0b00000001;


	while(1) {
		sendByte(255);
		__delay_cycles(1000000);
		__delay_cycles(1000000);
	}
}


void sendByte(unsigned char byte) {
	unsigned char i=0;
	for (i=0;i<8;i++){
		P6OUT |= 0b00000001;
		if ( byte & (1<<i) ){
			__delay_cycles(1); //High = 0.8us
		}else{
			__delay_cycles(1); //Low = 0.4us
		}
		P6OUT &= ~(0b00000001);
		if ( byte & (1<<i) ){
			__delay_cycles(1); //High = 0.45us
		}else{
			__delay_cycles(1); //Low = 0.85us
		}

	}

	P6OUT &= ~(0b00000001);
	__delay_cycles(10000);
}
