#include <16F877A.h> 
#fuses XT,NOWDT,NOPUT,NOLVP,NOPROTECT,BROWNOUT,NOCPD
#use delay(clock=4000000)
#define use_portb_lcd TRUE //Se habilita el PORTB para conectar el LCD.
#include <LCD.c>

#byte PORTC=7
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
         S1=0;
         S2++;
         if(S2==6){//Overflow de segundo a minutos.
            S2=0;
            M++;
            if(M>59){//Overflow de de minutos a horas.
              M=0;
              h++;   
            }
            if(h==24){//Overflow de horas a dias.
              h=00;
              DD++;
              }
         }
        }
     //Se establecen los dias para el ano bisiesto.
        if(MT==2){
            if(AA%4==0){
                if(DD>29){
                    DD=1;
                    MT++;
                }
            }else{
                if(DD>28){
                  DD=1;
                  MT++;
                }
            }
        }
       //Se establecen los meses que contienen hasta el dia 31.
        if(MT==1 || MT==3 || MT==5 || MT==7 || MT==8 || MT==10 || MT==12){
            if(DD>31){
                DD=1;
                MT++;
            }
        }
       //Se establecen los meses que contienen hasta el dia 30.
        if(MT==4 || MT==6 || MT==9 || MT==11){
            if(DD>30){
                DD=1;
                MT++;
            }
        }
       //Overflow de mes, a anos.
        if(MT>12){
            MT=1;
            AA++;
        } 
    } 
   set_timer0(217);
}

#INT_TIMER2
void TIMER2_isr(void){

 desbordamiento++;
   if(desbordamiento==100)
   {
      desbordamiento=0;
 if(MT==2)
               {
        
                  if(AA%4==0)
                  {
                     if(DD>29)
                     {
                        DD=1;
                     }
                  }
                  else
                  {
                     if(DD>28)
                     {
                        DD=1;
                     }
                  }
               }
             
               if(MT==1 || MT==3 || MT==5 || MT==7 || MT==8 || MT==10 || MT==12)
               {
                  if(DD>31)
                  {
                     DD=1;
                  }
               }
               if(MT==4 || MT==6 || MT==9 || MT==11)
               {
                  if(DD>30)
                  {
                     DD=1;
                  }
               }
              if(MT>12)
               {
                  MT=1;
               } 
            }
        
            
            
       set_timer0(210);
}       

void guardar_alarma(){
 WRITE_EEPROM(16, hr_alarma);
 WRITE_EEPROM(17,m_alarma);
 WRITE_EEPROM(19,h_alarma);
 WRITE_EEPROM(20, 'A');  // Escribir el primer byte en la dirección 0
WRITE_EEPROM(21, 'M');
write_eeprom(22, '\0');// Escribir el segundo byte en la dirección 1
WRITE_EEPROM(23, 'P'); // Escribir el byte nulo en la dirección 2 para indicar el final de la cadena
WRITE_EEPROM(24, 'M'); // Escribir el byte nulo en la dirección 2 para indicar el final de la cadena
write_eeprom(25, '\0');
}
void leeralarma(){
      hr_alarma=READ_EEPROM(16);
      m_alarma=READ_EEPROM(17);
      h_alarma=READ_EEPROM(19);
      strcpy(ampm_alarma, READ_EEPROM(20));
      strcpy(ampm_alarma, READ_EEPROM(23));
}
void leerfecha(){
  DD = READ_EEPROM(13);
       MT = READ_EEPROM(14);
        AA = READ_EEPROM(15);
}
void reloj(){
      lcd_gotoxy(1,1);
      if(h<12){
      ampm="AM";
      if(h==0){
      hr=h+12;
      printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
 
      }
      else
       printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);

      }
      else if(h==12) {
      ampm="PM";
        printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
  
      }
      else if(h>12 && h<24){
      ampm="PM";
      hr=h-12;
       printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
   
      }
      else if(h>=24){
      ampm="AM";
      h=0;
      hr=h+12;
        printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
      }     
}
void alarma(){  
lcd_gotoxy(1,2);
      if(h_alarma<12){
    ampm_alarma="AM";
      if(h_alarma==0){
      hr_alarma=h_alarma+12;
      printf (lcd_putc,"%02d:%02d %s",hr_alarma,m_alarma,ampm_alarma);
 
      }
      else
      printf(lcd_putc,"%02d:%02d %s",h_alarma,m_alarma,ampm_alarma);
  
      }
      else if(h_alarma==12) {
      ampm_alarma="PM";
      printf (lcd_putc,"%02d:%02d %s",h_alarma,m_alarma,ampm_alarma);
      write_eeprom(6,ampm);
      }
      else if(h_alarma>12 && h_alarma<24){
      ampm_alarma="PM";
      hr_alarma=h_alarma-12;
      printf (lcd_putc,"%02d:%02d %s",hr_alarma,m_alarma,ampm_alarma);

      }
      else if(h_alarma>=24){
      ampm_alarma="AM";
      h_alarma=0;
      hr_alarma=h_alarma+12;
      printf(lcd_putc, "%02d:%02d %s",hr_alarma,m_alarma,ampm_alarma);

      }
}
void mostrar_fecha(){
   
  lcd_gotoxy(1,2);
            printf(lcd_putc,"%02i",DD);
             lcd_gotoxy(3,2);
            lcd_putc("/");
            lcd_gotoxy(4,2);
            printf(lcd_putc,"%02i",MT);
            lcd_gotoxy(6,2);
            lcd_putc("/");
            lcd_gotoxy(7,2);
            printf(lcd_putc,"20%02i",AA);
}
void ajuste_fecha(){
if(input(PIN_A1))
         {
         
          delay_ms(150);
               
               DD++;
               
               if(MT==2)
               {
               
                  if(AA%4==0)
                  {
                     if(DD>29)
                     {
                        DD=1;
                     }
                  }
                  else
                  {
                     if(DD>28)
                     {
                        DD=1;
                     }
                  }
               }
             
               if(MT==1 || MT==3 || MT==5 || MT==7 || MT==8 || MT==10 || MT==12)
               {
                  if(DD>31)
                  {
                     DD=1;
                  }
               }
               if(MT==4 || MT==6 || MT==9 || MT==11)
               {
                  if(DD>30)
                  {
                     DD=1;
                  }
               }
               
            }
          if(input(PIN_A2))
         {
               delay_ms(150);
               
               MT++;
               
               if(MT>12)
               {
                  MT=1;
               }
               
            } 
           if(input(PIN_A3))
         {
         delay_ms(150);
               
               AA++;
               
               if(AA>99)
               {
                  AA=00;
               }
               
            } 
  
          mostrar_fecha(); 
}
void ajuste_alarma(){
   lcd_gotoxy(1,1); //alarma
            lcd_putc("AJUSTE ALARMA:");
            lcd_gotoxy(6,2);
            printf(lcd_putc,"");
             if (input(pin_A2) == 1) { 
            m_alarma++;
            if (m_alarma > 59) {
            m_alarma = 0;
            }
            if (h_alarma == 0) { 
            hr_alarma = h_alarma + 12; 
            printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma,ampm_alarma);
            } else if (h_alarma >= 13 && h_alarma <= 23) { 
            hr_alarma = h_alarma - 12; 
            printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma ,ampm_alarma); 
            } else { 
            printf(lcd_putc, "%02d:%02d %s", h_alarma, m_alarma,ampm_alarma);
            }
            delay_ms(100);
            }
            
             if (input(pin_A1) == 1) { //alarma
            h_alarma++;
            if (h_alarma < 12) {
            ampm_alarma = "AM";
            if (h_alarma == 0) {
            hr_alarma = h_alarma + 12;
            printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma,ampm_alarma);
            } else {
            printf(lcd_putc, "%02d:%02d %s", h_alarma, m_alarma,ampm_alarma);
            }
            } else if (h_alarma == 12) { 
            ampm_alarma = "PM";
            printf(lcd_putc, "%02d:%02d %s", h_alarma, m_alarma,ampm_alarma);
            } else if (h_alarma > 12 && h_alarma < 24) {
            ampm_alarma = "PM";
            hr_alarma = h_alarma - 12;
            printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma,ampm_alarma);
            } else if (h_alarma >= 24) {
            ampm_alarma = "AM";
            h_alarma = 0;
            hr_alarma = h_alarma + 12;
            printf(lcd_putc, "%02d:%02d %s", hr_alarma, m_alarma,ampm_alarma);
            }
            delay_ms(100);
        
            } 
}
void ajuste_hora() {
 lcd_gotoxy(1,1); 
  lcd_putc("AJUSTE HORA:");    
  lcd_gotoxy(6,2);
    printf(lcd_putc,"");
         if (input(pin_A3) == 1) {
    S1++;
   
    if (S1 == 10) {
        S1 = 0;
        S2++;
    }
        if (S2 == 6) {
            S2 = 0;
        }
        delay_ms(70);
       
      }  
                    if (input(pin_A2) == 1) {
    M++;
   if(M>59){
   M=0;
   }           
    }
            if (input(pin_A1) == 1) { 
               h++;
               if (h < 12) {
                  ampm = "AM";
                  if (h == 0) {
                     hr = h + 12;
                        printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
                  } 
                  else {
              printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
                  }
               } 
               else if (h == 12) {
                  ampm = "PM";
                      printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
               } 
               else if (h > 12 && h < 24) {
                  ampm = "PM";
                  hr = h - 12;
                      printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
               } 
               else if (h >= 24) {
                  ampm = "AM";
                  h = 0;
                  hr = h + 12;
                       printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
               }
               delay_ms(120);
            }
 if (h < 12) {
                  ampm = "AM";
                  if (h == 0) {
                     hr = h + 12;
                      printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
                  } 
                  else {
             printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
                  }
               } 
               else if (h == 12) {
                  ampm = "PM";
                     printf (lcd_putc,"%02d:%02d:%u%u %s",h,M, S2, S1, ampm);
               } 
               else if (h > 12 && h < 24) {
                  ampm = "PM";
                  hr = h - 12;
                      printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
               } 
               else if (h >= 24) {
                  ampm = "AM";
                  h = 0;
                  hr = h + 12;
                       printf (lcd_putc,"%02d:%02d:%u%u %s",hr,M, S2, S1, ampm);
               }
               delay_ms(250);
}
void ajuste(){
int8 valh1=0,valh2=0,valh3=0,valh4=0; //Variables de control de ajuste
if (input(pin_A0) == 1) {
delay_ms(400);
valh3=1;
}
            do {
                if(valh3==1){
                S1=0;
                S2=0;
         valh4=1;
          LCD_PUTC('\f');
            }

            while (valh3==1 && valh4==1) {
             disable_interrupts(int_timer0);
                delay_ms(100);
                ampm = "AM"; // inicializar ampm con "AM"    
                ajuste_hora();
   
                  if (input(pin_A0) == 1) {
                  disable_interrupts(int_timer0);
                    valh4 = 0;
                    valh1= 1;
                     LCD_PUTC('\f');
                }
                   }
            while (valh1 == 1) {
            disable_interrupts(int_timer0);
             delay_ms(300);
             ajuste_alarma();
                 
                  if (input(pin_A0) == 1) {
                  disable_interrupts(int_timer0);
                   guardar_alarma();
                    valh1 = 0;
                     valh2 = 1;
                     if (DD != 1 || MT != 1|| AA != 0) {
                   DD=1;
                     MT=1;
                       AA=0;
                     }
                     LCD_PUTC('\f');
                }
              
            }
    while (valh2 == 1) { 
      disable_interrupts(int_timer0);
      ENABLE_interrupts(int_timer2);
      delay_ms(200);
    LCD_GOTOXY(1,1);
    LCD_PUTC("AJUSTE FECHA");
    ajuste_fecha();

        if (input(pin_A0) == 1) {
      ENABLE_interrupts(int_timer0);
         WRITE_EEPROM(13,DD);
         WRITE_EEPROM(14,MT);
         WRITE_EEPROM(15,AA);
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
      valh11=0;
      LCD_PUTC('\f');
      }
   }
 }
void main() {
 
   configuracion();
   
   configuracion2();
   

   //Puerto de alarma
    set_tris_c(0x00);
   //Puerto de los botones
   set_tris_b(0x07);
   //Puerto del lcd
   set_tris_d(0x00);
   //Establecemos puerto D como salida
   output_d(0x00);
    
   // Inicializamos la lcd
   lcd_init();
    //leer el valor de la alarma guardada
   leeralarma();
    //leer el valor de la fecha guardada
   leerfecha();
   //LED DE ALARMA(SPEAKER) apagado 
   BIT_CLEAR(LEDALAR);
            
while (TRUE) {
  disable_interrupts(int_timer2); 
 //AJUSTE
  ajuste();
    //RELOJ
      reloj();
       //ALARMA
         alarma();
         
     //ENCENDER LED SI SE CUMPLEN RELOJ=ALARMA
       if((hr_alarma == hr) && (m_alarma == M)) {
                BIT_SET(LEDALAR);
      }
      else{
          BIT_CLEAR(LEDALAR);
            lcd_gotoxy(2,2);
             printf (lcd_putc," ");
             }
                 //MODO HORA/CALENDARIO
                    modo_hora_fecha();     
      }    
}   
