/*
 * serial_master.h
 *
 *  Created on: 3. jul. 2012
 *      Author: Matej
 */

#ifndef SERIAL_MASTER_H_
#define SERIAL_MASTER_H_


#define RS485_ADDR_DVD 0x2000



#define STX 			0x55
#define ETX 			0xAA
#define PING_FUNCT  	0x00
#define ACK_FUNCT  	0x00
#define PING_BL_FUNCT	 0xf1
#define GET_APP_VERSION_FUNCT 	0xf2
#define PROGRAM_DATA_FUNCT 		0xf3
#define GET_APP_OFFSET_FUNCT 	0xf4
#define GET_CRC_FUNCT	 		0xf5
#define EXIT_BL_FUNCT	 		0xf6
#define ENTER_BL_FUNCT 			0xf0
#define GET_VERSION_FUNCT 		0x7f
/*
typedef struct
{
	unsigned short addr;

}rs485_device;*/

int serial_master_init(int port_n);

int MsgPing(int slave_addr);
int MsgBLPing(int slave_addr);
int MsgExitBL(int slave_addr);
int MsgEnterBL(int slave_addr);
int MsgGetImgCRC(int slave_addr,unsigned short *img_crc);
int MsgGetAppOffset(int slave_addr,int *offset);
int MsgGetVersion(int slave_addr,char *version);
int MsgGetAppVersion(int slave_addr,char *version);
int MsgProgramData(int slave_addr,unsigned char *ProgramData,int ProgramDataOffset,int ProgramDataLen,int AppAddr);

#endif /* SERIAL_MASTER_H_ */
