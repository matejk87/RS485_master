/*
 * stxetx.c
 *
 *  Created on: 10.3.2010
 *      Author: borut
 */
#include "stxetx.h"
#include "crc16.h"
#include "rs232.h"
#include <stdio.h>
#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int stxetx_init(t_stxetx * msg,int len,int com_port)
{
	msg->com_port = com_port;
	msg->state=STXETX_STX;
	msg->maxlen=0;
	msg->data=malloc(len);
	if(msg->data==NULL)return -1;  // error could not allocate memory
	msg->maxlen=len;
	msg->cnt=0;

	return 0;
}

int stxetx_rx_msg(t_stxetx * msg)
{

	int i=10,n;
	int iSport;
	unsigned char sResult[300];
	//PollComport(msg->com_port,sResult, 300); // zavrzi kar je priletelo vmes
	for(i=0;i<1000;i++)
	{
		iSport = PollComport(msg->com_port,sResult, 300);
		if (iSport > 0)
		{
			//printf(" rx %d chars:",iSport);
			for(n=0;n<iSport;n++)
			{
				//printf("r:0x%02x  ",sResult[n]);
				if(stxetx_rx_event(msg,sResult[n])==1)
				{
					//printf(" rxc %04x\n",msg->src);
					usleep(4000);
					return 1;
				}
			}
			//printf("\n");
		}
		usleep(100);
	}
	//printf("\nTimeout");
	return 0;
}


int stxetx_rx_event(t_stxetx * msg,unsigned char rxchar)
{
	//printf("\nr:%02x st:%d",rxchar,msg->state);
	switch(msg->state){
	case STXETX_STX:
			if(rxchar==0x55)msg->state=STXETX_SRC0;
		break;

	case STXETX_SRC0:
		//printf("src0\n");
			msg->src=rxchar&0x00FF;
			msg->crc_calc=0;
			crc16_n(&(msg->crc_calc),&rxchar,1);
			msg->state=STXETX_SRC1;
	break;
	case STXETX_SRC1:
			msg->src|=(((short)rxchar)<<8)&0xFF00;
			crc16_n(&(msg->crc_calc),&rxchar,1);
			msg->state=STXETX_DEST0;
	break;

	case STXETX_DEST0:
			msg->state=STXETX_DEST1;
			crc16_n(&(msg->crc_calc),&rxchar,1);
			msg->dest=rxchar&0x00FF;
		break;
	case STXETX_DEST1:
			msg->state=STXETX_MSG_NUMBER;
			crc16_n(&(msg->crc_calc),&rxchar,1);
			msg->dest|=(((short)rxchar)<<8)&0xFF00;
		break;
	case STXETX_MSG_NUMBER:
			msg->state=STXETX_FUNCT;
			crc16_n(&(msg->crc_calc),&rxchar,1);
			msg->msg_number=rxchar;
		break;
	case STXETX_FUNCT:
			msg->state=STXETX_FLAGS;
			crc16_n(&(msg->crc_calc),&rxchar,1);
			msg->funct=rxchar;
		break;
	case STXETX_FLAGS:
			msg->state=STXETX_DATALEN;
			crc16_n(&(msg->crc_calc),&rxchar,1);
			msg->flags=rxchar;
		break;
	case STXETX_DATALEN:
			crc16_n(&(msg->crc_calc),&rxchar,1);
			msg->datalen=rxchar;
			msg->cnt=0;
			if(msg->datalen)msg->state=STXETX_DATA;
			else msg->state=STXETX_CRC1;
		break;
	case STXETX_DATA:
			msg->data[msg->cnt]=rxchar;
			crc16_n(&(msg->crc_calc),&(msg->data[msg->cnt]),1);
			msg->cnt++;
			if(msg->cnt==msg->datalen){
				msg->state=STXETX_CRC1;
			}
		break;
	case STXETX_CRC1:
			msg->state=STXETX_CRC2;
			msg->crc=rxchar&0xFFUL;
		break;
	case STXETX_CRC2:
			msg->state=STXETX_ETX;
			msg->crc|=(rxchar&0xFFUL)<<8;
		break;
	case STXETX_ETX:
			//printf("etx\n");
			msg->state=STXETX_STX;
			if(msg->src==0){/*printf("\nsrc=0");*/return 0;};
			if(rxchar==ETX){  // ali je etx ok
				if(msg->crc_calc==msg->crc){  // preveri crc
					msg_rx_stats.rx_msg_cnt++; // update stats
					//printf("\nmsg_ok");
					return 1;
				}
				else // CRC ERROR
				{
					printf("\nCRC_ERR");
					msg_rx_stats.rx_crc_err_cnt++; // update stats
					return -2;
				}
			}
			else  // etx error
			{
				msg_rx_stats.rx_etx_err_cnt++;  // update stats
				printf("\netx_err");
				return -1;
			}
		break;
	}

return 0;
}

void (*Putchar)();

int stxetx_make_msg(t_stxetx * msg,unsigned short dest,unsigned char funct,unsigned char flags,unsigned char datalen, unsigned char * data )
{
static char cnt=0;
msg->src=0;
msg->dest=dest;
msg->funct=funct;
msg->flags=flags;
msg->msg_number=cnt++;
msg->datalen=datalen;
msg->data=data;
return 0;
}

int stxetx_tx_data2(t_stxetx * msg,int debug)
{
	unsigned char msgbuff[300];
	unsigned char cval;
		int n,i;
		i=0;
		if(debug)printf(("tx:STX "));
        msgbuff[i++]=STX;
		msg->crc=0;

		cval=msg->src&0xFF;
		crc16_n(&(msg->crc),&cval,1);
//		fputchar(cval);
		msgbuff[i++]=cval;

		cval=(msg->src>>8)&0xFF;
		crc16_n(&(msg->crc),&cval,1);
		//fputchar(cval);
		msgbuff[i++]=cval;

		cval=msg->dest&0xFF;
		crc16_n(&(msg->crc),&cval,1);
		//fputchar(cval);
		msgbuff[i++]=cval;
		cval=(msg->dest>>8)&0xFF;
		crc16_n(&(msg->crc),&cval,1);
		//fputchar(cval);
		msgbuff[i++]=cval;

		//fputchar(msg->msg_number);
		cval=msg->msg_number;
		msgbuff[i++]=cval;
		crc16_n(&(msg->crc),&cval,1);

		//fputchar(msg->funct);
		msgbuff[i++]=msg->funct;
		crc16_n(&(msg->crc),&msg->funct,1);

		//fputchar( msg->flags);
		msgbuff[i++]=msg->flags;
		crc16_n(&(msg->crc),&msg->flags,1);
		//fputchar(msg->datalen);

		msgbuff[i++]=msg->datalen;
		crc16_n(&(msg->crc),&msg->datalen,1);
		for(n=0;n<(msg->datalen);n++)
		{
//			fputchar( msg->data[n]);
		msgbuff[i++]=msg->data[n];
		crc16_n(&(msg->crc),&msg->data[n],1);
		}
		//fputchar(msg->crc&0xff);
		msgbuff[i++]=msg->crc&0xff;
		//fputchar((msg->crc>>8)&0xff);
		msgbuff[i++]=(msg->crc>>8)&0xff;
		//fputchar(ETX);

		msgbuff[i++]=ETX;
		if(debug)printf("datalen:%d\n",msg->datalen);
	/*	printf("\nTX:");
		for(n=0;n<i;n++)
		{
		printf("%02x ",msgbuff[n]);
		}*/
		//fputdata((char*)msgbuff,i);
		SendBuf(msg->com_port,msgbuff,i);
		msg_rx_stats.tx_msg_cnt++; // update stats
		return 0;
}

int stxetx_tx_data(t_stxetx * msg, int(*fputdata)( char * , int))
{
	unsigned char msgbuff[300];
	unsigned char cval;
		int n,i;
		i=0;
		//fputchar(STX);
        msgbuff[i++]=STX;
		msg->crc=0;

		cval=msg->src&0xFF;
		crc16_n(&(msg->crc),&cval,1);
//		fputchar(cval);
		msgbuff[i++]=cval;

		cval=(msg->src>>8)&0xFF;
		crc16_n(&(msg->crc),&cval,1);
		//fputchar(cval);
		msgbuff[i++]=cval;

		cval=msg->dest&0xFF;
		crc16_n(&(msg->crc),&cval,1);
		//fputchar(cval);
		msgbuff[i++]=cval;
		cval=(msg->dest>>8)&0xFF;
		crc16_n(&(msg->crc),&cval,1);
		//fputchar(cval);
		msgbuff[i++]=cval;

		//fputchar(msg->msg_number);
		cval=msg->msg_number;
		msgbuff[i++]=cval;
		crc16_n(&(msg->crc),&cval,1);

		//fputchar(msg->funct);
		msgbuff[i++]=msg->funct;
		crc16_n(&(msg->crc),&msg->funct,1);

		//fputchar( msg->flags);
		msgbuff[i++]=msg->flags;
		crc16_n(&(msg->crc),&msg->flags,1);
		//fputchar(msg->datalen);

		msgbuff[i++]=msg->datalen;
		crc16_n(&(msg->crc),&msg->datalen,1);
		for(n=0;n<(msg->datalen);n++)
		{
//			fputchar( msg->data[n]);
		msgbuff[i++]=msg->data[n];
		crc16_n(&(msg->crc),&msg->data[n],1);
		}
		//fputchar(msg->crc&0xff);
		msgbuff[i++]=msg->crc&0xff;
		//fputchar((msg->crc>>8)&0xff);
		msgbuff[i++]=(msg->crc>>8)&0xff;
		//fputchar(ETX);

		msgbuff[i++]=ETX;
//		printf("\nTX:");
//		for(n=0;n<i;n++)
//		{
//		printf("%02x ",msgbuff[n]);
//		}
		fputdata((char*)msgbuff,i);
		msg_rx_stats.tx_msg_cnt++; // update stats
		return 0;
}


int mybus_put_long(unsigned char * tx_data, unsigned long data)
{
  int tx_len = 0;
  tx_data[tx_len++] = data & 0xff;
  tx_data[tx_len++] = (data >> 8) & 0xff;
  tx_data[tx_len++] = (data >> 16) & 0xff;
  tx_data[tx_len++] = (data >> 24) & 0xff;
  return tx_len;
}

int mybus_put_short(unsigned char * tx_data, unsigned short data)
{
  int tx_len = 0;
  tx_data[tx_len++] = data & 0xff;
  tx_data[tx_len++] = (data >> 8) & 0xff;
  return tx_len;
}

int mybus_put_char(unsigned char * tx_data, unsigned char data)
{
  int tx_len = 0;
  tx_data[tx_len++] = data & 0xff;
  return tx_len;
}


unsigned long mybus_get(unsigned char * rx_data,int type)
{
  unsigned long tmp=0;
  tmp = rx_data[0];
  if(type == CHAR_LEN)return tmp;
  tmp |= (rx_data[1]<<8);
  if(type == SHORT_LEN)return tmp;
  tmp |= (rx_data[2]<<16);
  tmp |= (rx_data[3]<<24);
  return tmp;
}


