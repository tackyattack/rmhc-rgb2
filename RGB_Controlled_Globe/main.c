#include <msp430.h> 

/*
 * main.c
 */

#define STRING_LENGTH 16
#define ELEMENT_ADDRESS 223

//____VARIABLES___
unsigned char frameBuffer[15*STRING_LENGTH];
unsigned char redBuffer[STRING_LENGTH];
unsigned char greenBuffer[STRING_LENGTH];
unsigned char blueBuffer[STRING_LENGTH];
//________________

unsigned char i=0;
unsigned int ii=0;
unsigned char RGB_Counter=0;
unsigned int r=0;
unsigned int g=0;
unsigned int b=0;
unsigned char address=0;
unsigned int dataSize=0;
unsigned char data=0;

//____FUNCTIONS____
void updateFrame();
void decodeData();
//_________________

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_12MHZ;                    // Set range
    DCOCTL = CALDCO_12MHZ;                     // Set DCO step + modulation */
    //__________SPI___________________________________________________
    P1DIR |= BIT0 + BIT5;                     //
    P1SEL = BIT1 + BIT2 + BIT4;
    P1SEL2 = BIT1 + BIT2 + BIT4;
    UCA0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC;  // 3-pin, 8-bit SPI master
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 |= 0x04;                          // /4
    UCA0BR1 = 0;                              //
    UCA0MCTL = 0;                             // No modulation
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
     //________________________________________________________________

    P2DIR &= ~(BIT5);//HIGH
    P2DIR &= ~(BIT4);//LOW

    while(1){
    	if ((P2IN&BIT4)&&(P2IN&BIT5)){
    		decodeData();
    		updateFrame();
    	}

    }


}

void decodeData() {
	address=0;
	data=0;
	dataSize=0;
	r=0;
	g=0;
	b=0;
	RGB_Counter=0;

	while( ((P2IN&BIT4)!=0) && ((P2IN&BIT5)!=0) );//wait for data start (double LOW)

	//____ADDRESS____
	for(i=0;i<8;i++){
		while( ((P2IN&BIT4)==0) && ((P2IN&BIT5)==0) );//wait for next bit ready (either HIGH or LOW to turn on)
		if(P2IN&BIT5)address|=(1<<i);//HIGH
		if(P2IN&BIT4)address&=~(1<<i);//LOW
		while( ((P2IN&BIT4)==BIT4) || ((P2IN&BIT5)==BIT5) );//wait for next bit ready (while both HIGH and LOW pins are still active)
	}
	//_______________

	//____DATA SIZE____
	for(i=0;i<16;i++){
		while( ((P2IN&BIT4)==0) && ((P2IN&BIT5)==0) );//wait for next bit ready (either HIGH or LOW to turn on)
		if(P2IN&BIT5)dataSize|=(1<<i);//HIGH
		if(P2IN&BIT4)dataSize&=~(1<<i);//LOW
		while( ((P2IN&BIT4)==BIT4) || ((P2IN&BIT5)==BIT5) );//wait for next bit ready (while both HIGH and LOW pins are still active)
	}
	//_________________

	if (address == ELEMENT_ADDRESS){
		//____DATA____
		for(ii=0;ii<dataSize;ii++){
			for(i=0;i<8;i++){
				while( ((P2IN&BIT4)==0) && ((P2IN&BIT5)==0) );//wait for next bit ready (either HIGH or LOW to turn on)
				if(P2IN&BIT5)data|=(1<<i);//HIGH
				if(P2IN&BIT4)data&=~(1<<i);//LOW
				while( ((P2IN&BIT4)==BIT4) || ((P2IN&BIT5)==BIT5) );//wait for next bit ready (while both HIGH and LOW pins are still active)
				}
			switch(RGB_Counter){
			case 0:
				redBuffer[r]=data;
				r++;
				RGB_Counter=1;
				break;
			case 1:
				greenBuffer[g]=data;
				g++;
				RGB_Counter=2;
				break;
			case 2:
				blueBuffer[b]=data;
				b++;
				RGB_Counter=0;
				break;
			}
		}
		//____________

	}else{
		//____DATA____
		for(ii=0;ii<dataSize;ii++){
			for(i=0;i<8;i++){
				while( ((P2IN&BIT4)==0) && ((P2IN&BIT5)==0) );//wait for bit start (either to go HIGH)
				if(P2IN&BIT5)data|=(1<<i);//HIGH
				if(P2IN&BIT4)data&=~(1<<i);//LOW
				while( ((P2IN&BIT4)==BIT4) || ((P2IN&BIT5)==BIT5) );//wait for next bit ready (while both HIGH and LOW pins are still active)
				}
		}
		//___________
	}
}

void updateFrame() {

	//SPI MHz: 3Mhz

	//Bit pattern for 1: 0b11110
	//Bit pattern for 0: 0b10000
	//15 bytes total
	//GREEN/RED/BLUE

	unsigned char bits[15];
	unsigned char c=0;//color counter
	unsigned int f=0;//frame counter
	unsigned int i=0;

	for (c=0;c<STRING_LENGTH;c++){

	//______________GREEN___________________

	//byte 1 of 5
	if ((greenBuffer[c]&(1<<7)))bits[0]  =0b11110000;
	if (!(greenBuffer[c]&(1<<7)))bits[0] =0b10000000;
	if ((greenBuffer[c]&(1<<6)))bits[0] |=0b00000111;//xxxxx
	if (!(greenBuffer[c]&(1<<6)))bits[0]|=0b00000100;//xxxxx
	//----------

	//byte 2 of 5
	if ((greenBuffer[c]&(1<<6)))bits[1]  =0b10000000;
	if (!(greenBuffer[c]&(1<<6)))bits[1] =0b00000000;
	if ((greenBuffer[c]&(1<<5)))bits[1] |=0b00111100;//xx
	if (!(greenBuffer[c]&(1<<5)))bits[1]|=0b00100000;//xx
	if ((greenBuffer[c]&(1<<4)))bits[1] |=0b00000001;//xxxxxxx
	if (!(greenBuffer[c]&(1<<4)))bits[1]|=0b00000001;//xxxxxxx
	//----------

	//byte 3 of 5
	if ((greenBuffer[c]&(1<<4)))bits[2]  =0b11100000;
	if (!(greenBuffer[c]&(1<<4)))bits[2] =0b00000000;
	if ((greenBuffer[c]&(1<<3)))bits[2] |=0b00001111;//xxxx
	if (!(greenBuffer[c]&(1<<3)))bits[2]|=0b00001000;//xxxx
	//----------

	//byte 4 of 5
	if ((greenBuffer[c]&(1<<3)))bits[3]  =0b00000000;
	if (!(greenBuffer[c]&(1<<3)))bits[3] =0b00000000;
	if ((greenBuffer[c]&(1<<2)))bits[3] |=0b01111000;//x
	if (!(greenBuffer[c]&(1<<2)))bits[3]|=0b01000000;//x
	if ((greenBuffer[c]&(1<<1)))bits[3] |=0b00000011;//xxxxxx
	if (!(greenBuffer[c]&(1<<1)))bits[3]|=0b00000010;//xxxxxx
	//----------

	//byte 5 of 5
	if ((greenBuffer[c]&(1<<1)))bits[4]  =0b11000000;
	if (!(greenBuffer[c]&(1<<1)))bits[4] =0b00000000;
	if ((greenBuffer[c]&(1<<0)))bits[4] |=0b00011110;//---
	if (!(greenBuffer[c]&(1<<0)))bits[4]|=0b00010000;//---
	//----------

	//__________________________________________


	//______________RED___________________

	//byte 1 of 5
	if ((redBuffer[c]&(1<<7)))bits[5]  =0b11110000;
	if (!(redBuffer[c]&(1<<7)))bits[5] =0b10000000;
	if ((redBuffer[c]&(1<<6)))bits[5] |=0b00000111;//xxxxx
	if (!(redBuffer[c]&(1<<6)))bits[5]|=0b00000100;//xxxxx
	//----------

	//byte 2 of 5
	if ((redBuffer[c]&(1<<6)))bits[6]  =0b10000000;
	if (!(redBuffer[c]&(1<<6)))bits[6] =0b00000000;
	if ((redBuffer[c]&(1<<5)))bits[6] |=0b00111100;//xx
	if (!(redBuffer[c]&(1<<5)))bits[6]|=0b00100000;//xx
	if ((redBuffer[c]&(1<<4)))bits[6] |=0b00000001;//xxxxxxx
	if (!(redBuffer[c]&(1<<4)))bits[6]|=0b00000001;//xxxxxxx
	//----------

	//byte 3 of 5
	if ((redBuffer[c]&(1<<4)))bits[7]  =0b11100000;
	if (!(redBuffer[c]&(1<<4)))bits[7] =0b00000000;
	if ((redBuffer[c]&(1<<3)))bits[7] |=0b00001111;//xxxx
	if (!(redBuffer[c]&(1<<3)))bits[7]|=0b00001000;//xxxx
	//----------

	//byte 4 of 5
	if ((redBuffer[c]&(1<<3)))bits[8]  =0b00000000;
	if (!(redBuffer[c]&(1<<3)))bits[8] =0b00000000;
	if ((redBuffer[c]&(1<<2)))bits[8] |=0b01111000;//x
	if (!(redBuffer[c]&(1<<2)))bits[8]|=0b01000000;//x
	if ((redBuffer[c]&(1<<1)))bits[8] |=0b00000011;//xxxxxx
	if (!(redBuffer[c]&(1<<1)))bits[8]|=0b00000010;//xxxxxx
	//----------

	//byte 5 of 5
	if ((redBuffer[c]&(1<<1)))bits[9]  =0b11000000;
	if (!(redBuffer[c]&(1<<1)))bits[9] =0b00000000;
	if ((redBuffer[c]&(1<<0)))bits[9] |=0b00011110;//---
	if (!(redBuffer[c]&(1<<0)))bits[9]|=0b00010000;//---
	//----------

	//__________________________________________


	//______________BLUE___________________

	//byte 1 of 5
	if ((blueBuffer[c]&(1<<7)))bits[10]  =0b11110000;
	if (!(blueBuffer[c]&(1<<7)))bits[10] =0b10000000;
	if ((blueBuffer[c]&(1<<6)))bits[10] |=0b00000111;//xxxxx
	if (!(blueBuffer[c]&(1<<6)))bits[10]|=0b00000100;//xxxxx
	//----------

	//byte 2 of 5
	if ((blueBuffer[c]&(1<<6)))bits[11]  =0b10000000;
	if (!(blueBuffer[c]&(1<<6)))bits[11] =0b00000000;
	if ((blueBuffer[c]&(1<<5)))bits[11] |=0b00111100;//xx
	if (!(blueBuffer[c]&(1<<5)))bits[11]|=0b00100000;//xx
	if ((blueBuffer[c]&(1<<4)))bits[11] |=0b00000001;//xxxxxxx
	if (!(blueBuffer[c]&(1<<4)))bits[11]|=0b00000001;//xxxxxxx
	//----------

	//byte 3 of 5
	if ((blueBuffer[c]&(1<<4)))bits[12]  =0b11100000;
	if (!(blueBuffer[c]&(1<<4)))bits[12] =0b00000000;
	if ((blueBuffer[c]&(1<<3)))bits[12] |=0b00001111;//xxxx
	if (!(blueBuffer[c]&(1<<3)))bits[12]|=0b00001000;//xxxx
	//----------

	//byte 4 of 5
	if ((blueBuffer[c]&(1<<3)))bits[13]  =0b00000000;
	if (!(blueBuffer[c]&(1<<3)))bits[13] =0b00000000;
	if ((blueBuffer[c]&(1<<2)))bits[13] |=0b01111000;//x
	if (!(blueBuffer[c]&(1<<2)))bits[13]|=0b01000000;//x
	if ((blueBuffer[c]&(1<<1)))bits[13] |=0b00000011;//xxxxxx
	if (!(blueBuffer[c]&(1<<1)))bits[13]|=0b00000010;//xxxxxx
	//----------

	//byte 5 of 5
	if ((blueBuffer[c]&(1<<1)))bits[14]  =0b11000000;
	if (!(blueBuffer[c]&(1<<1)))bits[14] =0b00000000;
	if ((blueBuffer[c]&(1<<0)))bits[14] |=0b00011110;//---
	if (!(blueBuffer[c]&(1<<0)))bits[14]|=0b00010000;//---
	//----------

	//__________________________________________

	for (i=0;i<15;i++){
		frameBuffer[f]=bits[i];
		f++;
	}


	}//end of color counter



	for(f=0;f<(15*STRING_LENGTH);f++){
		UCA0TXBUF = frameBuffer[f];
		while (!(IFG2 & UCA0TXIFG));
	}
}
