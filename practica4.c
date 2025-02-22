#include <16f877a.h>
#use delay(clock=4000000)
#include <lcd.c>
#include <stdio.h>
#include <stdint.h>
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

//Declaracion de Funciones
int sec, min, hour, day, month, year, cont, parameter, ndays, i, result;
int anti_rebote(void);
void blink(void);
void clock_calendar(void);
int establecer_fecha(int x, int y, int parameter);
int dec_to_bcd(int num);
int bcd_to_dec(int num);
void save_memory(void);
void load_memory(void);

int anti_rebote(void){ 
	int cont = 0;
	for(int i=0; i<5; i++){ //Retardo de 10ms
		if(pulse_0 == 0 || pulse_2 == 0){ //Main & alarm config.
			cont++;
			if(cont > 2){
				result = 1;
				return result;
			}else{
				result = 0;
				return result;
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
void blink(void){
	int j = 0;
	while( j<100 && pulse_0 && pulse_1){
		j++;
	}
}
//X & Y son las coordenadas en el LCD.
int establecer_fecha(int x, int y, int parameter){
	while(anti_rebote()); //Mientras devuelva un 1.
	while(true){
		while(!pulse_1){
			parameter++;
			if(i==0 && parameter>99){ //Years.
				parameter = 0;
			}if(i==1 && parameter>12){ //Months.
				parameter = 1;
			}if(i==2 && parameter > ndays){ //Days determined by the calc.
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

			if(!pulse_1)
			if(anti_rebote()){
				i++;
				return parameter;
			}
		}
	}
}

int ndays(int month, int year) { 
    if (month == 1) //Enero
        return (31); 

    if (month == 2) { //Febrero, si el ano es bisiesto Feb tiene 29 dias.
        if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) 
            return (29); 
        else
            return (28); 
    } 

    if (month == 3) //Marzo
        return (31); 

    if (month == 4) //Abril
        return (30); 

    if (month == 5) //Mayo
        return (31); 

    if (month == 6) //Junio
        return (30); 

    if (month == 7) //Julio
        return (31); 

    if (month == 8) //Agosto
        return (31); 

    if (month == 9) //Sep.
        return (30); 

    if (month == 10) //Oct.
        return (31); 

    if (month == 11) //Nov.
        return (30); 

    if (month == 12) //Dic.
        return (31);
} 

void save_memory(){
	write_eeprom(0, year);
	write_eeprom(1, month);
	write_eeprom(2, day);
	write_eeprom(3, hour);
	write_eeprom(4, min);
}

void load_memory(){
	year = read_eeprom(0);
	month = read_eeprom(1);
	day = read_eeprom(2);
	hour = read_eeprom(3);
	min = read_eeprom(4);
}

void main(void){
	set_tris_a(0xFF);
	set_tris_b(0);
	set_tris_d(0);
	lcd_init();
	while(true){
		if(!pulse_1)
		if(anti_rebote()){
			i = 0;
			hour = establecer_fecha(1,1,hour);
			min = establecer_fecha(4,1,min);
			sec = establecer_fecha(7,1,sec);
			year = establecer_fecha(3,2,year);
			month = establecer_fecha(6,2,month);
			day = establecer_fecha(9,2,day);
			while(anti_rebote());
			hour = dec_to_bcd(hour);
			min = dec_to_bcd(min);
			sec = dec_to_bcd(sec);
			year = dec_to_bcd(year);
			month = dec_to_bcd(month);
			day = dec_to_bcd(day);
			save_memory();
		}
		load_memory();

		clock_calendar();
	}
}
