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

//Definicion de puertos de entrada.
#define pulse_0 PIN_A0	//To enter time config.
#define pulse_1 PIN_A1	//To change selected parameter.
#define pulse_2 PIN_A2	//To configure alarm.
#define pulse_3 PIN_A3	//To stop the alarm.

//Definicion de variables y funciones.
int sec, min, hour, day, month, year, cont, parameter, antirebote, ndays;
_int anti_rebote(void);
int blink(void);
int clock_calendar(void);
int establecer_fecha(int x, int y, int parameter);
int dec_to_bcd(int num);
int bcd_to_dec(int num);

int anti_rebote(void){ 
	int cont = 0;
	for(int i=0; i<5; i++){ //Retardo de 10ms
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

int clock_calendar(void){
	//Allocate a permanent memory with static_char 
	static char Time[] = "00:00:00";
	static char Date[] = "2000/00/00"; //YYYY/MM/DD

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
//Animacion de seleccion.
int blink(void){
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
			if(i==0 && parameter>99){ //Years.
				parameter = 0;
			}if(i==1 && parameter>12){ //Months.
				parameter = 1;
			}if(i==2 && parameter>ndays){ //Days determined by the calc.
				parameter = 1;
			}if(i==3 && parameter>23){ //Hours.
				parameter = 0;
			}if(i==4 && parameter>59){ //Minutes.
				parameter = 0;
			}
			LCD_GOTOXY(x,y);
			LCD_PUTC("  ");
			blink();
			LCD_GOTOXY(x,y);
			LCD_PUTC(parameter / 10 + '0');
			LCD_PUTC(parameter % 10 + '0');
			blink();

			if(!pulse_1){
				if(anti_rebote()){
					i++;
					return parameter;
				}
			}
		}
	}
}

nt ndays(int monthNumber, int year) 
{ 
    if (monthNumber == 0) //Enero
        return (31); 

    if (monthNumber == 1) { //Febrero
        // If the year is leap then Feb 
        // has 29 days 
        if (year % 400 == 0 
            || (year % 4 == 0 
                && year % 100 != 0)) 
            return (29); 
        else
            return (28); 
    } 

    if (monthNumber == 2) //Marzo
        return (31); 

    if (monthNumber == 3) //Abril
        return (30); 

    if (monthNumber == 4) //Mayo
        return (31); 

    if (monthNumber == 5) //Junio
        return (30); 

    if (monthNumber == 6) //Julio
        return (31); 

    if (monthNumber == 7) //Agosto
        return (31); 

    if (monthNumber == 8) //Septiembre
        return (30); 

    if (monthNumber == 9) //Octubre
        return (31); 

    if (monthNumber == 10) //Noviembre
        return (30); 

    if (monthNumber == 11) //Diciembre
        return (31);
} 



int main(){
	lcd_init();
	while(true){
		if(!pulse_1){
			if(anti_rebote()
}
