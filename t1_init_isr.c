#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "usart.h"

//RECALL _BV(bit) == 1<< bit
/* Pinout for DIP28 ATMega328P:

                           -------
     (PCINT14/RESET) PC6 -|1    28|- PC5 (ADC5/SCL/PCINT13)
       (PCINT16/RXD) PD0 -|2    27|- PC4 (ADC4/SDA/PCINT12)
       (PCINT17/TXD) PD1 -|3    26|- PC3 (ADC3/PCINT11)
      (PCINT18/INT0) PD2 -|4    25|- PC2 (ADC2/PCINT10)
 (PCINT19/OC2B/INT1) PD3 -|5    24|- PC1 (ADC1/PCINT9)
    (PCINT20/XCK/T0) PD4 -|6    23|- PC0 (ADC0/PCINT8)
                     VCC -|7    22|- GND
                     GND -|8    21|- AREF
(PCINT6/XTAL1/TOSC1) PB6 -|9    20|- AVCC
(PCINT7/XTAL2/TOSC2) PB7 -|10   19|- PB5 (SCK/PCINT5)
   (PCINT21/OC0B/T1) PD5 -|11   18|- PB4 (MISO/PCINT4)
 (PCINT22/OC0A/AIN0) PD6 -|12   17|- PB3 (MOSI/OC2A/PCINT3)
      (PCINT23/AIN1) PD7 -|13   16|- PB2 (SS/OC1B/PCINT2)
  (PCINT0/CLKO/ICP1) PB0 -|14   15|- PB1 (OC1A/PCINT1)
                           -------
*/

volatile int isr1_ctr =0;
volatile int timemode_flag =0;//read in isr so volatile
// 'Timer 1 output compare A' Interrupt Service Routine
ISR(TIMER1_COMPA_vect)
{
	if (timemode_flag)
	isr1_ctr--;
	else 
	isr1_ctr++;
}


void timer_init1 (void)
{
	TCCR1B |= _BV(WGM12)|_BV(CS10) | _BV(CS12); // pg 110 (cs10,!cs11,cs12) -> divide by 1024 (wgm12) = ctc mode
	//ctc mode = when tcnt1 = ocr1a -> tcnt1= 0
	OCR1A = 15625;//this is the compare value ( for 15625Hz, we need 15625 ticks for 1 second) ocf1a set high
	TIMSK1 |= _BV(OCIE1A); //execute interrupt vecotr wehn ocf1a flag set
	sei();
}
int main (void)
{ 
	usart_init(); // configure the usart and baudrate
	timer_init1();
}