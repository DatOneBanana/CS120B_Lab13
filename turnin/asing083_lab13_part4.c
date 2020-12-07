/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "bit.h"
#include "timer.h"
#include "scheduler.h"
#endif

#define HIGH 700
#define LOW  300

void A2D_init(void) {
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<20; i++ ) {
		asm("nop");
	}
}

enum STATES { WAIT};

int tick(int state) {

	static unsigned char pattern = 0x80;
    	static unsigned char row = 0xFE;
    	static unsigned short up_down;
	static unsigned short right_left;

	switch (state) {
		case WAIT:
			Set_A2D_Pin(0);
			right_left = ADC;
			Set_A2D_Pin(1);
			up_down = ADC;

            		if (right_left < (LOW - 100)) {
                		if (pattern != 0x80) {
                    			pattern = pattern << 1;
                		}
                		else {
                    			pattern = 0x01;
                		}
            		}
            		else if (right_left > (HIGH + 100)) {
                		if (pattern != 0x01) {
                    			pattern = pattern >> 1;
                		}
                		else {
                    			pattern = 0x80;
                		}
            		}

            		if (up_down < LOW) {
				if (row != 0xEF) {
					row = ~( ~row << 1);
				}
                		else {
                    			row  = 0xFE;
                		}
            		}
            		else if (up_down > HIGH ) {
                		if ( row != 0xFE) {
                    			row = ~( ~row >> 1) | 0x80;
                		}
                		else {
                    			row = 0xEF;
                		}
            		}
			break;
		default:
			state = WAIT;
			break;
    	}
	PORTC = pattern;
    	PORTD = row;
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	A2D_init();

	static task task1;
	task *tasks[] = { &task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);



	task1.state = 0;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &tick;

	TimerSet(100);
	TimerOn();

    	unsigned short i;
    	while (1) {
		for(i = 0; i < numTasks; i++){
			if(tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 100;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    	}
    return 1;
}
