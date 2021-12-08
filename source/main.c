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
#include <avr/eeprom.h> /* Include AVR EEPROM header file */
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "shift_register.h"
#include "joystick.h"
#include "io.h"
#endif

typedef struct task {
    int state;                   // Task's current state
    unsigned long period;        // Task period
    unsigned long elapsedTime;   // Time elapsed since last task tick
    int(*TickFct)(int);         // Task tick function
} task;

task tasks[6];
const unsigned char tasksNum = 6;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodStart = 50;
const unsigned long periodPaddle = 150;
const unsigned long periodLEDmatrix = 150;
const unsigned long periodDisplay = 150;
const unsigned long periodEEPROM = 50;
const unsigned long periodLCD = 100;

unsigned char high_score;
unsigned char end_display;
unsigned char end, start_display, game_on, t, time_limit;

enum StartSM{S_INIT,S_START,S_PLAY};
unsigned char start;
int StartTick(int state) {
    unsigned char tempA = ~PINC & 0x01;
    switch (state) {
    case S_INIT:
        start = 0;
        state = S_START;
        break;
    case S_START:
        state = (tempA == 0x01) ? S_PLAY : S_START;
        break;
    case S_PLAY:
        state = end ? S_START : S_PLAY;
        break;
    default:
        state = S_START;
        break;
    }
    switch (state) {
    case S_INIT:
        break;
    case S_START:
        start = 0;
        break;
    case S_PLAY:
        start = 1;
        break;
    default:
        break;
    }
    return state;
}

unsigned short col_paddle[8] = { ~0x0108,~0x0104,~0x0102,~0x0101,~0x0180,~0x0140,~0x0120,~0x0110 };
enum PaddleSM { P_INIT, P_START,P_PLAY };
unsigned short paddle;
int PaddleTick(int state) {
    static unsigned short x;
    static unsigned char i;
    switch (state) {
    case P_INIT:
        i = 3;
        paddle = 0x00;
        state = P_START;
        break;
    case P_START:
        state = game_on ? P_PLAY : P_START;
        break;
    case P_PLAY:
        state = game_on ? P_PLAY : P_START;
        break;
    default:
        state = P_INIT;
        break;
    }
    switch (state) {
    case P_INIT:
        break;
    case P_START:
        paddle = 0x00;
        i = 3;
        break;
    case P_PLAY:
        x = GetJoystickValue(0);
        x = x - 512;
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
        paddle = col_paddle[i];
        /*
        if (y <= 80) { ledy = 0x00; } //default
        else if (y >= 800) { ledy = 0x08; } //down
        else { ledy = 0x04; } //up
        PORTB = ledx | ledy;
        */
        break;
    default:
        break;
    }
    return state;
}

unsigned short col_1[8][8] = {
    {~0x8002,~0x4002,~0x2002,~0x1002,~0x0802,~0x0402,~0x0202,~0x0102}, //col 1
    {~0x8001,~0x4001,~0x2001,~0x1001,~0x0801,~0x0401,~0x0201,~0x0101}, //col 0
    {~0x8008,~0x4008,~0x2008,~0x1008,~0x0808,~0x0408,~0x0208,~0x0108}, //col 3
    {~0x8040,~0x4040,~0x2040,~0x1040,~0x0840,~0x0440,~0x0240,~0x0140}, //col 6
    {~0x8010,~0x4010,~0x2010,~0x1010,~0x0810,~0x0410,~0x0210,~0x0110}, //col 4
    {~0x8004,~0x4004,~0x2004,~0x1004,~0x0804,~0x0404,~0x0204,~0x0104}, //col 2
    {~0x8020,~0x4020,~0x2020,~0x1020,~0x0820,~0x0420,~0x0220,~0x0120}, //col 5
    {~0x8080,~0x4080,~0x2080,~0x1080,~0x0880,~0x0480,~0x0280,~0x0180} //col 7
};
enum LEDmatrixSM { LED_INIT, LED_START,LED_PLAY };
unsigned short led;
unsigned char src;
int LEDmatrixTick(int state) {
    static unsigned char i;
    static unsigned char j;
    switch (state) {
    case LED_INIT:
        i = 0;
        j = 0;
        src = 0;
        led = 0x00;
        state = LED_START;
        break;
    case LED_START:
        state = game_on ? LED_PLAY : LED_START;
        break;
    case LED_PLAY:
        if (game_on) {
            if (j >= 7) {
                i = (i + 1) % 8;
                j = 0;
            }
            else { j++; }
            state = LED_PLAY;
        }
        else { state = LED_START; }
        break;
    default:
        state = LED_INIT;
        break;
    }
    switch (state) {
    case LED_INIT:
        break;
    case LED_START:
        led = 0x00;
        src = 0;
        break;
    case LED_PLAY:
        send_data(col_1[i][j]);
        led = col_1[i][j];
        if (j >= 7) { src = 1; }
        else { src = 0; }
        break;
    default:
        break;
    }
    return state;
}

enum DisplaySM { D_INIT, D_START,D_PLAY,D_END };
unsigned char score;
int DisplayTick(int state) {
    switch (state) {
    case D_INIT:
        score = 1;
        end = 0;
        start_display = 0;
        game_on = 0;
        t = 0;
        time_limit = 0;
        state = D_START;
        break;
    case D_START:
        if (start) {
            score = 1;
            game_on = 1;
            end = 0;
            start_display = 0;
            t = 0;
            time_limit = 100;
            LCD_ClearScreen();
            state = D_PLAY;
        }
        else { state = D_START; }
        break;
    case D_PLAY:
        if (t <= time_limit) { state = D_PLAY; }
        else if (t > time_limit) {
            end = 1;
            start_display = 1;
            game_on = 0;
            state = D_END;
        }
        break;
    case D_END:
        if (end_display) {
            start_display = 0;
            end = 0;
            state = D_START;
        }
        else { state = D_END; }
        break;

    default:
        state = D_INIT;
        break;
    }
    switch (state) {
    case D_INIT:
        break;
    case D_START:
        LCD_DisplayString(1, "Press Start to  Play!!");
        break;
    case D_PLAY:
        if (src && led == paddle) { score++; }
        else if (src && led != paddle) { score--; }
        t++;
        LCD_Cursor(1);
        LCD_WriteData(high_score + '0');
        break;
    case D_END:
        break;
    default:
        break;
    }
    return state;
}

enum EEPROMSM { EEPROM_INIT, EEPROM_START, EEPROM_PLAY,EEPROM_END,EEPROM_NEW_SCORE };
int EEPROMTick(int state) {
    switch (state) {
    case EEPROM_INIT:
        state = EEPROM_START;
        break;
    case EEPROM_START:
        state = game_on ? EEPROM_PLAY : EEPROM_START;
        break;
    case EEPROM_PLAY:
        if ((!start_display) && (score == high_score)) { state = EEPROM_PLAY; }
        else if ((!start_display) && (score != high_score)) { state = EEPROM_NEW_SCORE; }
        else { state = EEPROM_END; }
        break;
    case EEPROM_NEW_SCORE:
        state = EEPROM_PLAY;
        break;
    case EEPROM_END:
        state = start_display ? EEPROM_END : EEPROM_START;
        break;
    default:
        state = EEPROM_INIT;
        break;
    }
    switch (state) {
    case EEPROM_INIT:
        break;
    case EEPROM_START:
        eeprom_update_byte((uint8_t*)64, 1);
        break;
    case EEPROM_PLAY:
        high_score = eeprom_read_byte((const uint8_t*)64);
        break;
    case EEPROM_NEW_SCORE:
        eeprom_update_byte((uint8_t*)64, score);
        break;
    case EEPROM_END:
        break;
    default:
        break;
    }
    return state;
}

enum LCDSM { LCD_INIT, LCD_START,LCD_WIN,LCD_WIN_1,LCD_LOSE,LCD_LOSE_1 };
int LCDTick(int state) {
    static unsigned char i;
    unsigned char reset = ~PINC & 0x02;
    switch (state) {
    case LCD_INIT:
        end_display = 0;
        i = 0;
        state = LCD_START;
        break;
    case LCD_START:
        if (start_display && score >= 7) { state = LCD_WIN; }
        else if (start_display && score < 7) { state = LCD_LOSE; }
        else { state = LCD_START; }
        break;
    case LCD_WIN:
        if (i <= 30) { state = LCD_WIN; }
        else {
            state = LCD_WIN_1;
            LCD_ClearScreen();
        }
        break;
    case LCD_WIN_1:
        state = LCD_START;
        break;
    case LCD_LOSE:
        if (reset == 0x02) {
            state = LCD_LOSE_1;
            LCD_ClearScreen();
        }
        else { state = LCD_LOSE; }
        break;
    case LCD_LOSE_1:
        state = LCD_START;
        break;
    default:
        state = LCD_INIT;
        break;
    }
    switch (state) {
    case LCD_INIT:
        break;
    case LCD_START:
        end_display = 0;
        i = 0;
        break;
    case LCD_WIN:
        LCD_DisplayString(1, "Congratulations!You won.");
        i++;
        break;
    case LCD_WIN_1:
        end_display = 1;
        break;
    case LCD_LOSE:
        LCD_DisplayString(1, "Game Over!Try   Again");
        break;
    case LCD_LOSE_1:
        end_display = 1;
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
    DDRC = 0x00; PORTC = 0xFF;
    DDRD = 0xFF; PORTD = 0x00;
    Joystick_init();
    LCD_init();

    tasks[i].state = S_INIT;
    tasks[i].period = periodStart;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &StartTick;
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
    ++i;

    tasks[i].state = D_INIT;
    tasks[i].period = periodDisplay;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &DisplayTick;
    ++i;

    tasks[i].state = EEPROM_INIT;
    tasks[i].period = periodEEPROM;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &EEPROMTick;
    ++i;

    tasks[i].state = LCD_INIT;
    tasks[i].period = periodLCD;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &LCDTick;

    TimerSet(tasksPeriodGCD);
    TimerOn();

    while (1) {}
    return 0;
}



