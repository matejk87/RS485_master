
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs232.h"
#include "crc16.h"
#include "stxetx.h"
#include "serial_master.h"
#include "rol.h"
#include "timer.h"


#define MAX_DEVICES 20



t_timer check_log_timer;


int timeout(clock_t *timer,unsigned long timeout)
{
	if(!timeout){*timer= clock();return 1;}
	if((clock()- *timer)>timeout)
	{
		*timer = clock();
		return 1;
	}
	return 0;
}

void gotoxy(int x,int y)
 {
 printf("%c[%d;%df",0x1B,y,x);
 }

int main(int argc, char ** argp)
{
	int n_of_dev=0;
	clock_t print_timer;
	  unsigned long dev_errors[MAX_DEVICES];
	  int i=0,result,device_addresses[MAX_DEVICES];
	unsigned short dev_img_crc=0,my_crc=0;
  int  temperature=0,PacketSent=0;
  unsigned long error_cntr=0,msg_cntr=0,byte_rate=0,msg_per_sec=0,print_n=0,msg_cntr_old=0;
  time_t rawtime;
  struct tm * timest;
  int n=0,device_addres,app_offset=0,progress=0,
  cport_nr=0,        /* /dev/ttyS0 (COM1 on windows) */
  bdrate=115200;


  if(argc < 3)
  {
	  printf(" RS485_tester - argc error!\n");
	  printf(" arguments: com_port_n RS485_addr\n");void gotoxy(int x,int y)
	  {
	  printf("%c[%d;%df",0x1B,y,x);
	  }
      return 0;
  }
  cport_nr=strtoul(argp[1],0,0)-1;
  if(OpenComport(cport_nr, bdrate))
  {
    printf("Can not open comport COM%d\n",cport_nr);
    getchar();
    return(0);
  }else printf("\n COM%d open:OK\n",cport_nr);
  if(serial_master_init(cport_nr)!=1)
  {
	  	printf("serial_master_init ERROR");
	    getchar();
	    return(0);
  }
  for(i=1;argc>i;i++)
  {
	  printf("arg[%d]=",i);
	  printf(argp[i]);
	  printf("\n");
  }
  printf("clocks_per_sec:%u\n",CLOCKS_PER_SEC);


  time_t clk = time(NULL);
  printf("%s", ctime(&clk));


  printf("RS485 devices:\n");
   for(i=2;argc>i;i++)
   {
 	  device_addresses[n_of_dev]= strtoul(argp[i],0,0);
 	  printf("\n - 0x%x",device_addresses[n_of_dev]);
 	  dev_errors[n_of_dev]=0;
 	  n_of_dev++;
   }
  while(1)
  {

	  time(&rawtime);
	  timest = localtime ( &rawtime );

	  for(i=0;i<n_of_dev;i++)
	  {
		  if(!MsgPing(device_addresses[i]))
		  {
			  error_cntr++;
			  dev_errors[i]++;

		  }
		  msg_cntr++;
	  }
	  if(timeout(&print_timer,30000))
	 	  {
	 		  print_n++;
	 		  if(print_n >= 10)
	 		  {
	 			print_n = 0;
	 			msg_per_sec = msg_cntr-msg_cntr_old;
	 			msg_cntr_old = msg_cntr;
	 		  }
	 		 // gotoxy(0,10);
	 		  system("clear"); //"clear" for linux  "cls for win"
	 		  printf("total_msg_cnt:%u errors:%u msg/s:%u  \n",msg_cntr,error_cntr,msg_per_sec);
	 		  for(i=0;i<n_of_dev;i++)
	 		  {
	 			  printf("dev:0x%x msg:%u err:%u\n",device_addresses[i],msg_cntr/n_of_dev,dev_errors[i]);
	 		  }
	 	  }

  }
  CloseComport(cport_nr);

  return(0);
}


