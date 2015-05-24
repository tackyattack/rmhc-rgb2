/*
 * SD.c
 *
 *  Created on: Dec 10, 2014
 *      Author: Henry Bergin
 */
#include <msp430.h>
#include "SD.h"

void SPI_init()
{

	P6DIR |= BIT0;//CS 6.0
	CS_DISABLE();

	//__________SPI_B___________________________________________________
	P3SEL |= BIT0+BIT1+BIT2;                       // P3.0,1,2 option select
	UCB0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCB0CTL0 |= UCMST+UCSYNC+UCCKPL+UCMSB;    // 3-pin, 8-bit SPI master
	UCB0CTL1 |= UCSSEL__SMCLK;                     // SMCLK
    UCB0BR0 = 32;            // /16
    UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	//__________________________________________________________________

}

void SD_openFile(unsigned char *filename)
{
	unsigned char i=0;

	for(i=0;i<15;i++){
		if(SD_File[i].filename[0] == filename[0]&&
		   SD_File[i].filename[1] == filename[1]&&
		   SD_File[i].filename[2] == filename[2]&&
		   SD_File[i].filename[3] == filename[3]&&
		   SD_File[i].filename[4] == filename[4]&&
		   SD_File[i].filename[5] == filename[5]&&
		   SD_File[i].filename[6] == filename[6]&&
		   SD_File[i].filename[7] == filename[7]){
			openedFile=i;
		}
	}

}

void SD_readFile(unsigned long sector, unsigned char *buffer)
{
	unsigned int i=0;
	unsigned int FATCluster=0;

	unsigned long sectorToRead=((SD_File[openedFile].starting_cluster-2)*sectors_per_cluster*1.0)+(rootDirectory+sectors_per_cluster);

	if((sector/sectors_per_cluster)>0){
		FATCluster=SD_File[openedFile].starting_cluster;
		for(i=0;i<sector/sectors_per_cluster;i++){
			FATCluster=FAT[FATCluster*2]+FAT[(FATCluster*2)+1];
		}
		sectorToRead=((FATCluster-2)*sectors_per_cluster*1.0)+(rootDirectory+sectors_per_cluster)+(sector-((sector/sectors_per_cluster)*sectors_per_cluster));
	}else{
		sectorToRead=sectorToRead+sector;
	}

	SD_read(sectorToRead,buffer,512);//Read in sector

}

char SPI_write(char byte)
{
	UCB0TXBUF = byte;
	while (!(UCB0IFG&UCRXIFG));
	return UCB0RXBUF;
}

char SD_init()
{
	char i;

    SPI_init();
    __delay_cycles(8000);

    // ]r:10
    CS_DISABLE();
    for(i=0; i<10; i++){
    	SPI_write(0xff); // idle for 1 bytes / 80 clocks
    	__delay_cycles(8000);

    }

    // [0x40 0x00 0x00 0x00 0x00 0x95 r:8] until we get "1"
    for(i=0; i<10 && SD_command(0x40,0x00000000,0x95,8) != 1; i++)
    	__delay_cycles(1600000);//100ms

    if(i == 10) // card did not respond to initialization
        return 1;

    // CMD1 until card comes out of idle, but maximum of 10 times
    for(i=0; i<10 && SD_command(0x41,0x00000000,0xFF,8) != 0; i++)
    	__delay_cycles(1600000);//100ms

    if(i == 10) // card did not come out of idle
        return 2;

    // SET_BLOCKLEN to 512
    SD_command(0x50, 0x00000200, 0xFF, 8);



	//__________SPEED UP SPI___________________________________________________
	UCB0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCB0BR0 = 4;            // /4
    UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	//__________________________________________________________________
	__delay_cycles(1600000);//100ms


	//___Partition Table___
	typedef struct {
	    unsigned char first_byte;
	    unsigned char start_chs[3];
	    unsigned char partition_type;
	    unsigned char end_chs[3];
	    unsigned long start_sector;
	    unsigned long length_sectors;
	}PartitionTable;
	PartitionTable pt[4];//Create 4 partition tables

	SD_read(0,dataBuffer,512);//Read in first sector

	unsigned int pos=0x1BE;
	for(i=0;i<4;i++){
		pt[i].first_byte=dataBuffer[pos];
		pos++;
		pt[i].start_chs[0]=dataBuffer[pos];
		pos++;
		pt[i].start_chs[1]=dataBuffer[pos];
		pos++;
		pt[i].start_chs[2]=dataBuffer[pos];
		pos++;
		pt[i].partition_type=dataBuffer[pos];
		pos++;
		pt[i].end_chs[0]=dataBuffer[pos];
		pos++;
		pt[i].end_chs[1]=dataBuffer[pos];
		pos++;
		pt[i].end_chs[2]=dataBuffer[pos];
		pos++;
		pt[i].start_sector=(dataBuffer[pos]+dataBuffer[pos++]+dataBuffer[pos++]+dataBuffer[pos]);
		pos++;
		pt[i].length_sectors=(dataBuffer[pos]+dataBuffer[pos++]+dataBuffer[pos++]+dataBuffer[pos]);
		pos++;
	}
	//_____________________

	//____Find Valid Partition____
	unsigned char validPt;
	for(i=0;i<4;i++){
		if( ((pt[i].partition_type)==4)||((pt[i].partition_type)==6)||((pt[i].partition_type)==14) ){
			validPt=i;
		}
	}
	//___________________________


	//____Boot Sector_____
	typedef struct {
	    unsigned char jmp[3];
	    char oem[8];
	    unsigned short sector_size;
	    unsigned char sectors_per_cluster;
	    unsigned short reserved_sectors;
	    unsigned char number_of_fats;
	    unsigned short root_dir_entries;
	    unsigned short total_sectors_short; // if zero, later field is used
	    unsigned char media_descriptor;
	    unsigned short fat_size_sectors;
	    unsigned short sectors_per_track;
	    unsigned short number_of_heads;
//	    unsigned long hidden_sectors;
//	    unsigned long total_sectors_long;
//
//	    unsigned char drive_number;
//	    unsigned char current_head;
//	    unsigned char boot_signature;
//	    unsigned long volume_id;
//	    char volume_label[11];
//	    char fs_type[8];
//	    char boot_code[448];
//	    unsigned short boot_sector_signature;
	}Fat16BootSector;
	Fat16BootSector bs;//Create

	SD_read(pt[validPt].start_sector,dataBuffer,512);//Read in boot sector

	pos=0;
	bs.jmp[0]=dataBuffer[pos];
	pos++;
	bs.jmp[1]=dataBuffer[pos];
	pos++;
	bs.jmp[2]=dataBuffer[pos];
	pos++;
	bs.oem[0]=dataBuffer[pos];
	pos++;
	bs.oem[1]=dataBuffer[pos];
	pos++;
	bs.oem[2]=dataBuffer[pos];
	pos++;
	bs.oem[3]=dataBuffer[pos];
	pos++;
	bs.oem[4]=dataBuffer[pos];
	pos++;
	bs.oem[5]=dataBuffer[pos];
	pos++;
	bs.oem[6]=dataBuffer[pos];
	pos++;
	bs.oem[7]=dataBuffer[pos];
	pos++;
	bs.sector_size=((dataBuffer[pos])+dataBuffer[pos++]);
	pos++;
	bs.sectors_per_cluster=dataBuffer[pos];
	pos++;
	bs.reserved_sectors=(dataBuffer[pos]+dataBuffer[pos++]);
	pos++;
	bs.number_of_fats=dataBuffer[pos];
	pos++;
	bs.root_dir_entries=(dataBuffer[pos]+dataBuffer[pos++]);
	pos++;
	bs.total_sectors_short=(dataBuffer[pos]+dataBuffer[pos++]);
	pos++;
	bs.media_descriptor=dataBuffer[pos];
	pos++;
	bs.fat_size_sectors=(dataBuffer[pos]+dataBuffer[pos++]);
	pos++;
	bs.sectors_per_track=(dataBuffer[pos]+dataBuffer[pos++]);
	pos++;
	bs.number_of_heads=(dataBuffer[pos]+dataBuffer[pos++]);

	sectors_per_cluster = bs.sectors_per_cluster;

	//_________________


	SD_read(pt[validPt].start_sector+bs.reserved_sectors,FAT,512);//Read in FAT

	//___ROOT DIRECTORY (15 files)___
	unsigned long rd=0;
	rd=((bs.reserved_sectors + bs.fat_size_sectors * bs.number_of_fats)*512.0)+(512.0*(pt[validPt].start_sector));
	rd=rd/512.0;//byte address to sector address
	rootDirectory=rd;

	SD_read(rd,dataBuffer,512);//Read in root directory

	pos=0;
	for(i=0;i<15;i++){
		SD_File[i].filename[0]=dataBuffer[pos];
		pos++;
		SD_File[i].filename[1]=dataBuffer[pos];
		pos++;
		SD_File[i].filename[2]=dataBuffer[pos];
		pos++;
		SD_File[i].filename[3]=dataBuffer[pos];
		pos++;
		SD_File[i].filename[4]=dataBuffer[pos];
		pos++;
		SD_File[i].filename[5]=dataBuffer[pos];
		pos++;
		SD_File[i].filename[6]=dataBuffer[pos];
		pos++;
		SD_File[i].filename[7]=dataBuffer[pos];
		pos++;
		SD_File[i].ext[0]=dataBuffer[pos];
		pos++;
		SD_File[i].ext[1]=dataBuffer[pos];
		pos++;
		SD_File[i].ext[2]=dataBuffer[pos];
		pos++;
		SD_File[i].attributes=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[0]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[1]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[2]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[3]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[4]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[5]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[6]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[7]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[8]=dataBuffer[pos];
		pos++;
		SD_File[i].reserved[9]=dataBuffer[pos];
		pos++;
		SD_File[i].modify_time=((dataBuffer[pos])+dataBuffer[pos++]);
		pos++;
		SD_File[i].modify_date=((dataBuffer[pos])+dataBuffer[pos++]);
		pos++;
		SD_File[i].starting_cluster=((dataBuffer[pos])+dataBuffer[pos++]);
		pos++;
		SD_File[i].file_size=dataBuffer[pos];
		pos++;
		SD_File[i].file_size+=dataBuffer[pos];
		pos++;
		SD_File[i].file_size+=dataBuffer[pos];
		pos++;
		SD_File[i].file_size+=dataBuffer[pos];
		pos++;
	}

	//___________________________________

    return 0;
}


void SD_read(unsigned long sector, unsigned char * buffer,
             unsigned short len)
{
    unsigned short i = 0;

    CS_ENABLE();
    SPI_write(0x51);
    SPI_write(sector>>15); // sector*512 >> 24
    SPI_write(sector>>7);  // sector*512 >> 16
    SPI_write(sector<<1);  // sector*512 >> 8
    SPI_write(0);          // sector*512
    SPI_write(0xFF);

    for(i=0; i<200 && (SPI_write(0xFF) != 0x00); i++) {} // wait for 0

    for(i=0; i<200 && (SPI_write(0xFF) != 0xFE); i++) {} // wait for data start

    for(i=0; i<len; i++) // read len bytes
        buffer[i] = SPI_write(0xFF);


    // skip checksum
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);
    SPI_write(0xFF);

    CS_DISABLE();
}

char SD_command(unsigned char cmd, unsigned long arg,
                unsigned char crc, unsigned char read)
{
    unsigned char i;
    unsigned char buffer[8];
    unsigned char output=0;


    CS_ENABLE();
    SPI_write(cmd);
    SPI_write(arg>>24);
    SPI_write(arg>>16);
    SPI_write(arg>>8);
    SPI_write(arg);
    SPI_write(crc);

    for(i=0; i<read; i++) buffer[i] = SPI_write(0xFF);

    CS_DISABLE();

    // print out read bytes
    for(i=0; i<read; i++){
    	if(buffer[i] != 0xFF){
    		output=buffer[i];
    	}
    }

    return output;
}
