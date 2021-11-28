#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define OUTPUT   PORTB
#define INPUT    DDRB
#define  SER PB0      
#define  SRCLK PB1      
#define  RCLK PB2      


void send_data(unsigned short data)
{
	for (unsigned short i = 0; i < 16; i++)
	{
		if (i < 8) {
			if (data & 0x8000)
			{
				OUTPUT = OUTPUT | (0x0001 << SER);
			}
			else
			{
				OUTPUT = OUTPUT & ~(0x0001 << SER);
			}
		}
		else {
			if (data & 0x8000) {
				OUTPUT = OUTPUT & ~(0x0001 << SER);
			}
			else {
				OUTPUT = OUTPUT | (0x0001 << SER);
			}
		}
		data = data << 1;
		
		OUTPUT = OUTPUT | (0x0001 << SRCLK);
		OUTPUT = OUTPUT & ~(0x0001 << SRCLK);
	}
	OUTPUT = OUTPUT | (0x0001 << RCLK);
	OUTPUT = OUTPUT & ~(0x0001 << RCLK);
}
#endif // !shift_register_H

