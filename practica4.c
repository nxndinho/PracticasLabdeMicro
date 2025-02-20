#include <16f877a.h>
#include <lcd.c>
#include <stdio.h>
#include <stdint.h>
#use delay(clock=4000000)
#define USE_PORTB_LCD TRUE	//Habilita uso en PORTB para LCD
#fuses xt, noprotect, nowdt, nobrownout, put, nolvp
#BYTE PORTA = 5
#BYTE PORTB = 6
#BYTE PORTD = 8

#define pulse_0 PIN_A0	//To enter time config.
#define pulse_1 PIN_A1	//To change selected parameter.
#define pulse_2 PIN_A2	//To configure alarm.
#define pulse_3 PIN_A3	//To stop the alarm.


int second, minute, hour, day, month, year, cont, parameter, antirebote;
__bit anti_rebote(void);
void blink(void);
void clock_calendar(void);


__bit anti_rebote(void){ //Esta funcion no generara una funcion impermicible.
	int cont = 0;
	for(int i=0; i<10; i++){ //Retardo de 10ms
		if(pulse_0 == 0 || pulse_2 == 0){ //Main & alarm config.
			cont++;
			if(cont > 2){
				return 1;
			}else{
				return 0;
			}
		}
	}
}
		
void clock_calendar(void){
	//Allocate a permanent memory with static_char 
	static_char Time[] = "00:00:00";
	static_char Date[] = "00/00/2000";
	

int establecer_fecha(int x, int y, int parameter){


int main(){
	lcd_init();
	lcd_putc("\f");
}
