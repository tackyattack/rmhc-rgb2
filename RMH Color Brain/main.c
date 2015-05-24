#include <msp430.h> 
#include <string.h>
#include "SD.h"

/*
 * main.c
 */
/*DATA STRUCTURE:
 * ----
 * [{223}{255,255,255}{255,255,255}]
 * [{255,001,255}{020,255,255}]
 * [d000001]
 * [{223}{255,255,255}{255,255,255}]
 * [r]
 * ----
 */

#define SPEED 50

char buffer[1000];
void sendData(char address,int size);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    UCSCTL3 = SELREF__REFOCLK;                       // Set DCO FLL reference = REFO
    UCSCTL4 |= SELM__DCOCLKDIV  | SELA__REFOCLK | SELS__DCOCLKDIV;
    UCSCTL5 |= DIVS_1;

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


      SD_init();//Setup SD Card
      SD_openFile("SHOW_1  ");//Open SETUP file  "SHOW_1  "

      __delay_cycles(16000000);

      P1DIR |= BIT4+BIT5;//1.2 is low bit and 1.3 is high bit
      P1OUT &= ~(BIT4+BIT5); //Make sure the data pins are off


      unsigned int cc=0;
      unsigned int i=0;
      unsigned int sector=0;
      unsigned int byte=0;
      unsigned char color=0;
      unsigned char address=0;
      unsigned int delay=0;//ms (up to a minute delay)

//      while(1){
//          for(cc=0;cc<432;cc++){
//          	buffer[cc]=5;
//          	cc++;
//          	buffer[cc]=0;
//          	cc++;
//          	buffer[cc]=0;
//
//          }
//    	  sendData(223,432);
//    	 __delay_cycles(1000000);
//        for(cc=0;cc<432;cc++){
//        	buffer[cc]=0;
//        	cc++;
//        	buffer[cc]=0;
//        	cc++;
//        	buffer[cc]=5;
//        }
//  	  sendData(223,432);
//  	  __delay_cycles(1000000);
//      }


      while(1)
      {


    	  SD_readFile(sector,fileBuffer);

    	  for(;(fileBuffer[byte]!='[');byte++){
    		  //Find start of command
			  if(byte>=512){
				  sector++;
				  SD_readFile(sector,fileBuffer);
				  byte=0;
			  }
    	  }
    	  byte++;
		  if(byte>=512){
			  sector++;
			  SD_readFile(sector,fileBuffer);
			  byte=0;
		  }

    	  if((fileBuffer[byte]=='{')){
    		  byte++;
			  if(byte>=512){
				  sector++;
				  SD_readFile(sector,fileBuffer);
				  byte=0;
			  }

    		  cc=0;
    		  memset(buffer, 0, 1000);
    		  for(byte=byte;(fileBuffer[byte]!=']');byte++){
				  if(byte>=512){
					  sector++;
					  SD_readFile(sector,fileBuffer);
					  byte=0;
				  }
    			  color=0;
    			  if(fileBuffer[byte]!=','){
    				  color+=100*(fileBuffer[byte]-48);
    				  byte++;
    				  if(byte>=512){
    					  sector++;
    					  SD_readFile(sector,fileBuffer);
    					  byte=0;
    				  }
    			  }
    			  if(fileBuffer[byte]!=','){
    				  color+=10*(fileBuffer[byte]-48);
    				  byte++;
    				  if(byte>=512){
    					  sector++;
    					  SD_readFile(sector,fileBuffer);
    					  byte=0;
    				  }
    			  }
        		  if(fileBuffer[byte]!=','){
        			  color+=1*(fileBuffer[byte]-48);
        		  	  byte++;
        			  if(byte>=512){
        				  sector++;
        				  SD_readFile(sector,fileBuffer);
        				  byte=0;
        			  }
        		  }

    			  if(cc==0){
    				  address=color;
    				  cc++;
    			  }else{
    				  buffer[cc-1]=color;
    				  cc++;
    			  }
    			  if(fileBuffer[byte] == '}'){
    				  byte++;//skip separator
        			  if(byte>=512){
        				  sector++;
        				  SD_readFile(sector,fileBuffer);
        				  byte=0;
        			  }
    			  }
    			  if(fileBuffer[byte]==']'){
    				  byte--;
        			  if(byte==0){
        				  sector--;
        				  SD_readFile(sector,fileBuffer);
        				  byte=0;
        			  }
    			  }

    		  }

    		  sendData(address,(cc-1));

    	  }

    	  if((fileBuffer[byte]=='r')){
    		  sector=0;
    		  byte=0;
    	  }

    	  if((fileBuffer[byte]=='d')){
    		  delay=0;
    		  byte++;
    		  if(byte>=512){
    			  sector++;
    			  SD_readFile(sector,fileBuffer);
    			  byte=0;
    		  }
			  if(fileBuffer[byte]!=']'){
				  delay+=100000*(fileBuffer[byte]-48);
				  byte++;
			  }
    		  if(byte>=512){
    			  sector++;
    			  SD_readFile(sector,fileBuffer);
    			  byte=0;
    		  }
			  if(fileBuffer[byte]!=']'){
				  delay+=10000*(fileBuffer[byte]-48);
				  byte++;
			  }
    		  if(byte>=512){
    			  sector++;
    			  SD_readFile(sector,fileBuffer);
    			  byte=0;
    		  }
    		  if(fileBuffer[byte]!=']'){
    			  delay+=1000*(fileBuffer[byte]-48);
    			  byte++;
    		  }
    		  if(byte>=512){
    			  sector++;
    			  SD_readFile(sector,fileBuffer);
    			  byte=0;
    		  }
			  if(fileBuffer[byte]!=']'){
				  delay+=100*(fileBuffer[byte]-48);
				  byte++;
			  }
    		  if(byte>=512){
    			  sector++;
    			  SD_readFile(sector,fileBuffer);
    			  byte=0;
    		  }
			  if(fileBuffer[byte]!=']'){
				  delay+=10*(fileBuffer[byte]-48);
				  byte++;
			  }
    		  if(byte>=512){
    			  sector++;
    			  SD_readFile(sector,fileBuffer);
    			  byte=0;
    		  }
    		  if(fileBuffer[byte]!=']'){
    			  delay+=1*(fileBuffer[byte]-48);
    			  byte++;
    		  }
    		  if(byte>=512){
    			  sector++;
    			  SD_readFile(sector,fileBuffer);
    			  byte=0;
    		  }
    		  for(i=0;i<delay;i++)__delay_cycles(16000);//1ms
    	  }


      }//main while-loop end

}


void sendData(char address,int size)
{
	//{address(8),data size(16),data}
	//1.2=low, 1.3=high
	unsigned char i=0;
	unsigned int ii=0;
	unsigned int dataSize=0;
	//____START TRANSMISSION____
	P1OUT |= (BIT4+BIT5); //Start data transmission
	__delay_cycles(1000); //Give the elements a bit of time to listen up
	P1OUT &= ~(BIT4+BIT5); //Data pins go low. Tranmission begins.
	__delay_cycles(1000); //Wait
	//__________________________

	//____ADDRESS____
	for(i=0;i<8;i++){
		if ( (address&(1<<i)) != 0 ) P1OUT |= BIT5;//HIGH
		if ( (address&(1<<i)) == 0 ) P1OUT |= BIT4;//LOW
		__delay_cycles(SPEED);
		P1OUT &= ~(BIT4+BIT5); //Data pins low.
		__delay_cycles(SPEED);
	}
	//______________

	//____DATA SIZE____
	//dataSize=sizeof(buffer);
	dataSize=size;
	for(i=0;i<16;i++){
		if ( (dataSize&(1<<i)) != 0 ) P1OUT |= BIT5;//HIGH
		if ( (dataSize&(1<<i)) == 0 ) P1OUT |= BIT4;//LOW
		__delay_cycles(SPEED);
		P1OUT &= ~(BIT4+BIT5); //Data pins low.
		__delay_cycles(SPEED);
	}
	//_________________

	//____DATA____
	for(ii=0;ii<dataSize;ii++){
	for(i=0;i<8;i++){
		if ( (buffer[ii]&(1<<i)) != 0 ) P1OUT |= BIT5;//HIGH
		if ( (buffer[ii]&(1<<i)) == 0 ) P1OUT |= BIT4;//LOW
		__delay_cycles(SPEED);
		P1OUT &= ~(BIT4+BIT5); //Data pins low.
		__delay_cycles(SPEED);
	}
	}
	//___________
}
