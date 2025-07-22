///updated 19-07-2025 
//pb@pd2 toggles between countdown and countup mode on timer1

#define F_CPU 16000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "usart.h"
#include "lcd.h"  

/* 
	main code -- toggle timer to count up seconds 



	Pinout for DIP28 ATMega328P:

                           -------
     (PCINT14/RESET)*PC6 -|1    28|- /PC5 (ADC5/SCL/PCINT13)
       (PCINT16/RXD)*PD0 -|2    27|- /PC4 (ADC4/SDA/PCINT12)
       (PCINT17/TXD)*PD1 -|3    26|- /PC3 (ADC3/PCINT11)
     (PCINT18/INT0) PD2/ -|4    25|- /PC2 (ADC2/PCINT10)
 (PCINT19/OC2B/INT1)*PD3 -|5    24|- /PC1 (ADC1/PCINT9)
    (PCINT20/XCK/T0)*PD4 -|6    23|- /PC0 (ADC0/PCINT8)
                    *VCC -|7    22|- GND*
                    *GND -|8    21|- AREF*
(PCINT6/XTAL1/TOSC1)*PB6 -|9    20|- AVCC*
(PCINT7/XTAL2/TOSC2)*PB7 -|10   19|- PB5*(SCK/PCINT5)
   (PCINT21/OC0B/T1)*PD5 -|11   18|- PB4*(MISO/PCINT4)
 (PCINT22/OC0A/AIN0)*PD6 -|12   17|- PB3*(MOSI/OC2A/PCINT3)
      (PCINT23/AIN1)*PD7 -|13   16|- /PB2 (SS/OC1B/PCINT2)
  (PCINT0/CLKO/ICP1)*PB0 -|14   15|- /PB1 (OC1A/PCINT1)
                           -------

	set up base circuit + load lcd
	set up timer 1
	push to lcd
	initialize other timers
	push buttons add

*/

// These are the connections between the LCD and the ATMega328P:
//
// LCD          ATMega328P
//----------------------------
// D7           PD3 (5)
// D6           PD4 (6)
// D5           PD5 (11)
// D4           PD6 (12)


// LCD_E        PD7 (13)
// LCD_W        GND
// LCD_RS       PB0 (14)
// V0           2k+GND
// VCC          5V
// GND          GND


//---------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------PHASE1:BASE LCD------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------
void Configure_Pins(void) //for LCD 
{
	DDRB	|=0b00000001; // PB0 is output.
    DDRD	|=0b11111000; // PD2, PD3, PD4, PD5, PD6, and PD7 are outputs. PD2 IS an input
	PORTD	|=0b00000100; //PD2 activate pull-up
	//DDRD|= 0b11111100; // PD2�PD7 as output
}
//---------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------PHASE2:timer------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------
volatile int timemode_flag = 0; 
volatile int isr1_ctr_s =40; //
volatile int isr1_ctr_m =1; //
volatile int isr1_ctr_h =00; //

volatile int init =1;
//read in isr so volatile

void time_handle (void) 
{
	if (timemode_flag) //if it hits 00 it just keeps subtracting ?? 
	{
		isr1_ctr_s = (isr1_ctr_s == 0)? 59: isr1_ctr_s-1;//reset to 0 if isr1 ctr is 59
		init =0; //clear init flag

		if (isr1_ctr_s == 59 && !init)
		{
			isr1_ctr_m = (isr1_ctr_m == 0)? 59: isr1_ctr_m-1; //xx:-1:59
			if (isr1_ctr_m == 59){ // -1:59:59
				isr1_ctr_h = (isr1_ctr_h == 0)? 24: isr1_ctr_h-1;
			}
		}
	}

	else 
	{//timemode flag = 0 : stopwatch mode
		isr1_ctr_s = (isr1_ctr_s == 59)? 0: isr1_ctr_s+1;//reset to 0 if isr1 ctr is 59
		init =0; //clear init flag

		if (isr1_ctr_s == 0 && !init)
		{
			isr1_ctr_m = (isr1_ctr_m == 59)? 0: isr1_ctr_m+1; //xx:+1:00
			if (isr1_ctr_m == 0){ // +1:00:00
				isr1_ctr_h = (isr1_ctr_h == 24)? 0: isr1_ctr_h+1;
			}
		}
	}
}

void timer_init1(void)
{
	TCCR1B |= _BV(WGM12)|_BV(CS10) | _BV(CS12); // pg 110 (cs10,!cs11,cs12) -> divide by 1024 (wgm12) = ctc mode
	//ctc mode = when tcnt1 = ocr1a -> tcnt1= 0
	OCR1A = 15625;//this is the compare value ( for 15625Hz, we need 15625 ticks for 1 second) ocf1a set high
	TIMSK1 |= _BV(OCIE1A); //execute interrupt vecotr wehn ocf1a flag set
	
}
// 'Timer 1 output compare A' Interrupt Service Routine
ISR(TIMER1_COMPA_vect)//handles only seconds
{
	time_handle();
}
//---------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------PHASE4:PUSHBUTTON------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------
int pb_pd2 ()
{
	int x = (PIND&0b00000100)? 1:0;
	return x;
}


//---------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------maincodesection------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------
void  init_all(void)
{
	usart_init(); // configure the usart and baudrate

	timer_init1();//trigger the timer1_initializing which triggers sei
	sei();//MAYBE PUT THIS IN MAIN

	Configure_Pins();
	LCD_4BIT();
}

int main(void)
{
	init_all();
	time_handle();

	char time[17];
	while (1)
	{
		if (pb_pd2() == 1) //if there pb is clicked on then we swap the flag val
		{
			timemode_flag = !timemode_flag;
		}
		// convert isr1_ctr to string and show on line 1
		sprintf(time, "%02d:%02d:%02d", isr1_ctr_h, isr1_ctr_m, isr1_ctr_s);  // pad with 0 if <10
		LCDprint(time, 1, 1);  // clear line
		_delay_ms(500);        // update every 0.5s to avoid flicker
	}

}
