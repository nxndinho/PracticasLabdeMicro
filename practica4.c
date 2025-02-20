#include <16f877a.h>
#include <lcd.c>
#include <stdio.h>
#use delay(clock=4000000)
#define USE_PORTB_LCD TRUE	//Habilita uso en PORTB para LCD
#fuses xt, noprotect, nowdt, nobrownout, put, nolvp
#BYTE PORTA = 5
#BYTE PORTB = 6
#BYTE PORTD = 8

#define pulse_0 PIN_A0
#define pulse_1 PIN_A1
#define pulse_2 PIN_A2
#define pulse_3 PIN_A3

int second, minute, hour, day, month, year;

int main(){
	lcd_init();
	lcd_putc("\f");
}
