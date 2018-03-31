/*
 * serial_master.c
 *
 *  Created on: 3. jul. 2012
 *      Author: Matej
 */

#include "stxetx.h"
#include "serial_master.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

t_stxetx tx_msg,rx_msg;
int serial_master_init(int port_n)
{
	  if(stxetx_init(&tx_msg,300,port_n))return 0;
	  if(stxetx_init(&rx_msg,300,port_n))return 0;
	  return 1;
}

int MsgPing(int slave_addr)
{
	stxetx_make_msg(&tx_msg,slave_addr,PING_FUNCT,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	//Sleep(5);
	if(stxetx_rx_msg(&rx_msg))
	{
		if(rx_msg.funct == PING_FUNCT)return 1;
		else return 0;
	}
	else return 0;
}

int MsgBLPing(int slave_addr)
{
	stxetx_make_msg(&tx_msg,slave_addr,PING_BL_FUNCT,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	//Sleep(1);
	if(stxetx_rx_msg(&rx_msg))
	{
		return 1;
	}
	return 0;
}

int MsgExitBL(int slave_addr)
{
	stxetx_make_msg(&tx_msg,slave_addr,EXIT_BL_FUNCT,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		if(rx_msg.funct == EXIT_BL_FUNCT)return 1;
		else return 0;
	}
	else return 0;
}

int MsgEnterBL(int slave_addr)
{
	stxetx_make_msg(&tx_msg,slave_addr,ENTER_BL_FUNCT,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		return 1;
		/*if(rx_msg.funct == ACK_FUNCT)return 1;
		else return 0;*/
	}
	return 0;
}

int MsgGetImgCRC(int slave_addr,unsigned short *img_crc)
{
	stxetx_make_msg(&tx_msg,slave_addr,GET_CRC_FUNCT,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		if(rx_msg.funct == GET_CRC_FUNCT)
		{
			if(rx_msg.datalen == 2)
			{
				//printf("crc_msg:0x%02x 0x%02x",rx_msg.data[1],rx_msg.data[0]);
				*img_crc = (  (((unsigned short)(rx_msg.data[1]))<<8) | ((unsigned short)(rx_msg.data[0])) );
				return 1;
			}
		}
	}
	return 0;
}

int MsgGetAppOffset(int slave_addr,int *offset)
{
	stxetx_make_msg(&tx_msg,slave_addr,GET_APP_OFFSET_FUNCT,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		//printf("datalen:%d f:%d\n",rx_msg.datalen,rx_msg.funct);
		if(rx_msg.funct == GET_APP_OFFSET_FUNCT)
		{

			if(rx_msg.datalen == 4)
			{
				*offset = ((int)rx_msg.data[3]<<24)|((int)rx_msg.data[2]<<16)|((int)rx_msg.data[1]<<8)|((int)rx_msg.data[0]);
				return 1;
			}
		}
	}//else printf("nini!!!!!\n");
	return 0;
}

int MsgGetVersion(int slave_addr,char *version)
{
	int i;
	stxetx_make_msg(&tx_msg,slave_addr,GET_VERSION_FUNCT,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		if(rx_msg.funct == GET_VERSION_FUNCT)
		{
			if(rx_msg.datalen > 8)
			{
				for(i=0;i<rx_msg.data[8];i++)
				{
					version[i]= rx_msg.data[9+i];
				}
				return 1;
			}
		}
	}
	return 0;
}

int MsgGetAppVersion(int slave_addr,char *version)
{
	int i;
	stxetx_make_msg(&tx_msg,slave_addr,GET_APP_VERSION_FUNCT,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		if(rx_msg.funct == GET_APP_VERSION_FUNCT)
		{
			if(rx_msg.datalen > 0)
			{
				for(i=0;i<rx_msg.datalen;i++)
				{
					version[i]= rx_msg.data[i];
				}
				return 1;
			}
		}
	}
	return 0;
}

int MsgProgramData(int slave_addr,unsigned char *ProgramData,int ProgramDataOffset,int ProgramDataLen,int AppAddr)
{
	int i;
	unsigned char tx_data[150];
	for(i=0;i<ProgramDataLen;i++)
	{
		tx_data[i] = ProgramData[ProgramDataOffset+i];
	}
	tx_data[ProgramDataLen] = AppAddr & 0xff;
	tx_data[ProgramDataLen+1] = (AppAddr>>8) & 0xff;
	tx_data[ProgramDataLen+2] = (AppAddr>>16) & 0xff;
	tx_data[ProgramDataLen+3] = (AppAddr>>24) & 0xff;

	stxetx_make_msg(&tx_msg,slave_addr,PROGRAM_DATA_FUNCT,0,ProgramDataLen+4,tx_data);
	stxetx_tx_data2(&tx_msg,0);

	if(stxetx_rx_msg(&rx_msg))
	{
		/*if(rx_msg.funct == PROGRAM_DATA_FUNCT)*/return 1; //todo odkomentiraj
	}
	return 0;

}





int MsgSetRol(int slave_addr,unsigned char *rol_openness,int roln)
{
	int i;
	unsigned char tx_data[10];
	for(i=0;i<roln;i++)
	{
		tx_data[i] = rol_openness[i];
	}
	stxetx_make_msg(&tx_msg,slave_addr,SET_ROL,0,roln,tx_data);
	stxetx_tx_data2(&tx_msg,0);

	if(stxetx_rx_msg(&rx_msg))
	{
		/*if(rx_msg.funct == PROGRAM_DATA_FUNCT)*/return 1; //todo odkomentiraj
	}
	return 0;
}

int MsgSetRolParam(int slave_addr,t_rol_param *rol_param,int roln)
{
	int datalen =0;
	unsigned char tx_data[30];
	int n=0;
	for(n=0;n<roln;n++)
	{
		datalen += mybus_put_short(&tx_data[datalen], rol_param[n].uptime); //todo preveri
		datalen += mybus_put_short(&tx_data[datalen], rol_param[n].downtime);
		datalen += mybus_put_char(&tx_data[datalen], rol_param[n].man_auto);
	}
	stxetx_make_msg(&tx_msg,slave_addr,SET_ROL_PARAM,0,datalen,tx_data);
	stxetx_tx_data2(&tx_msg,0);

	if(stxetx_rx_msg(&rx_msg))
	{
		/*if(rx_msg.funct == PROGRAM_DATA_FUNCT)*/return 1; //todo odkomentiraj
	}
	return 0;
}


int MsgGetRol(int slave_addr,unsigned char *rol_opennes,int *roln)
{
	int n=0;
	stxetx_make_msg(&tx_msg,slave_addr,GET_ROL,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		//printf("datalen:%d f:%d\n",rx_msg.datalen,rx_msg.funct);
		if(rx_msg.funct == GET_ROL && rx_msg.datalen<=4)
		{
			for(n=0;n<(rx_msg.datalen);n++)
			{
				rol_opennes[n]=rx_msg.data[n];
			}
			*roln = n;
			return 1;
		}
	}//else printf("nini!!!!!\n");
	return 0;
}
int MsgGetRolStatus(int slave_addr,t_rol_status_msg *rol_status)
{
	int i=0;
	stxetx_make_msg(&tx_msg,slave_addr,GET_STATUS,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		//printf("datalen:%d f:%d\n",rx_msg.datalen,rx_msg.funct);
		if(rx_msg.funct == GET_STATUS && rx_msg.datalen<=7)
		{

			rol_status->temp_valid = mybus_get(rx_msg.data+i,CHAR_LEN);
			i+=CHAR_LEN;
			rol_status->temperature = mybus_get(rx_msg.data+i,SHORT_LEN);
			i+=SHORT_LEN;
			rol_status->rol_auto_man[0] = mybus_get(rx_msg.data+i,CHAR_LEN);
			i+=CHAR_LEN;
			rol_status->rol_opennes[0] = mybus_get(rx_msg.data+i,CHAR_LEN);
			i+=CHAR_LEN;
			rol_status->rol_auto_man[1] = mybus_get(rx_msg.data+i,CHAR_LEN);
			i+=CHAR_LEN;
			rol_status->rol_opennes[1] = mybus_get(rx_msg.data+i,CHAR_LEN);
			i+=CHAR_LEN;
			return 1;
		}
	}//else printf("nini!!!!!\n");
	return 0;
}




int MsgGetRolParam(int slave_addr,t_rol_param *rol_param,int *roln)
{
	int n=0;
	int i=0;
	stxetx_make_msg(&tx_msg,slave_addr,GET_ROL_PARAM,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		//printf("datalen:%d f:%d\n",rx_msg.datalen,rx_msg.funct);
		if(rx_msg.funct == GET_ROL_PARAM && rx_msg.datalen<=10)
		{
			//mybus_get(unsigned char * rx_data,int type)

			for(n=0;n<(rx_msg.datalen/5);n++)
			{
				rol_param[n].uptime = mybus_get(rx_msg.data+i,SHORT_LEN);
				i+=SHORT_LEN;
				rol_param[n].downtime = mybus_get(rx_msg.data+i,SHORT_LEN);
				i+=SHORT_LEN;
				rol_param[n].man_auto = mybus_get(rx_msg.data+i,CHAR_LEN);
				i+=CHAR_LEN;
			}
			*roln = n;
			return 1;
		}
	}//else printf("nini!!!!!\n");
	return 0;
}

int MsgGetTemperature(int slave_addr,int *temperature)
{

	stxetx_make_msg(&tx_msg,slave_addr,GET_TEMP,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		//printf("datalen:%d f:%d\n",rx_msg.datalen,rx_msg.funct);
		if(rx_msg.funct == GET_TEMP && rx_msg.datalen==3)
		{
			*temperature = mybus_get(rx_msg.data+1,SHORT_LEN);
			/*for(n=0;n<(rx_msg.datalen);n++)
			{
				rol_opennes[n]=rx_msg.data[n];
			}*/
			return 1;
		}
	}//else printf("nini!!!!!\n");
	return 0;
}

int MsgGetUptime(int slave_addr,unsigned long *uptime)
{

	stxetx_make_msg(&tx_msg,slave_addr,UPTIME,0,0,0);
	stxetx_tx_data2(&tx_msg,0);
	if(stxetx_rx_msg(&rx_msg))
	{
		//printf("datalen:%d f:%d\n",rx_msg.datalen,rx_msg.funct);
		if(rx_msg.funct == UPTIME && rx_msg.datalen==4)
		{
			*uptime = mybus_get(rx_msg.data,LONG_LEN);
			/*for(n=0;n<(rx_msg.datalen);n++)
			{
				rol_opennes[n]=rx_msg.data[n];
			}*/
			return 1;
		}
	}//else printf("nini!!!!!\n");
	return 0;
}

int MsgSetAddr(unsigned short slave_addr,unsigned short slave_addr_new)
{
	int datalen =0;
	unsigned char tx_data[30];

		datalen += mybus_put_short(&tx_data[datalen], slave_addr_new);

	stxetx_make_msg(&tx_msg,slave_addr,SET_ADDR,0,datalen,tx_data);
	stxetx_tx_data2(&tx_msg,0);

	if(stxetx_rx_msg(&rx_msg))
	{
		if(rx_msg.funct == SET_ADDR)return 1;
	}
	return 0;
}






