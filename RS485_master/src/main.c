
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


#define LOG_BUFF_SIZE 5000

t_timer check_log_timer;

void print_cfg(void)
{
	printf("CFG from CFG file:\n");
	for(int n=0;n<ROL_NUMBER_MAX;n++)
	{
		printf("addr:0x%x ",rol_cfg_file[n].addr);
		printf("name:%s ",rol_cfg_file[n].rol_name);
		printf("fulltime[0]:%dms ",rol_cfg_file[n].rol_fulltime_up[0]);
		printf("fulltime[1]:%dms ",rol_cfg_file[n].rol_fulltime_up[1]);
		printf("auto_man:%d ",rol_cfg_file[n].rol_auto_man);
		printf("up[0]:%d up[1]:%d ",rol_cfg_file[n].minutes_up[0],rol_cfg_file[n].minutes_up[1]);
		printf("down[0]:%d down[1]:%d \n",rol_cfg_file[n].minutes_down[0],rol_cfg_file[n].minutes_down[1]);
	}
}


char TemperatureLogBuff[LOG_BUFF_SIZE];
char RoletaLogBuff[LOG_BUFF_SIZE];

void DataLog(void)
{
	static int minutes_before;
	if(timeout_ms(&check_log_timer,900))
	{
		unsigned long unix_time;
		int n,i,len;
		char *p;

		time_t rawtime;
		struct tm * timeinfo;

		time (&rawtime);
		unix_time=time(NULL);
		timeinfo = localtime (&rawtime);



		if((timeinfo->tm_min%5==0) && (minutes_before != timeinfo->tm_min))
		{

			minutes_before = timeinfo->tm_min;
			printf("okrogla ura(5m): ");
			printf("%s", ctime(&rawtime));

			p=ctime(&rawtime);
			len = strlen(p);
			p[len-1]=0;

			snprintf(&TemperatureLogBuff[strlen(TemperatureLogBuff)],LOG_BUFF_SIZE,"%lu,%s,",unix_time,p);
			for(n=0;n< ROL_NUMBER_MAX;n++)
			{
				snprintf(&TemperatureLogBuff[strlen(TemperatureLogBuff)],LOG_BUFF_SIZE,"%s,%d,",rol_cfg_file[n].rol_name,rol_status[n].temperature*10);
			}
			snprintf(&TemperatureLogBuff[strlen(TemperatureLogBuff)],LOG_BUFF_SIZE,"\n");

			snprintf(&RoletaLogBuff[strlen(RoletaLogBuff)],LOG_BUFF_SIZE,"%lu,%s,",unix_time,p);
			for(n=0;n< ROL_NUMBER_MAX;n++)
			{
				snprintf(&RoletaLogBuff[strlen(RoletaLogBuff)],LOG_BUFF_SIZE,"%s,%d,%d,%d,%d,",rol_cfg_file[n].rol_name,rol_status[n].rol_opennes[0],rol_status[n].rol_auto_man[0],rol_status[n].rol_opennes[1],rol_status[n].rol_auto_man[0]);
			}
			snprintf(&RoletaLogBuff[strlen(RoletaLogBuff)],LOG_BUFF_SIZE,"\n");

			printf("strlen1:%lu strlen2:%lu\n",strlen(TemperatureLogBuff),strlen(RoletaLogBuff));
			printf("%s",TemperatureLogBuff);
			printf("%s",RoletaLogBuff);


			if(timeinfo->tm_min==0)
			{
				printf("polna ura\n");
				FILE *f;
				f = fopen("TemperatureLog.txt", "a+"); // a+ (create + append)
				if (f == NULL) { printf("error opening file TemperatureLog.txt");}
				fprintf(f,"%s",TemperatureLogBuff);
				TemperatureLogBuff[0]=0;
				fclose(f);

				f = fopen("RoletaLog.txt", "a+");
				if (f == NULL) { printf("error opening file TemperatureLog.txt");}
				fprintf(f,"%s",RoletaLogBuff);
				RoletaLogBuff[0]=0;
				fclose(f);
			}

		}

	}
}


int main(int argc, char ** argp)
{
	unsigned short dev_img_crc=0,my_crc=0;
  int  temperature=0,PacketSent=0;
  int n=0,i=0,file_size,result,device_addres,app_offset=0,progress=0,
  cport_nr=0,        /* /dev/ttyS0 (COM1 on windows) */
  bdrate=115200;
  unsigned char *program_data;
  FILE * binfile;


  if(argc < 2)
  {
	  printf(" argc error!\n");
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


  FILE *CFG_ROL_fp;
  CFG_ROL_fp=fopen("CFG_ROL.txt", "r");
  if(CFG_ROL_fp == NULL){
	  printf("Can't open cfg file:CFG_ROL.txt");
	  return 0;
  }
  char line[256];
  char *pch;
  printf("Reading CFG_ROL.txt file:");
  n=0;
  while (fgets(line, sizeof(line), CFG_ROL_fp)) {
      /* note that fgets don't strip the terminating \n, checking its
         presence would allow to handle lines longer that sizeof(line) */

      printf("%s", line);
      pch = strtok (line," ");
      i=0;
      while (pch != NULL)
      {
        printf ("%s\n",pch);
        switch(i)
        {
        case 0:
        	rol_cfg_file[n].addr = strtoul(pch,0,0);
        	break;
        case 1:
        	//rol_cfg_file.rol_name = pch;
        	strcpy(rol_cfg_file[n].rol_name,pch); //preveri
        	break;
        case 2:
        	rol_cfg_file[n].rol_fulltime_up[0] = strtoul(pch,0,0);
        	rol_cfg_file[n].rol_fulltime_down[0] = strtoul(pch,0,0);
        	break;
        case 3:
        	rol_cfg_file[n].rol_fulltime_up[1] = strtoul(pch,0,0);
        	rol_cfg_file[n].rol_fulltime_down[1] = strtoul(pch,0,0);
        	break;
        case 4:
        	rol_cfg_file[n].rol_auto_man = strtoul(pch,0,0);

        	break;
        case 5:
        	rol_cfg_file[n].minutes_up[0] = strtoul(pch,0,0);
        	break;
        case 6:
        	rol_cfg_file[n].minutes_up[1] = strtoul(pch,0,0);
        	break;
        case 7:
        	rol_cfg_file[n].minutes_down[0] = strtoul(pch,0,0);
        	break;
        case 8:
        	rol_cfg_file[n].minutes_down[1] = strtoul(pch,0,0);
        	break;
        default:
        	printf("CFG size error \n");

        	break;
        }

        pch = strtok (NULL, " ,.-");
        i++;
      }
      n++;
  }
  fclose(CFG_ROL_fp);

  print_cfg();

  time_t clk = time(NULL);
  printf("%s", ctime(&clk));

  TemperatureLogBuff[0]=0;
  RoletaLogBuff[0]=0;

  while(1)
  {
	  RoletaMain();
	  DataLog();



  }
  CloseComport(cport_nr);

  return(0);
}


