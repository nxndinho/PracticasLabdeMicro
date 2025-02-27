#include <16F877A.h> 
#fuses XT,NOWDT,NOPUT,NOLVP,NOPROTECT,BROWNOUT,NOCPD
#use delay(clock=4000000)
#define use_portb_lcd TRUE //Se habilita el PORTB para conectar el LCD.
#include <LCD.c>

#byte PORTA = 5
#byte PORTB = 6
#byte PORTC = 7
#define LEDALAR PORTC,4

//Declaracion de variables
int8 valh11=0;
int h=0,hr=0,S1=0,S2=0,M=0; 
int m_alarma=0,h_alarma=0,hr_alarma=0;
int DD=1,MT=1,AA=0;
char ampm[3] = "AM";
char ampm_alarma[3];
int8 desbordamiento=0;

//Se configuran los timers e interrupciones del programa a 10ms
void configuracion2(void){
  setup_timer_2(T2_DIV_BY_16,209,3);  
   set_timer0(210);                             
   enable_interrupts(global);                   
   enable_interrupts(int_timer2);               
 }
void configuracion(void){
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_256|RTCC_8_BIT);    
   set_timer0(217);                             
   enable_interrupts(global);                   
   enable_interrupts(int_timer0);               
 }

#int_timer0
void timer_reloj(void)
{
   desbordamiento++;
   if(desbordamiento==100){ //Establece para contar a S1 cada 1s.
     desbordamiento=0;
     S1++;
      if(S1==10){//Overflow para el primer digito de cada segundo.
         S1 = 0;
         S2++;
         if(S2==6){//Overflow de segundo a minutos.
            S2=0;
            M++;
            if(M>59){//Overflow de de minutos a horas.
              M = 0;
              h++;   
            }
            if(h==24){//Overflow de horas a dias.
              h = 00;
              DD++;
            }
         }
    }
     //Se establecen los dias para el ano bisiesto.
        if(MT==2){
            if(AA%4==0){
                if(DD>29){
                    DD = 1;
                    MT++;
                }
            }else{ //Si no se cumple la condicion seran 28.
                if(DD>28){
                  DD = 1;
                  MT++;
                }
            }
        }
       //Se establecen las condicionales para los dias 31.
        if(MT==1 || MT==3 || MT==5 || MT==7 || MT==8 || MT==10 || MT==12){
            if(DD>31){
                DD = 1;
                MT++;
            }
        }
       //Se establecen las condicionales para los dias 30.
        if(MT==4 || MT==6 || MT==9 || MT==11){
            if(DD>30){
                DD = 1;
                MT++;
            }
        }
       //Overflow de mes, a anos.
        if(MT>12){
            MT = 1;
            AA++;
        } 
    } 
   set_timer0(217);
}

#INT_TIMER2
void TIMER2_isr(void){

    desbordamiento++;
    if(desbordamiento==100){
        desbordamiento=0;
        if(MT==2){
            if(AA%4==0){
                if(DD>29){
                    DD = 1;
                }
            }else{
                if(DD>28){
                    DD = 1;
                }
            }
        }
        if(MT==1 || MT==3 || MT==5 || MT==7 || MT==8 || MT==10 || MT==12){
            if(DD>31){
                DD = 1;
            }
        }
        if(MT==4 || MT==6 || MT==9 || MT==11){
                if(DD>30){
                    DD = 1;
                }
            }
            if(MT>12){
                MT = 1;
            } 
    } 
       set_timer0(210);
}       
//Funcion para guardar variables de alarma en la EEPROM.
void guardar_alarma(){
    write_eeprom(16, hr_alarma);
    write_eeprom(17,m_alarma);
    write_eeprom(19,h_alarma);
    write_eeprom(20, 'A');
    write_eeprom(21, 'M');
    write_eeprom(22, '\0');
    write_eeprom(23, 'P'); 
    write_eeprom(24, 'M'); 
    write_eeprom(25, '\0');
}
//Funcion de lectura de EEPROM para las variables de alarma.
void leeralarma(){
    hr_alarma=read_eeprom(16);
    m_alarma=read_eeprom(17);
    h_alarma=read_eeprom(19);
    strcpy(ampm_alarma, read_eeprom(20)); //Si la condicion es AM.
    strcpy(ampm_alarma, read_eeprom(23)); //Si la condicon es PM.
}
//Funcion de lectura de fecha.
void leerfecha(){
  DD = read_eeprom(13); //Dia.
  MT = read_eeprom(14); //Mes.
  AA = read_eeprom(15); //Ano.
}
//Funcion para el display del reloj en el LCD.
void reloj(){
    lcd_gotoxy(1,1); //Columna 1, Fila 1.
    //Print para AM.
        if(h<12){
            ampm = "AM";
            if(h==0){
                hr = h+12;
                printf(lcd_putc,"%02d:%02d:%u%u %s", hr, M, S2, S1, ampm);
            }else
                printf(lcd_putc,"%02d:%02d:%u%u %s", h, M, S2, S1, ampm);
        }
        //Print para PM
        else if(h==12) {
            ampm = "PM";
            printf(lcd_putc,"%02d:%02d:%u%u %s", h, M, S2, S1, ampm);
        }else if(h>12 && h<24){
            ampm = "PM";
            hr = h-12;
            printf(lcd_putc,"%02d:%02d:%u%u %s", hr, M, S2, S1, ampm);
      }else if(h>=24){ //Overflow de PM a AM en el display.
            ampm = "AM";
            h = 0;
            hr = h+12;
            printf (lcd_putc,"%02d:%02d:%u%u %s", hr, M, S2, S1, ampm);
      }     
}
//Funcion para el display de alarma en el LCD.
void alarma(){  
    lcd_gotoxy(1,2); //Columna 1, Fila 2.
    //Print para AM en la alarma.
        if(h_alarma<12){
            ampm_alarma="AM";
            if(h_alarma==0){
                hr_alarma=h_alarma+12;
                printf (lcd_putc,"%02d:%02d %s", hr_alarma, m_alarma, ampm_alarma);
            }else
                printf(lcd_putc,"%02d:%02d %s", h_alarma, m_alarma, ampm_alarma);
        }
        //Print para PM en la alarma.
        else if(h_alarma==12) {
            ampm_alarma = "PM";
            printf (lcd_putc,"%02d:%02d %s", h_alarma, m_alarma, ampm_alarma);
            write_eeprom(6,ampm);
        }else if(h_alarma>12 && h_alarma<24){
            ampm_alarma = "PM";
            hr_alarma=h_alarma-12;
            printf (lcd_putc,"%02d:%02d %s", hr_alarma, m_alarma, ampm_alarma);
        }else if(h_alarma>=24){
            ampm_alarma="AM";
            h_alarma=0;
            hr_alarma=h_alarma+12;
            printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma, ampm_alarma);
      }
}
//Funcion para display de fecha en el LCD 
void mostrar_fecha(){
    lcd_gotoxy(1,2); //Columna 1, Fila 2.
        printf(lcd_putc,"%02i",DD); //Dias.
        lcd_gotoxy(3,2); //Columna 3, FIla 2.
        lcd_putc("/");
        lcd_gotoxy(4,2); //Columna 4, Fila 2.
        printf(lcd_putc,"%02i",MT); //Mes.
        lcd_gotoxy(6,2);//Columna 6, Fila 2.
        lcd_putc("/");
        lcd_gotoxy(7,2); //Columna 7, Fila 2.
        printf(lcd_putc,"20%02i",AA); //Ano.
}
//Funciona de ajuste de fecha.
void ajuste_fecha(){
    //Boton A1 para la config. de dias.
    if(input(PIN_A1)){
        delay_ms(150);
        DD++;
        //Ajuste de seleccion de dia para ano bisiesto.
        //Mes de Febrero.
        if(MT==2){
            if(AA%4==0){
                if(DD>29){ //Al sobrepasar 29, volver a 1.
                    DD=1;
                }
            }else{
                if(DD>28){ //Al sobrepasar 28, volver 1.
                    DD=1;
                }
            }
        }
        //Ajuste de seleccion para los meses con 31 dias.
        if(MT==1 || MT==3 || MT==5 || MT==7 || MT==8 || MT==10 || MT==12){
            if(DD>31){ //Al sobrepasar volver a 1.
                DD=1;
                  }
               }
        //Ajuste de seleccion para los meses con 30 dias.
        if(MT==4 || MT==6 || MT==9 || MT==11){
            if(DD>30){
                DD=1;
            }
        }
               
    }
    //Boton A2 para la config. de mes.
    if(input(PIN_A2)){
        delay_ms(150);
        MT++;
        if(MT>12){ //Al sonbre pasar 12, volver a 1.
            MT=1;
        }
    } 
    //Boton A3 para la config. de ano.
    if(input(PIN_A3)){
        delay_ms(150);
        AA++;
        if(AA>99){ //Al sobre pasar volver 99, volver a 00.
            AA=00; //Ya que el ano parte del ano 2000.
        }
    }
    //Al ya ser asignados llama a la funcion para mostrar
    //en el display de forma permanente.
    mostrar_fecha();
}
//Funcion ajuste de alarma
void ajuste_alarma(){
    lcd_gotoxy(1,1); //Columna 1, Fila 1.
            lcd_putc("AJUSTE ALARMA:");
            lcd_gotoxy(6,2); //Columna 6, Fila 2.
            printf(lcd_putc,"");
            //Boton A2 para el ajuste de minuto de alarma.
            if(input(pin_A2) == 1){ 
                m_alarma++;
                if(m_alarma > 59){ //Reset a 0, si excede 59 min.
                    m_alarma = 0;
                }
                if(h_alarma == 0){ 
                    hr_alarma = h_alarma + 12; 
                    printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma,ampm_alarma);
                }else if(h_alarma >= 13 && h_alarma <= 23){ //Limite para mantener el valor de AM.
                    hr_alarma = h_alarma - 12; 
                    printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma ,ampm_alarma); 
                }else{ 
                    printf(lcd_putc, "%02d:%02d %s", h_alarma, m_alarma,ampm_alarma);
                }
                delay_ms(100);
            }
            //Boton A1 para el ajuste de hora de alarma.
            if (input(pin_A1) == 1){
                h_alarma++;
            if (h_alarma < 12) { //Display de AM
                ampm_alarma = "AM";
            if (h_alarma == 0) {
                hr_alarma = h_alarma + 12;
                printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma,ampm_alarma);
            }else{
                printf(lcd_putc, "%02d:%02d %s", h_alarma, m_alarma,ampm_alarma);
            }
            }else if(h_alarma == 12){ //Display de PM
                ampm_alarma = "PM";
                printf(lcd_putc, "%02d:%02d %s", h_alarma, m_alarma,ampm_alarma);
            }else if(h_alarma > 12 && h_alarma < 24){ //Limite para mantener el valor de PM
                ampm_alarma = "PM";
                hr_alarma = h_alarma - 12;
                printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma,ampm_alarma);
            }else if(h_alarma >= 24){ //Overflow a AM
                ampm_alarma = "AM";
                h_alarma = 0;
                hr_alarma = h_alarma + 12;
                printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma,ampm_alarma);
            }
            delay_ms(100);
            } 
}
//Funcion de ajuste de hora.
void ajuste_hora() {
    lcd_gotoxy(1,1); //Columna 1, Fila 1.
    lcd_putc("AJUSTE HORA:");    
    lcd_gotoxy(6,2); //Columna 6, Fila 2.
    printf(lcd_putc,"");
    //Boton A3 para ajuste de segundos.
    if(input(pin_A3) == 1){
        S1++;
        if (S1 == 10) { //Overflow del primer digito de segundos.
            S1 = 0;
            S2++;
        }
        if(S2 == 6){ //Reset del segundo digito para no exceder 60s.
            S2 = 0;
        }
        delay_ms(70);
    }  
    //Boton A2 para ajuste de de minutos.
    if(input(pin_A2) == 1){
        M++;
        if(M>59){ //Reset del contador si excede a 59m.
            M=0;
        }           
    }
    //Boton A1 para ajuste de horas.
    if(input(pin_A1) == 1){ 
        h++;
        if(h < 12){ //Indicador de AM.
        	ampm = "AM";
            if (h == 0){
                hr = h + 12;
                printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
            }else{
                printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
            }
        }else if(h == 12){ //Indicador de PM
            ampm = "PM";
            printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
        }else if(h > 12 && h < 24) { //Limite para mantener el valor de PM.
            ampm = "PM";
            hr = h - 12;
            printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
        } else if(h >= 24){ //Overflow a AM al exceder.
            ampm = "AM";
            h = 0;
            hr = h + 12;
            printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
        }
        delay_ms(120);
    }
    //Chequear funcion!!!
    if(h < 12){
        ampm = "AM";
        if(h == 0){
            hr = h + 12;
            printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
        }else{
            printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
        }
    }else if(h == 12){
        ampm = "PM";
        printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
    }else if(h > 12 && h < 24){
        ampm = "PM";
        hr = h - 12;
        printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
    }else if(h >= 24){
        ampm = "AM";
        h = 0;
        hr = h + 12;
        printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
    }
    delay_ms(250);
}
//Funcion de ajuste master.
void ajuste(){
    int8 valh1=0,valh2=0,valh3=0,valh4=0; //Variables de control de ajuste
    if(input(pin_A0) == 1){
        delay_ms(400);
        valh3 = 1;
    }
    do{
        if(valh3==1){
        S1 = 0;
        S2 = 0;
        valh4 = 1;
        LCD_PUTC('\f');
        }
        while(valh3==1 && valh4==1){
            disable_interrupts(int_timer0);
            delay_ms(100);
            ampm = "AM"; // inicializar ampm con "AM"    
            ajuste_hora();
            if(input(pin_A0) == 1){
                disable_interrupts(int_timer0);
                    valh4 = 0;
                    valh1 = 1;
                    LCD_PUTC('\f');
            }
        }
        while(valh1 == 1){
            disable_interrupts(int_timer0);
            delay_ms(300);
            ajuste_alarma();
                 
            if(input(pin_A0) == 1){
                disable_interrupts(int_timer0);
                guardar_alarma();
                valh1 = 0;
                valh2 = 1;
                if(DD != 1 || MT != 1|| AA != 0){
                    DD=1;
                    MT=1;
                    AA=0;
                }
                LCD_PUTC('\f');
            }  
        }
        
        while(valh2 == 1){ 
            disable_interrupts(int_timer0);
            ENABLE_interrupts(int_timer2);
            delay_ms(200);
            LCD_GOTOXY(1,1); //Columna 
            LCD_PUTC("AJUSTE FECHA");
            ajuste_fecha();
                if(input(pin_A0) == 1){
                    ENABLE_interrupts(int_timer0);
                    //Guardado de Fecha a la memoria.
                    write_eeprom(13,DD);
                    write_eeprom(14,MT);
                    write_eeprom(15,AA);
                    valh3= 0;
                    valh4= 0;
                    valh2= 0;
                    LCD_PUTC('\f');
                    delay_ms(400);
                }
        }
    }  
    while(valh3==1);{}         
}
//Cambio de fila 2 para mostrar la fecha
void modo_hora_fecha(){
    if (input(pin_A3) == 1) { //modo hora/fecha 
        delay_ms(450);
        valh11 = 1;
        LCD_PUTC('\f');
    }
    while (valh11 == 1) { 
        mostrar_fecha();
        reloj(); 
        if(input(pin_A3) == 1){
            delay_ms(450);
            valh11 = 0;
            LCD_PUTC('\f');
        }
    }
}

void main() {
    configuracion();
    configuracion2();
    set_tris_c(0x00); //LED Alarma.
    set_tris_a(0xFF); //Botones.
    set_tris_b(0x00); //LCD Display.
    output_b(0x00);

    lcd_init();
    leeralarma();
    leerfecha();
    BIT_CLEAR(LEDALAR); //LED Alarma inicia apagada
            
    while (TRUE) {
        disable_interrupts(int_timer2);
        //Llama las funciones y se ejecutan.
        ajuste();
        reloj();
        alarma();
        //Endendido de LED cuando se cumpla la hora y minuto indicado.
        if((hr_alarma == hr) && (m_alarma == M)) {
            BIT_SET(LEDALAR);
        }else{
            BIT_CLEAR(LEDALAR);
            lcd_gotoxy(2,2); //Columna 2, Fila 2
            printf (lcd_putc," ");
        }
            modo_hora_fecha();     
    }    
}   
