/*
 * SD.h
 *
 *  Created on: Dec 10, 2014
 *      Author: Henry Bergin
 */

#ifndef SD_H_
#define SD_H_

#define CS_ENABLE() (P6OUT &= ~BIT0)
#define CS_DISABLE() (P6OUT |= BIT0)

void SPI_init();
char SPI_write(char byte);
char SD_init();
void SD_openFile(unsigned char *filename);
void SD_readFile(unsigned long sector, unsigned char *buffer);
char SD_command(unsigned char cmd, unsigned long arg,
                unsigned char crc, unsigned char read);
void SD_read(unsigned long sector, unsigned char * buffer,
             unsigned short len);
//-------------
unsigned char fileBuffer[512];
unsigned char dataBuffer[512];
unsigned char FAT[512];
typedef struct {
    unsigned char filename[8];
    unsigned char ext[3];
    unsigned char attributes;
    unsigned char reserved[10];
    unsigned short modify_time;
    unsigned short modify_date;
    unsigned short starting_cluster;
    unsigned long file_size;
}Fat16Entry;
Fat16Entry SD_File[15];
unsigned char sectors_per_cluster;
unsigned char openedFile;
unsigned long rootDirectory;
unsigned long currentCluster;
//--------------

#endif /* SD_H_ */
