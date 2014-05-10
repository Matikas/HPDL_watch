/*
* HDPL2416_laikrodis.c
*
* Created: 2014-02-15 15:21:30
*  Author: Marius
*/

//crystal - 32,768 kHz
//Check fuse bits in "fuse bits.PNG" for khazama AVR Programmer


#ifndef F_CPU
#define F_CPU 32768
#endif

#define DISPLAY_POWER 0x73
#define BLANK_DISPLAY 0x80

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>


uint32_t time_s = 0;
uint32_t TimeStamp = 0;

uint8_t minutes = 0;
uint8_t hours = 0;

uint8_t dispIsOn;

ISR(TIMER2_OVF_vect)
{
	time_s+=8;
	//time_s += 1;
	if(time_s >= 86400)
	time_s = 0;
	
	if(time_s - TimeStamp <= 16)
	{
		UpdateDisplay(time_s);
	}
	else
	{
		
		PORTD&=~BLANK_DISPLAY;
		PORTD&=~DISPLAY_POWER;

		cli();
		sleep_enable();
		sei();
		dispIsOn = 0;
		
	}
}

//Prideti valanda
ISR(INT0_vect)
{
	if(dispIsOn == 1)
	{
		time_s += (60*60);
		if(time_s >= 86400)
		time_s = 0;
	}
	TimeStamp = time_s;
	PORTD |= DISPLAY_POWER;
	PORTD |= BLANK_DISPLAY;
	UpdateDisplay(time_s);
	
	if(dispIsOn == 0)
	{
		cli();
		_delay_ms(2000);
		sei();
	}
	dispIsOn = 1;
}

//Prideti minute
ISR(INT1_vect)
{
	if(dispIsOn == 1)
	{
		time_s += 60;
		if(time_s >= 86400)
		time_s = 0;
	}
	TimeStamp = time_s;
	PORTD |= DISPLAY_POWER;
	PORTD |= BLANK_DISPLAY;
	UpdateDisplay(time_s);
	
	if(dispIsOn == 0)
	{
		cli();
		_delay_ms(2000);
		sei();
	}
	dispIsOn = 1;
}

//Ijungti displeju
ISR(INT2_vect)
{
	TimeStamp = time_s;
	PORTD |= DISPLAY_POWER;
	PORTD|=BLANK_DISPLAY;
	UpdateDisplay(time_s);
}

void UpdateDisplay(uint32_t seconds)
{
	minutes = (seconds / 60) % 60;
	hours = (seconds / 3600) % 3600;
	
	uint8_t address = 0;
	
	PORTA = address;
	PORTC = minutes%10 + 48;
	
	address++;
	
	PORTA = address;
	PORTC = (minutes/10)%10+48;
	
	address++;
	
	PORTA = address;
	PORTC = hours%10+48;
	
	address++;
	
	PORTA = address;
	PORTC = (hours/10)%10+48;
}

int main(void)
{
	DDRD=0xFF;
	DDRC=0xFF;
	DDRA=0xFF;
	DDRB=0xFF;
	PORTC&=0;	//Duomenys
	//PORTD&=0;	//BL prijungtas prie PD7
	PORTA&=0;	//Adresai
	PORTB&=0;
	
	PORTD |= DISPLAY_POWER;
	PORTD |= BLANK_DISPLAY;
	
	//8bit timer2 nustatymai
	//Set prescale to 1024
	TCCR2 |= (1<<CS22)|(1<<CS21)|(1<<CS20);
	//wait for registers update
	while (ASSR & ((1<<TCN2UB)|(1<<TCR2UB)));
	//clear interrupt flags
	TIFR  = (1<<TOV2);
	//enable TOV2 interrupt
	TIMSK  = (1<<TOIE2);
	
	//Mygtuku interruptai
	MCUCR |= 0x0F; //(ISC00 - ISC11) == 1
	GICR |= (1<<INT0) | (1<<INT1) | (1<<INT2);
	MCUCSR |= (1<<ISC2);
	
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	sleep_enable();
	sei();
	while(1)
	{
		
	}
}