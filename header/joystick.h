#ifndef JOY_STICK_H
#define JOT_STICK_H

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

#endif // !#JOY_STICK_H

