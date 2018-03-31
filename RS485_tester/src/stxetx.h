/*
 * stxetx.h
 *
 *  Created on: 10.3.2010
 *      Author: borut
 */

#ifndef STXETX_H_
#define STXETX_H_

#define CHAR_LEN  1
#define SHORT_LEN 2
#define LONG_LEN  4


#define STX 0x55
#define ETX 0xAA

#define STXETX_STX 0
#define STXETX_SRC0 1
#define STXETX_SRC1 2
#define STXETX_DEST0 3
#define STXETX_DEST1 4
#define STXETX_MSG_NUMBER 5
#define STXETX_FUNCT 6
#define STXETX_FLAGS 7
#define STXETX_DATALEN 8
#define STXETX_DATA	9
#define STXETX_CRC1 10
#define STXETX_CRC2 11
#define STXETX_ETX 12

typedef struct
{
	unsigned long tx_msg_cnt;
	unsigned long rx_msg_cnt;
	unsigned long rx_crc_err_cnt;
	unsigned long rx_etx_err_cnt;
	unsigned long rx_timeout_cnt;
}t_stxetx_stats;

typedef struct
{
	short  addr;
	int state;
	void * data;


}t_stxetx_module;


typedef struct
{
	int com_port;
	char state;
	int cnt;
	int maxlen;
	unsigned short src;
	unsigned short dest;
	unsigned char msg_number;
	unsigned char funct;
	unsigned char flags;
	unsigned char datalen;
	unsigned char * data;
	unsigned short crc;
	unsigned short crc_calc;
	//

} t_stxetx;
t_stxetx_stats msg_rx_stats;


extern t_stxetx_stats stxetx_stats;

int stxetx_init(t_stxetx * msg,int len,int com_port);
int stxetx_rx_event(t_stxetx * dev,unsigned char rxchar);
int stxetx_tx_data(t_stxetx * dev, int(*fputdata)( char * , int));
int stxetx_tx_data2(t_stxetx * msg,int debug);
int stxetx_rx_msg(t_stxetx * msg);
//int stxetx_tx_data(t_stxetx * dev,int(*fputchar)(char));
int stxetx_make_msg(t_stxetx * dev,unsigned short dest,unsigned char funct,unsigned char flags,unsigned char datalen, unsigned char * data );

int mybus_put_long(unsigned char * tx_data, unsigned long data);
int mybus_put_short(unsigned char * tx_data, unsigned short data);
int mybus_put_char(unsigned char * tx_data, unsigned char data);
unsigned long mybus_get(unsigned char * rx_data,int type);

#endif /* STXETX_H_ */
