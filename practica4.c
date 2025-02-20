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


int sec, min, hour, day, month, year, cont, parameter, antirebote;
__bit anti_rebote(void);
void blink(void);
void clock_calendar(void);
int establecer_fecha(int x, int y, int parameter);
int dec_to_bcd(int num);
int bcd_to_dec(int num);

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
//Funcion de conversion de bcd a decimal.
int bcd_to_dec(int num){
	return((num >> 4) * 10 + (num & 0x0F)); //Retornamos
}
//Funcion de conversion de decimal a bcd.
int dec_to_bcd(int num){
	return(((num/10) << 4) + (num % 10)); // Retornamos
}

void clock_calendar(void){
	//Allocate a permanent memory with static_char 
	static_char Time[] = "00:00:00";
	static_char Date[] = "2000/00/00"; //YYYY/MM/DD

	sec = bcd_to_dec(sec);
	min = bcd_to_dec(min);
	hour = bcd_to_dec(hour);
	day = bcd_to_dec(day);
	month = bcd_to_dec(month);
	year = bcd_to_dec(year);
	
	//Se dispone ahora a guardar las variables en las posiciones del array.
	Time[0] = hour / 10 + '0';
	Time[1] = hour % 10 + '0';
	Time[3] = min / 10 + '0';
	Time[4] = min % 10 + '0';
	Time[6] = sec / 10 + '0';
	Time[7] = sec % 10 + '0';

	Date[2] = year / 10 + '0';
	Date[3] = year % 10 + '0';
	Date[5] = month / 10 + '0';
	Date[6] = month % 10 + '0';
	Date[8] = day / 10 + '0';
	Date[9] = day % 10 + '0';

	LCD_GOTOXY(1,1); //Cursor en fila 1, columna 1.
	LCD_PUTC(Time);  //Imprime el array Time.
	LCD_GOTOXY(1,2); //Cursor en fila 2, columna 1.
	LCD_PUTC(Date);  //Imprime el array Date.
}

void blink(void){
	int j = 0;
	while( j<100 && pulse_0 && pulse_1){
		j++;
	}
}

int establecer_fecha(int x, int y, int parameter){
	while(anti_rebote()); //Mientras devuelva un 1.
	while(true){
		while(!pulse_1){
			parameter++;
			if



int main(){
	lcd_init();
	lcd_putc("\f");
}
