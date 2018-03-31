/*
 * timer.c
 *
 *  Created on: Oct 29, 2017
 *      Author: matej
 */



//SysTick_Config(SystemCoreClock / 1000);
#include <time.h>
#include "timer.h"

/*

void SysTick_Handler(void)  /// systick interrupt
{
  //TimingDelay_Decrement();
   sample_input_states(); //todo odkometiraj
  timetick++;
  if((timetick%1000)==0)timesec++;
}
*/

t_timer timer_duration_us(t_timer * timer)
{
	return (clock() - *timer);
}

int timeout_us(t_timer * timer, unsigned long timeout)
{
	volatile unsigned long tmp;
	tmp=clock();
	if(timeout==0)
	{
		*timer=clock();
		return 1;
	}
	if((tmp-*timer)>=timeout)
	{
		*timer=clock();
		return 1;
	}
	return 0;

}

void delay_us(unsigned int ms)
{
	t_timer delay_timer;
	timeout_us(&delay_timer,0);
	while(!timeout_us(&delay_timer,ms));
}






t_timer timer_duration_ms(t_timer * timer)
{
	return (clock()/1000 - *timer);
}

int timeout_ms(t_timer * timer, unsigned long timeout)
{
	volatile unsigned long tmp;
	tmp=clock()/1000;
	if(timeout==0)
	{
		*timer=clock()/1000;
		return 1;
	}
	if((tmp-*timer)>=timeout)
	{
		*timer=clock()/1000;
		return 1;
	}
	return 0;

}

void delay_ms(unsigned int ms)
{
	t_timer delay_timer;
	timeout_ms(&delay_timer,0);
	while(!timeout_ms(&delay_timer,ms));
}


int GetMinuteOfDay(void)
{
	time_t rawtime;
	struct tm * timeinfo;

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	//printf("Current local time and date: %s", asctime(timeinfo));
	return timeinfo->tm_hour*60+timeinfo->tm_min;

}




