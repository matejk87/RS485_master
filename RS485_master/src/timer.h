/*
 * timer.h
 *
 *  Created on: Oct 29, 2017
 *      Author: matej
 */

#ifndef TIMER_H_
#define TIMER_H_


typedef volatile unsigned long t_timer;
int timeout_ms(t_timer * timer, unsigned long timeout);
void delay_ms(unsigned int ms);
t_timer timer_duration_ms(t_timer * timer);

int timeout_us(t_timer * timer, unsigned long timeout);
void delay_us(unsigned int ms);
t_timer timer_duration_us(t_timer * timer);


int GetMinuteOfDay(void);

#endif /* TIMER_H_ */
