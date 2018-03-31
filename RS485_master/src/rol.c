/*
 * rol.c
 *
 *  Created on: Oct 22, 2017
 *      Author: matej
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "rol.h"
#include "timer.h"
#include "serial_master.h"


t_rol_handler rolh;
t_rol_msg_handler rolmsgh;

t_rol_status_msg rol_status_msg;

t_rol_status_msg rol_status[ROL_NUMBER_MAX];

t_rol_cmd rol_cmd[ROL_NUMBER_MAX];

t_rol_cfg_file rol_cfg_file[ROL_NUMBER_MAX];

t_rol_param   rol_param_cmd[2];

t_timer cyc_status_print_timer;


void CyclicRead(void)
{
	if(MsgGetRolStatus(ROL_BASE_ADDR+rolmsgh.cyc_read_roln+1,&rol_status_msg))
	{
		rol_status[rolmsgh.cyc_read_roln] = rol_status_msg;
	}
	rolmsgh.cyc_read_roln++;
	if(rolmsgh.cyc_read_roln >= rolh.n_of_rols)rolmsgh.cyc_read_roln=0;
}


void CyclicWrite(void)
{
	if(rolh.n_of_rols > 0)
	{
		if(MsgSetRol(ROL_BASE_ADDR+rolmsgh.cyc_write_roln+1,rol_cmd[rolmsgh.cyc_write_roln].rol_openness,2))
		{
		//	printf("successfull write[%d]:%d %d\n",rolmsgh.cyc_write_roln,rol_cmd[rolmsgh.cyc_write_roln].rol_openness[0],rol_cmd[rolmsgh.cyc_write_roln].rol_openness[1]);

		}//else printf("Unsuccessfull write\n");
	/*	if(rol_cmd[rolmsgh.cyc_write_roln].rol_param[0].man_auto)
		{
			if(MsgSetRolParam(ROL_BASE_ADDR+rolmsgh.cyc_write_roln+1,rol_cmd[rolmsgh.cyc_write_roln].rol_param,2))  //
			{
				printf("\n rol parameter written![%d]",rolmsgh.cyc_write_roln);
				rol_cmd[rolmsgh.cyc_write_roln].rol_param[0].man_auto=0;
			}
		}*/
		rolmsgh.cyc_write_roln++;
		if(rolmsgh.cyc_write_roln >= rolh.n_of_rols)rolmsgh.cyc_write_roln=0;
	}
}


void AnyNewModule(void)
{
	if(MsgPing(ROL_BASE_ADDR+rolh.n_of_rols+1) && (rolh.n_of_rols < ROL_NUMBER_MAX))
	{

		printf("New ROL module found at address:0x%x  Number of ROL:%d\n",ROL_BASE_ADDR+rolh.n_of_rols+1,rolh.n_of_rols+1);

		rol_param_cmd[0].uptime= rol_cfg_file[rolh.n_of_rols].rol_fulltime_up[0];
		rol_param_cmd[0].downtime= rol_cfg_file[rolh.n_of_rols].rol_fulltime_down[0];
		rol_param_cmd[0].man_auto= rol_cfg_file[rolh.n_of_rols].rol_auto_man;
		rol_param_cmd[1].uptime= rol_cfg_file[rolh.n_of_rols].rol_fulltime_up[1];
		rol_param_cmd[1].downtime= rol_cfg_file[rolh.n_of_rols].rol_fulltime_down[1];
		rol_param_cmd[1].man_auto=rol_cfg_file[rolh.n_of_rols].rol_auto_man;
		if(MsgSetRolParam(ROL_BASE_ADDR+rolh.n_of_rols+1,rol_param_cmd,2))
		{
			printf("config of ROL module:OK\n");
		}
		else
		{
			printf("config of ROL module:ERROR\n");
		}
		rolh.n_of_rols++;
	}
	if(MsgPing(ROL_BASE_ADDR))
	{
		printf("New ROL module found at address:0x%x\n",ROL_BASE_ADDR);

		if(!MsgPing(ROL_BASE_ADDR+rolh.n_of_rols+1)) //address empty
		{

			if(MsgSetAddr(ROL_BASE_ADDR,ROL_BASE_ADDR+rolh.n_of_rols+1) && (rolh.n_of_rols < ROL_NUMBER_MAX))
			{
				rolh.n_of_rols++;
				printf("Successfully changed addr:0x%x to 0x%x  Number of ROL:%d\n",ROL_BASE_ADDR,ROL_BASE_ADDR+rolh.n_of_rols,rolh.n_of_rols);

			}
			else printf("Error changing device addr:0x%x to 0x%x",ROL_BASE_ADDR,ROL_BASE_ADDR+rolh.n_of_rols+1);
		}
	}
}


void SetRolStruct(void)
{
	int minutes_before;
	int n=0;

	if(/*minutes_before != GetMinuteOfDay()*/1)
	{
		minutes_before = GetMinuteOfDay();
		//printf("minutes_before:%d min_up:%d min_down:%d\n",minutes_before,rol_cfg_file[0].minutes_up[0],rol_cfg_file[0].minutes_down[0]);

		for(n=0;n<rolh.n_of_rols;n++)
		{
			if(rol_cfg_file[n].rol_fulltime_up[0])
			{
				if((minutes_before >= rol_cfg_file[n].minutes_up[0]) && (minutes_before <= rol_cfg_file[n].minutes_down[0]))rol_cmd[n].rol_openness[0] = 100;
				else rol_cmd[n].rol_openness[0] = 0;
			}
			if(rol_cfg_file[n].rol_fulltime_up[1])
			{
				if((minutes_before >= rol_cfg_file[n].minutes_up[1]) && (minutes_before <= rol_cfg_file[n].minutes_down[1]))rol_cmd[n].rol_openness[1] = 100;
				else rol_cmd[n].rol_openness[1] = 0;
			}
		}
//		rol_cmd[rolmsgh.cyc_write_roln].rol_openness
	}
}

void PrintRolStatus(void)
{
	int n=0;
	  time_t clk = time(NULL);
	  printf("\n%s", ctime(&clk));
	printf("ROL status:\n");
	for(n=0;n<rolh.n_of_rols;n++)
	{
		printf("%d ma:%d ma:%d rol:%d rol:%d temp:%d temp_valid:%d\n",n,rol_status[n].rol_auto_man[0],rol_status[n].rol_auto_man[1],rol_status[n].rol_opennes[0],rol_status[n].rol_opennes[1],rol_status[n].temperature,rol_status[n].temp_valid);
	}
}

void RoletaMain(void)
{
	int i;
	if(timeout_ms(&rolmsgh.cycr_timer,CYC_READ_TIME))
	{
		CyclicRead();
	}
	if(timeout_ms(&rolmsgh.cycw_timer,CYC_WRITE_TIME))
	{
		SetRolStruct();
		CyclicWrite();
	}
	if(timeout_ms(&rolmsgh.cyc_ping_new_timer,CYC_PING_NEW_TIME))
	{
		/*for(i=0;i<rolh.n_of_rols;i++)
		{
			printf(" T[%d]:%d",i,rol_status[i].temperature);
		}
		printf("\n");*/
		AnyNewModule();
	}
	if(timeout_ms(&cyc_status_print_timer,CYC_STATUS_PRINT_TIME))
	{
		PrintRolStatus();

	}


}
