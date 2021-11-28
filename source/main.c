/*	Author: lab
 *  Partner(s) Name: Aung Thu Hein
 *	Lab Section:
 *	Assignment: First Demo
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *	
 *	demo link: https://www.youtube.com/watch?v=QEKUVQeDGBs
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "shift_register.h"
#include "joystick.h"
#endif

typedef struct task {
    int state;                   // Task's current state
    unsigned long period;        // Task period
    unsigned long elapsedTime;   // Time elapsed since last task tick
    int(*TickFct)(int);         // Task tick function
} task;

task tasks[3];
const unsigned char tasksNum = 3;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodJoyStick = 150;
const unsigned long periodPaddle = 150;
const unsigned long periodLEDmatrix = 150;

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
        break;
    default:
        break;
    }
    return state;
}

unsigned short col_paddle[8] = { ~0x0108,~0x0104,~0x0102,~0x0101,~0x0180,~0x0140,~0x0120,~0x0110 };
enum PaddleSM{P_INIT,P_START};
int PaddleTick(int state) {
    x = x - 512;
    static unsigned char i;
    switch (state) {
    case P_INIT:
        i = 3;
        state = P_START;
        break;
    case P_START:
        break;
    default:
        state = P_INIT;
        break;
    }
    switch (state) {
    case P_INIT:
        break;
    case P_START:
        if (x <= 80) {
            send_data(col_paddle[i]);
        } //default
        else if (x >= 800) {
            if (i > 0) { i--; }
            send_data(col_paddle[i]);
        } //left
        else {
            if (i < 7) { i++; }
            send_data(col_paddle[i]);
        } //right
        /*
        if (y <= 80) { ledy = 0x00; } //default
        else if (y >= 800) { ledy = 0x08; } //down
        else { ledy = 0x04; } //up
        PORTB = ledx | ledy;
        break;
        */
    default:
        break;
    }
        return state;
}



unsigned short col_1[8] = { ~0x8002,~0x4002,~0x2002,~0x1002,~0x0802,~0x0402,~0x0202,~0x0102 };
enum LEDmatrixSM { LED_INIT, LED_START };
int LEDmatrixTick(int state) {
    static unsigned char i;
    switch (state) {
    case LED_INIT:
        i = 0;
        state = LED_START;
        break;
    case LED_START:
        i = (i + 1) % 8;
        break;
    default:
        state = LED_INIT;
        break;
    }
    switch (state) {
    case LED_INIT:
        break;

    case LED_START:
        send_data(col_1[i]);
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
    Joystick_init();

    tasks[i].state = J_INIT;
    tasks[i].period = periodJoyStick;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &JoystickTick;
    ++i;

    tasks[i].state = P_INIT;
    tasks[i].period = periodPaddle;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &PaddleTick;
    ++i;

    tasks[i].state = LED_INIT;
    tasks[i].period = periodLEDmatrix;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &LEDmatrixTick;


    TimerSet(tasksPeriodGCD);
    TimerOn();

    while (1) {}
    return 0;
}



