#include <msp430.h> 

/*
 * main.c
 */

void sendByte(unsigned char data);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

     UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
     UCSCTL4 |= SELA_2;                        // Set ACLK = REFO
     __bis_SR_register(SCG0);                  // Disable the FLL control loop
     UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
     UCSCTL1 = DCORSEL_5;                      // Select DCO range 24MHz operation
     UCSCTL2 = FLLD_1 + 374;                // Set DCO Multiplier for 12MHz
                                               // (N + 1) * FLLRef = Fdco
                                               // (374 + 1) * 32768 = 12MHz
                                               // Set FLL Div = fDCOCLK/2
     __bic_SR_register(SCG0);                  // Enable the FLL control loop

    P3DIR |= 0b10000000; //Set the dual data lines 3.7=high bit
    P4DIR |= 0b00000001; //Set the dual data lines 4.0=low bit
    P8DIR |= 0b00000100; //Set the select line

    while(1){
    	unsigned int i=0;

    	for (i=0;i<800;i++){
    		sendByte(0xef);
    	}
    	P8OUT ^= (0b00000100);

//    	 __delay_cycles(1000000);
//    	 __delay_cycles(1000000);
//    	 __delay_cycles(1000000);

    }
	
}


void sendByte(unsigned char data) {
	unsigned char i=0;
//	P8OUT |= 0b00000100; possible enable line
	for (i=0;i<8;i++){
		if ( data & (1<<i) ){
			P3OUT |= 0b10000000;
		}else{
			P4OUT |= 0b00000001;
		}
		//__delay_cycles(10000);
		P3OUT &= ~(0b10000000);
		P4OUT &= ~(0b00000001);
		//__delay_cycles(10000);
	}
//	P8OUT &= ~(0b00000100);

}//Remember that dual high = packet start/end

