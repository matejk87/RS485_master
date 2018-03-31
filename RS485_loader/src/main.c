
/**************************************************

file: main.c
purpose: simple demo that receives characters from
the serial port and print them on the screen

**************************************************/

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define PRINT_XML 0

#include "rs232.h"
#include "crc16.h"
#include "stxetx.h"
#include "serial_master.h"
/*
int xml_save(char * xmlpath,char * buff)
{
	FILE * xmlfile;
	xmlfile=fopen(xmlpath,"w");
	if(xmlfile==NULL)return -1;
	fputs(buff,xmlfile);
	fflush(xmlfile);
	fclose(xmlfile);
	return 0;
}*/
/*
void exit(void)
{
}*/

int main(int argc, char ** argp)
{
	unsigned short dev_img_crc=0,my_crc=0;
  int  PacketSent=0;
  int i,file_size,result,device_addres,app_offset=0,progress=0,
  cport_nr=0,        /* /dev/ttyS0 (COM1 on windows) */
  bdrate=115200;
  unsigned char *program_data;
  FILE * binfile;
  if(argc < 4)
  {
	  printf(" argc error!");
      return 0;
  }
  cport_nr=strtoul(argp[1],0,0)-1;
  if(OpenComport(cport_nr, bdrate))
  {
    printf("Can not open comport COM%d\n",cport_nr);
  //  Sleep(3000);
    getchar();
    return(0);
  }else printf("\n COM%d open:OK\n",cport_nr);
  if(serial_master_init(cport_nr)!=1)
  {
	  	printf("serial_master_init ERROR");
	    getchar();
	    return(0);
  }
  //serial_master_init = 1;
  for(i=0;argc>i;i++)
  {
	  printf("arg[%d]=",i);
	  printf(argp[i]);
	  printf("\n");
  }
  device_addres = strtoul(argp[2],0,0);
  printf("device addres:0x%x\n",device_addres);
  binfile = fopen (argp[3],"rb") ;
  if(binfile == NULL )
  {
        printf("cannot open file\n");
        getchar();
        return 0;
  }
  // obtain file size:
  fseek (binfile , 0 , SEEK_END);
  file_size = ftell (binfile);
  rewind (binfile);
  printf("file size:%u\n",file_size);
  // allocate memory to contain the whole file:
  program_data = (unsigned char*) malloc (sizeof(unsigned char)*file_size);
  if (program_data == NULL)
  {
	  printf("Memory error\n");
	  getchar();
	  return 0;
  }
  result = fread(program_data,1,file_size,binfile);
  if(result != file_size)
  {
	  printf("read file to ram error: size:%u\n",result);
	  getchar();
	  return 0;
  }
  printf("read file to ram:OK size:%d\n",result);
  fclose(binfile);
 /* while(1)//todo odstrani while zanko
  {
	  Sleep(10);
	  if(MsgPing(device_addres))
	  {
		  printf("\n ping ok");
	  }
	  else
	  {
		  printf("\n no response from device");
		  return 1;
	  }
  }*/
  if(MsgPing(device_addres))
  {
	  printf("device:0x%x detected - going to BL:",device_addres);
	  usleep(5000);
	  if(MsgEnterBL(device_addres))
	  {
		  printf("OK\n");
	  }
	  else
	  {
		  printf("ERROR\n");
		  getchar();
		  return 0;
	  }
	  usleep(4000000); // todo koliko �asa?
	  if(MsgBLPing(device_addres))
	  {
		  printf(" BL ping:OK\n");
	  }
	  else
	  {
		  printf(" BL ping:ERROR ");
		  getchar();
		  return 0;
	  }
  }
  else if(MsgBLPing(device_addres))
  {
	  printf("device:0x%x already in BL\n",device_addres);
  }
  else
  {
	  printf("ERROR - no response from device:0x%x\n",device_addres);
	  getchar();
	  return 0;
  }
  usleep(10000);
  if(MsgGetAppOffset(device_addres,&app_offset)!=1)
  {
	  printf("ERROR - no response from device:0x%x\n",device_addres);
	  getchar();
	  return 0;
  }
  if(app_offset>=file_size)return 0;
  printf("app offset:0x%x\n ",app_offset);
  PacketSent = 0;
  for(i=app_offset;i<(file_size);i+=128)
  {
	  usleep(3000);
		if(MsgProgramData(device_addres,program_data,i,128,i-app_offset)==0)
		{
			// en retry
			usleep(10000);
			printf("data msg retry!\n");
			if(MsgProgramData(device_addres,program_data,i,128,i-app_offset)==0)
			{
				printf("ERROR - no response from device:0x%x\n",device_addres);
				getchar();
				return 0 ;
			}
		}
		PacketSent++;
		progress = ((i - app_offset)*100)/(file_size - app_offset);
		printf("\rprogress:%02d ",progress);
	}
    printf("\rprogress:100\n");
	if(MsgGetImgCRC(device_addres,&dev_img_crc))
	{
		crc16_n(&my_crc,&program_data[app_offset],PacketSent*128);
		printf("my_crc:0x%04x module_crc:0x%04x - :",my_crc,dev_img_crc);
		if(dev_img_crc == my_crc )
		{
			printf("OK\n");
			if(MsgExitBL(device_addres))
			{
				printf("Flashing:OK\n");
			}
		}
		else printf("ERROR!\n");
	}
	else printf("MsgGetImgCRC error");
   CloseComport(cport_nr);
#ifdef _WIN32
  	getchar(); //todo odstrani
    Sleep(2000);  /* it's ugly to use a sleeptimer, in a real program, change the while-loop into a (interrupt) timerroutine */
#else
    usleep(100000);  /* sleep for 100 milliSeconds */
#endif

  return(0);
}


