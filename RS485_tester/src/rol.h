/*
 * rol.h
 *
 *  Created on: Oct 22, 2017
 *      Author: matej
 */

#ifndef ROL_H_
#define ROL_H_

#include "timer.h"

#define ROL_NAME_MAX_SIZE 20
#define ROL_BASE_ADDR 0x1100
#define MAX_ROLS_PER_MODULE 2

#define ROL_MANUAL 0
#define ROL_AUTO 1

#define ROL_NUMBER_MAX 11
#define CYC_READ_TIME 200
#define CYC_WRITE_TIME 630
#define CYC_PING_NEW_TIME 5000
#define CYC_STATUS_PRINT_TIME 60000



typedef struct
{
	short addr;
	unsigned char rol_name[ROL_NAME_MAX_SIZE];
	unsigned short rol_fulltime_up[MAX_ROLS_PER_MODULE];
	unsigned short rol_fulltime_down[MAX_ROLS_PER_MODULE];
	unsigned short rol_auto_man;
	unsigned short minutes_up[MAX_ROLS_PER_MODULE];
	unsigned short minutes_down[MAX_ROLS_PER_MODULE];
}
t_rol_cfg_file;


typedef struct
{
	unsigned char temp_valid;
	short temperature;
	unsigned char rol_auto_man[MAX_ROLS_PER_MODULE];
	unsigned char rol_opennes[MAX_ROLS_PER_MODULE];
}
t_rol_status_msg;

typedef struct
{
	int n_of_rols;

}t_rol_handler;

typedef struct
{
	int cyc_read_roln;
	int cyc_write_roln;
	t_timer cycr_timer;
	t_timer cycw_timer;
	t_timer cyc_ping_new_timer;


}t_rol_msg_handler;

typedef struct
{
	unsigned short uptime;
	unsigned short downtime;
	unsigned char  man_auto;
}t_rol_param;



typedef struct
{
	unsigned char rol_openness[2];
	//unsigned char auto_cmd;
	t_rol_param   rol_param[2];
}
t_rol_cmd;


extern t_rol_cmd rol_cmd[ROL_NUMBER_MAX];

extern t_rol_cfg_file rol_cfg_file[ROL_NUMBER_MAX];

extern t_rol_status_msg rol_status[ROL_NUMBER_MAX];

void RoletaMain(void);



#endif /* ROL_H_ */
