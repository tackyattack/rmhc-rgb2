#include <msp430.h>

/*
 * main.c
 */
//NOTE: MAX CYCLES DELAY BETWEEN TRANSFERS FOR 200 LED SETUP IS (280000)
//11110_111
//11110_100
//

#define STRING_LENGTH 200 //4x4 (16) LEDs
#define ELEMENT_ADDRESS 108

#define FADE_SPEED 400000
#define FADE_INCREMENT 1

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

    UCSCTL3 = SELREF__REFOCLK;                       // Set DCO FLL reference = REFO
    UCSCTL4 |= SELM__DCOCLKDIV  |
    		SELA__REFOCLK | SELS__DCOCLKDIV;
    UCSCTL5 |= DIVS_2;

    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;                      // Select DCO range 16MHz operation
    UCSCTL2 |= 488;                           // Set DCO Multiplier for 12MHz
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
      do
      {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                                // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
      }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
	


    //__________SPI___________________________________________________
    P3SEL |= BIT3+BIT4;                       // P3.3,4 option select
    P2SEL |= BIT7;                            // P2.7 option select

     UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
     UCA0CTL0 |= UCMST+UCSYNC+UCCKPL+UCMSB;    // 3-pin, 8-bit SPI master
                                               // Clock polarity high, MSB
     UCA0CTL1 |= UCSSEL__SMCLK;                     // SMCLK
     UCA0MCTL = 0;                             // No modulation
     UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
     //________________________________________________________________

     //__________SPI_B___________________________________________________
         P3SEL |= BIT0+BIT1+BIT2;                       // P3.0,1,2 option select

          UCB0CTL1 |= UCSWRST;                      // **Put state machine in reset**
          UCB0CTL0 |= UCMST+UCSYNC+UCCKPL+UCMSB;    // 3-pin, 8-bit SPI master
                                                    // Clock polarity high, MSB
          UCB0CTL1 |= UCSSEL__SMCLK;                     // SMCLK
          UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
     //________________________________________________________________

          unsigned int cc=0;
          unsigned int ccc=0;

          while(1){
        	  //----------------------
        	  for(cc=0;cc<256;cc++){
        		  for(ccc=0;ccc<16;ccc++){
        		  //--
        		  if(redBuffer[ccc]<251) redBuffer[ccc]=redBuffer[ccc]+=FADE_INCREMENT;
        		  if(redBuffer[ccc]>251) redBuffer[ccc]=redBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  if(greenBuffer[ccc]<55) greenBuffer[ccc]=greenBuffer[ccc]+=FADE_INCREMENT;
        		  if(greenBuffer[ccc]>55) greenBuffer[ccc]=greenBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  if(blueBuffer[ccc]<200) blueBuffer[ccc]=blueBuffer[ccc]+=FADE_INCREMENT;
        		  if(blueBuffer[ccc]>200) blueBuffer[ccc]=blueBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  }
        		  updateFrame();
        		  __delay_cycles(FADE_SPEED);
        	  }
        	  //End of color fade
        	  //----------------------
        	  for(cc=0;cc<256;cc++){
        		  for(ccc=0;ccc<16;ccc++){
        		  //--
        		  if(redBuffer[ccc]<255) redBuffer[ccc]=redBuffer[ccc]+=FADE_INCREMENT;
        		  if(redBuffer[ccc]>255) redBuffer[ccc]=redBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  if(greenBuffer[ccc]<54) greenBuffer[ccc]=greenBuffer[ccc]+=FADE_INCREMENT;
        		  if(greenBuffer[ccc]>54) greenBuffer[ccc]=greenBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  if(blueBuffer[ccc]<255) blueBuffer[ccc]=blueBuffer[ccc]+=FADE_INCREMENT;
        		  if(blueBuffer[ccc]>255) blueBuffer[ccc]=blueBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  }
        		  updateFrame();
        		  __delay_cycles(FADE_SPEED);
        	  }
        	  //End of color fade
        	  for(cc=0;cc<256;cc++){
        		  for(ccc=0;ccc<16;ccc++){
        		  //--
        		  if(redBuffer[ccc]<54) redBuffer[ccc]=redBuffer[ccc]+=FADE_INCREMENT;
        		  if(redBuffer[ccc]>54) redBuffer[ccc]=redBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  if(greenBuffer[ccc]<41) greenBuffer[ccc]=greenBuffer[ccc]+=FADE_INCREMENT;
        		  if(greenBuffer[ccc]>41) greenBuffer[ccc]=greenBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  if(blueBuffer[ccc]<255) blueBuffer[ccc]=blueBuffer[ccc]+=FADE_INCREMENT;
        		  if(blueBuffer[ccc]>255) blueBuffer[ccc]=blueBuffer[ccc]-=FADE_INCREMENT;
        		  //--
        		  }
        		  updateFrame();
        		  __delay_cycles(FADE_SPEED);
        	  }
//        	  //End of color fade
//        	  for(cc=0;cc<256;cc++){
//        		  for(ccc=0;ccc<16;ccc++){
//        		  //--
//        		  if(redBuffer[ccc]<54) redBuffer[ccc]=redBuffer[ccc]+=FADE_INCREMENT;
//        		  if(redBuffer[ccc]>54) redBuffer[ccc]=redBuffer[ccc]-=FADE_INCREMENT;
//        		  //--
//        		  if(greenBuffer[ccc]<255) greenBuffer[ccc]=greenBuffer[ccc]+=FADE_INCREMENT;
//        		  if(greenBuffer[ccc]>255) greenBuffer[ccc]=greenBuffer[ccc]-=FADE_INCREMENT;
//        		  //--
//        		  if(blueBuffer[ccc]<255) blueBuffer[ccc]=blueBuffer[ccc]+=FADE_INCREMENT;
//        		  if(blueBuffer[ccc]>255) blueBuffer[ccc]=blueBuffer[ccc]-=FADE_INCREMENT;
//        		  //--
//        		  }
//        		  updateFrame();
//        		  __delay_cycles(FADE_SPEED);
//        	  }
//        	  //End of color fade
          }//end of main while-loop
//Never reached

//          P6DIR &= ~(BIT4);//LOW
//          P6DIR &= ~(BIT5);//HIGH
//
//          while(1){
//        	  if ((P6IN&BIT4)&&(P6IN&BIT5)){
//        		  decodeData();
//        	  }
//          }
}//end of main

//
//void decodeData() {
//	address=0;
//	data=0;
//	dataSize=0;
//	r=0;
//	g=0;
//	b=0;
//	RGB_Counter=0;
//
//	while((P6IN&BIT4)&&(P6IN&BIT5));//while it's still double high
//	while(!(P6IN&BIT4)&&(P6IN&BIT5));//wait for data start (double LOW)
//
//	//____ADDRESS____
//	for(i=0;i<8;i++){
//		while( ((P6IN&BIT4)==0) && ((P6IN&BIT5)==0) );//wait for next bit ready (either HIGH or LOW to turn on)
//		if(P6IN&BIT5)address|=(1<<i);//HIGH
//		if(P6IN&BIT4)address&=~(1<<i);//LOW
//		while( ((P6IN&BIT4)==BIT4) || ((P6IN&BIT5)==BIT5) );//wait for next bit ready (while both HIGH and LOW pins are still active)
//	}
//	//_______________
//
//	//____DATA SIZE____
//	for(i=0;i<16;i++){
//		while( ((P6IN&BIT4)==0) && ((P6IN&BIT5)==0) );//wait for next bit ready (either HIGH or LOW to turn on)
//		if(P6IN&BIT5)dataSize|=(1<<i);//HIGH
//		if(P6IN&BIT4)dataSize&=~(1<<i);//LOW
//		while( ((P6IN&BIT4)==BIT4) || ((P6IN&BIT5)==BIT5) );//wait for next bit ready (while both HIGH and LOW pins are still active)
//	}
//	//_________________
//	if(dataSize>1000)dataSize=0;
//
//	if (address == ELEMENT_ADDRESS){
//		//____DATA____
//		for(ii=0;ii<dataSize;ii++){
//			for(i=0;i<8;i++){
//				while( ((P6IN&BIT4)==0) && ((P6IN&BIT5)==0) );//wait for next bit ready (either HIGH or LOW to turn on)
//				if(P6IN&BIT5)data|=(1<<i);//HIGH
//				if(P6IN&BIT4)data&=~(1<<i);//LOW
//				while( ((P6IN&BIT4)==BIT4) || ((P6IN&BIT5)==BIT5) );//wait for next bit ready (while both HIGH and LOW pins are still active)
//				}
//			switch(RGB_Counter){
//			case 0:
//				redBuffer[r]=data;
//				r++;
//				RGB_Counter=1;
//				break;
//			case 1:
//				greenBuffer[g]=data;
//				g++;
//				RGB_Counter=2;
//				break;
//			case 2:
//				blueBuffer[b]=data;
//				b++;
//				RGB_Counter=0;
//				break;
//			}
//		}
//		//____________
//
//	}else{
//		//____DATA____
//		for(ii=0;ii<dataSize;ii++){
//			for(i=0;i<8;i++){
//				while( ((P6IN&BIT4)==0) && ((P6IN&BIT5)==0) );//wait for bit start (either to go HIGH)
//				if(P6IN&BIT5)data|=(1<<i);//HIGH
//				if(P6IN&BIT4)data&=~(1<<i);//LOW
//				while( ((P6IN&BIT4)==BIT4) || ((P6IN&BIT5)==BIT5) );//wait for next bit ready (while both HIGH and LOW pins are still active)
//				}
//		}
//		//___________
//	}
//	if (address == ELEMENT_ADDRESS) updateFrame(); //update frame only if it matches the address
//}


void updateFrame() {

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
		UCB0TXBUF = frameBuffer[f];
		while (!(UCB0IFG&UCTXIFG));
	}
}
