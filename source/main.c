/*	Author: lab
 *  Partner(s) Name: Aung Thu Hein
 *	Lab Section:
 *	Assignment: First Demo
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *	
 *	demo link: https://www.youtube.com/watch?v=AvZlZurKj0s
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#endif

void Joystick_init() {
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
}

unsigned short GetJoystickValue(unsigned char ch)
{
    ch &= 0b00000111;
    ADMUX = (ADMUX & 0xf8) | ch;
    ADCSRA |= (1 << ADSC);
    while ((ADCSRA) & (1 << ADSC));
    return(ADC);
}

typedef struct task {
    int state;                   // Task's current state
    unsigned long period;        // Task period
    unsigned long elapsedTime;   // Time elapsed since last task tick
    int(*TickFct)(int);         // Task tick function
} task;

task tasks[2];
const unsigned char tasksNum = 2;
const unsigned long tasksPeriodGCD = 5;
const unsigned long periodJoyStick = 5;
const unsigned long periodOutput = 5;

enum JoystickSM{J_INIT,J_START};
unsigned short x, y;
int JoystickTick(int state) {
    switch (state) {
    case J_INIT:
        state = J_START;
        break;
    case J_START:
        break;
    default:
        state = J_INIT;
        break;
    }
    switch (state) {
    case J_INIT:
        break;
    case J_START:
        x = GetJoystickValue(0);
        y = GetJoystickValue(1);
        break;
    default:
        break;
    }
    return state;
}

enum OutputSM{O_INIT,O_START};
int OutputTick(int state) {
    x = x - 512;
    y = y - 512;
    static unsigned char ledx;
    static unsigned char ledy;
    switch (state) {
    case O_INIT:
        state = O_START;
        break;
    case O_START:
        break;
    default:
        state = O_INIT;
        break;
    }
    switch (state) {
    case O_INIT:
        break;
    case O_START:
        if (x <= 80) { ledx = 0x00; } //default
        else if (x >= 800) { ledx = 0x02; } //left
	else { ledx = 0x01;} //right

        if (y <= 80) { ledy = 0x00; } //default
        else if (y >= 800) { ledy = 0x08; } //down
	else { ledy = 0x04;} //up
        PORTB = ledx | ledy;
        break;
    default:
        break;
    }
        return state;
}

void TimerISR() {
    unsigned char i;
    for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
        if (tasks[i].elapsedTime >= tasks[i].period) { // Ready
            tasks[i].state = tasks[i].TickFct(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += tasksPeriodGCD;
    }
}

int main() {
    unsigned char i = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    tasks[i].state = J_INIT;
    tasks[i].period = periodJoyStick;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &JoystickTick;
    ++i;

    tasks[i].state = O_INIT;
    tasks[i].period = periodOutput;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &OutputTick;

    Joystick_init();
    TimerSet(tasksPeriodGCD);
    TimerOn();

    while (1) {}
    return 0;
}

