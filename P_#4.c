
#include <16F877A.h>
#FUSES XT,NOWDT,NOPROTECT,NOBROWNOUT, NOLVP
#USE DELAY(CLOCK=4000000)
#BYTE PORTA = 5
#BYTE PORTB = 6
#BYTE PORTC = 7
#BYTE PORTD = 8
#BYTE PORTE = 9


BYTE CONST DISPLAY[10]= {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x67};


int DISPN, MODE, FLAGS, SWFLAGS, BTNCNT;
int MIN, HOUR, SEC, AMIN, AHOUR;
int DAY, MONTH;
int YEAR;
int COUNTSEC;
int MONTHARRAY[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


#DEFINE BUZZER PORTC,0
#DEFINE LED PORTC,1


#DEFINE SW_HOLD SWFLAGS,0
#DEFINE SW1 SWFLAGS,1
#DEFINE SW2 SWFLAGS,2
#DEFINE SW3 SWFLAGS,3
#DEFINE SW4 SWFLAGS,4
#DEFINE SW_OK SWFLAGS,5
#DEFINE SW_NOTRELEASED SWFLAGS,6

#DEFINE AM FLAGS,0
#DEFINE AMA FLAGS,1
#DEFINE STOP_SECTIMER FLAGS,2
#DEFINE ALARM_ON FLAGS,3
#DEFINE ALARM_MUTE FLAGS,4
#DEFINE CHANGES_MADE FLAGS,5
#DEFINE MODE_CHANGED FLAGS,6
#DEFINE BUZZ_ACT FLAGS,7

#INCLUDE <LCD.C>

#INT_RTCC
   void TIMER_2500uS()                           //rutima de interrupción del RTCC (TMR0) para 2500uS
  {
   DELAY_US(2);                              //completa los 5000uS
   SET_RTCC(102);                              //prepara el RTCC para temporizar los proximos 2500uS
   DISPN++;                                 //cambia a la proxima posicion del multiplexado
  }

void DATECALC(){ //Función para calcular la fecha. Es llamada por la función TIMECALC(), cada vez que se incrementa el día.
      
   if((YEAR % 4 == 0 && YEAR % 100 != 0) || (YEAR % 100 == 0 && YEAR % 400 == 0)) { //Verificación de año bisiesto, 100% precisa.
   MONTHARRAY[1] = 29; //Establecer el último día de Febrero (índice 1 de MONTHARRAY[12]) en 29
   }
   else{
   MONTHARRAY[1] = 28; //Si no es bisiesto, ponerlo en 28.
   }

   if(DAY > MONTHARRAY[MONTH-1]){ //Se compara el día actual con el último día del índice [MONTH -1] del arreglo MONTHARRAY[12]; Si MONTH = 2, el programa está en el 2do mes, febrero, y compara eso con el índice [2 - 1] = [1] de MONTHARRAY[12], que corresponde
      //Al último día del mes de Febrero.
      
   DAY = 1; //Se establece el día en 1 cuando la variable DAY sobrepasa el índice [MONTH -1] de MONTHARRAY, y se incrementa el mes. 
   MONTH++;
   }

   if(MONTH >= 13){
   MONTH = 1; //Se resetea el mes si llega a 13, y se incrementa el año.
   YEAR++;
   }

}

void ALARMTONE(){ //Rutina para activar la alarma.
   if(!BIT_TEST(ALARM_ON) || BIT_TEST(ALARM_MUTE)){
   SET_PWM2_DUTY(0);
   }
 
   if(AMIN == MIN && AHOUR == HOUR && ((BIT_TEST(AMA) && BIT_TEST(AM)) || (!BIT_TEST(AMA) && !BIT_TEST(AM))) && BIT_TEST(ALARM_ON)){
  
   if(COUNTSEC % 15 == 0){
      BIT_SET(BUZZER);
    if(BIT_TEST(LED)){
     BIT_CLEAR(LED);
     }
      else{
      BIT_SET(LED);
      }
   
   if(!BIT_TEST(ALARM_MUTE)){
   
   if(BIT_TEST(BUZZ_ACT)){ 
      BIT_CLEAR(BUZZ_ACT);
      }   
   else{
      BIT_SET(BUZZ_ACT);
      }
     
   }
   
   }
  if(BIT_TEST(BUZZ_ACT)){ 
     SET_PWM2_DUTY(500);
     }
     
   else{
      SET_PWM2_DUTY(0);
      }
   
   }
 
  else{
  SET_PWM2_DUTY(0);
  BIT_CLEAR(LED);
  BIT_CLEAR(BUZZER);
  }
}
   

void TIMECALC(){
   //Rutina para calcular los minutos y los segundos. Es llamada cada vez que se incrementan los segundos.
   
   if(MIN >= 60){ 
      MIN = 0; //Resetear los minutos cuando pasen de 59, y aumentar la hora.
      HOUR++;
   }
   
   if(HOUR >= 12){
      
      if(BIT_TEST(AM)){ //Si la hora está en AM, y se llega a las 12
      BIT_CLEAR(AM); //Desactivar la bandera de AM (indicando que sería PM ahora)
      HOUR = 0;          //Y establecer la hora en 0. El programa maneja la hora de 0 a 11
      }
      
      if(!BIT_TEST(AM)){ //Si la hora está en PM y se llega a las 12, quiere decir que ha pasado un día.
      BIT_SET(AM);  //Se vuelve a activar AM
      DAY++;  //Se incrementa el día
      HOUR = 0; //Se resetea la hora
      DATECALC(); //Y se llama a la rutina que calcula la fecha.
      }

   }

}

void DET_BTNS(){ //Rutina para detectar pulsaciones de botones

   if(PORTA !=0){ 
   BIT_SET(SW_HOLD); //Si se detecta un valor distinto de 0 en PORTA, activar la bandera de que se presionó un switch
   }
   else{  //Si no se detecta nada, limpiar la bandera de que se presionó un switch, y la bandera de que se presionó un switch correctamente y aún no se ha borrado
      BIT_CLEAR(SW_HOLD);
      BIT_CLEAR(SW_NOTRELEASED);
      }
   
   if(BIT_TEST(SW_HOLD)){ 
      
      if(!BIT_TEST(SW_NOTRELEASED)){ //Incrementar el contador BTNCNT, que da una ventana de 200ms para detectar correctamente una pulsación de botón, solamente si la bandera SW_NOTRELEASED está desactivada.
      BTNCNT++;
      }
      
      if(BTNCNT == 80){
      BTNCNT = 0; //Una vez pasados 200ms (80 * 2.5ms = 200), resetear BTNCNT.
      
      BIT_SET(SW_NOTRELEASED); //Establecer la bandera de que sigue presionado el switch

      switch(PORTA){ //Toma el valor de PORTA, y activa la bandera correspondiente al switch que fue presionado
         case(1):{
               BIT_SET(SW1);
               BIT_CLEAR(SW_HOLD);
               BIT_SET(SW_OK);
               break;
               }
         case(2):{
               BIT_SET(SW2);
               BIT_CLEAR(SW_HOLD);
               BIT_SET(SW_OK);
               break;
               }
         case(4):{
               BIT_SET(SW3);
               BIT_CLEAR(SW_HOLD);
               BIT_SET(SW_OK);
               break;
               }
         case(8):{
               BIT_SET(SW4);
               BIT_CLEAR(SW_HOLD);
               BIT_SET(SW_OK);
               break;
               }
         default:{
               BIT_CLEAR(SW_HOLD);
               BIT_CLEAR(SW_OK);
         }

         }
      }
   }   
}

void SAVE_EEPROM(){
   /* GUARDAR HORA */

   if(BIT_TEST(AM)){
   WRITE_EEPROM(0, HOUR);
   }
   else{
   WRITE_EEPROM(0, HOUR + 12);
   }
   //Usando este if-else, se ahorra un byte en la EEPROM. En este montaje no importa mucho, pero para un montaje más complicado sería muy razonable aplicar este truco. Además, se desgasta menos la EEPROM.
   
   
   WRITE_EEPROM(1, MIN);
   
   /* GUARDAR FECHA */
   WRITE_EEPROM(2, DAY);
   WRITE_EEPROM(3, MONTH);
   WRITE_EEPROM(4, YEAR);
   
   /* GUARDAR ALARMA */
   
   WRITE_EEPROM(5, AMIN);

   if(BIT_TEST(AMA)){
   WRITE_EEPROM(6, AHOUR);
   }
   else{
   WRITE_EEPROM(6, AHOUR + 12);
   }
}

void LOAD_EEPROM(){

   /* CARGAR HORA */
   HOUR = READ_EEPROM(0);
   if(HOUR>23) HOUR=12;

   if(HOUR >= 12){ //En el formato en el que se guarda la hora en la EEPROM, 0 = 12 AM; 12 = 12 PM.
   HOUR -= 12; //Si la hora es igual a 12 o mayor, restar 12
   BIT_CLEAR(AM); //
   }
   
   MIN = READ_EEPROM(1);
   if(MIN>59) MIN=0;
   
   /* CARGAR FECHA */
   DAY = READ_EEPROM(2);
   if(DAY==0 || DAY>31) DAY=1;
   MONTH = READ_EEPROM(3);
   if(MONTH==0 || MONTH>12) MONTH=1;
   YEAR = READ_EEPROM(4);
   if(YEAR>99) YEAR=0;
   
   if((YEAR % 4 == 0 && YEAR % 100 != 0) || (YEAR % 100 == 0 && YEAR % 400 == 0)) { //Verificación 100% precisa para años bisiestos. 
   MONTHARRAY[1] = 29;
   }

   /* CARGAR ALARMA */

   AMIN = READ_EEPROM(5);
   if(AMIN>59) AMIN=0;

   AHOUR = READ_EEPROM(6);
   if(AHOUR>23) AHOUR=12;
   
   if(AHOUR >= 12){
   AHOUR -= 12;
   BIT_SET(AMA);
   }
   
    /* VERIFICAR VALIDEZ DE LOS DATOS CARGADOS */
   
   if(MONTH > 12){ //Se verifica aparte el mes, primero que todo lo demás. Esto se hace ya que, si el valor cargado como MES resulta ser mayor a 13, y se intenta acceder al arreglo MONTHARRAY (tamaño 12), cargará un valor inválido que podría 
             //Permitir que se cargara un día inválido
     
       HOUR = 0;
       MIN = 0;
       SEC = 0;
       BIT_SET(AM);
       
       AHOUR = 0;
       AMIN = 0;
       BIT_SET(AMA);
       
       DAY = 1;
       MONTH = 1;
       YEAR = 0;
            
      }
   
      
   if(DAY > MONTHARRAY[MONTH] || YEAR > 99 || HOUR > 23 || MIN > 59 || SEC > 59 || AHOUR > 23 || AMIN > 59){ //Si alguno de los valores almacenados en la EEPROM no tiene sentido, cargar unos predeterminados. 
      /* 1: Si el día almacenado es mayor al último día del mes actual 
    2: Si el año es mayor a (20)99 (El programa trabaja desde el 01/01/2000 hasta el 31/12/2099
    3: Si la hora (en formato de 24H, que es como se almacena en la EEPROM) es mayor a 23 (y la de la alarma también)
    4: Si los minutos (tanto de la hora como de la alarma) son mayores a 59
    5: Si los segundos son mayores a 59
    Cualquiera de esas condiciones indica que lo almacenado en la EEPROM es una fecha incorrecta.
      */
       HOUR = 0;
       MIN = 0;
       SEC = 0;
       BIT_SET(AM);
       
       AHOUR = 0;
       AMIN = 0;
       BIT_SET(AMA);
       
       DAY = 1;
       MONTH = 1;
       YEAR = 0;
      }
   
     }   

void MODECYCLE(){ //Rutina para hacer el ciclo entre los modos al presionar el botón SW1
   MODE++;
  
   if(MODE >=5){ //El programa trabaja los modos del 0 al 4. Si se pasa de 4, devolver al modo 0.
   MODE = 0;
      
   if(BIT_TEST(CHANGES_MADE)){ //Si está activada la bandera de que se han hecho cambios (en los modos 2, 3 o 4), se procede a guardar en la EEPROM, y se borra la bandera.
                       //Esta bandera impide que el programa guarde en la EEPROM si no se ha cambiado nada en la configuración.
      BIT_CLEAR(CHANGES_MADE);
      SAVE_EEPROM();
      
      }
   }
   
   PORTB = DISPLAY[MODE]; //Mostrar el modo en la EEPROM
   
   BIT_SET(MODE_CHANGED); //Activar bandera de que se cambió el modo
}

void DISPLCD(){
   
    if(BIT_TEST(MODE_CHANGED)){ //Si está activa la bandera de que se cambió el modo
       LCD_PUTC("\f"); //Limpiar el LCD
       BIT_CLEAR(MODE_CHANGED); //Y limpiar la bandera
       }
       
   switch(MODE){
       /*
      MODOS:
      0: HORA - FECHA
      1: HORA - ALARMA
      2: CONFIGURAR HORA
      3: CONFIGURAR FECHA
      4: CONFIGURAR ALARMA
      */
      
      case(0):{ 
         LCD_GOTOXY(1,1);
         LCD_PUTC("HORA ");
         if(HOUR == 0){
         PRINTF(LCD_PUTC, "12:%02d:%02d", MIN, SEC);
         }
         else{
            PRINTF(LCD_PUTC, "%02d:%02d:%02d", HOUR, MIN, SEC);
            }
         if(BIT_TEST(AM)){
            LCD_PUTC("AM");
         }
         else{
            LCD_PUTC("PM");
         }
      
      LCD_GOTOXY(1,2);
      LCD_PUTC("FECHA ");
      PRINTF(LCD_PUTC, "%02d/%02d/20%02d", DAY, MONTH, YEAR);
      break;
      }

      case(1):{

               LCD_GOTOXY(1,1);
         LCD_PUTC("HORA ");
         
         if(HOUR == 0){
         PRINTF(LCD_PUTC, "12:%02d:%02d", MIN, SEC);
         }
         else{
            PRINTF(LCD_PUTC, "%02d:%02d:%02d", HOUR, MIN, SEC);
            }
         if(BIT_TEST(AM)){
            LCD_PUTC("AM");
         }
         else{
            LCD_PUTC("PM");
         }


      LCD_GOTOXY(1,2);
      LCD_PUTC("ALARMA ");
     
      if(AHOUR == 0){
         PRINTF(LCD_PUTC, "12:%02d", AMIN);
         }
         
         else{
            PRINTF(LCD_PUTC, "%02d:%02d", AHOUR, AMIN);
            }
            
      if(BIT_TEST(AMA)){
         LCD_PUTC("AM");
      }
      else{
         LCD_PUTC("PM");
      }
      break;
      
      }
      
      case(2):{
         LCD_GOTOXY(1,1);
         LCD_PUTC("CONFIG. HORA");
         LCD_GOTOXY(1,2);
         if(HOUR == 0){
         PRINTF(LCD_PUTC, "12:%02d:%02d", MIN, SEC);
         }
         else{
            PRINTF(LCD_PUTC, "%02d:%02d:%02d", HOUR, MIN, SEC);
            }
            
         if(BIT_TEST(AM)){
            LCD_PUTC("AM");
         }
         else{
            LCD_PUTC("PM");
         }
         break;
         }
         
        case(3):{
           LCD_GOTOXY(1,1);
           LCD_PUTC("CONFIG. FECHA");
           LCD_GOTOXY(1,2);
           LCD_PUTC("FECHA ");
           PRINTF(LCD_PUTC, "%02d/%02d/20%02d", DAY, MONTH, YEAR);
           break;
           }
           
           case(4):{
         LCD_GOTOXY(1,1);
         LCD_PUTC("CONFIG. ALARMA");
         LCD_GOTOXY(1,2);
         LCD_PUTC("ALARMA ");
         
      if(AHOUR == 0){
         PRINTF(LCD_PUTC, "12:%02d", AMIN);
         }
         
         else{
            PRINTF(LCD_PUTC, "%02d:%02d", AHOUR, AMIN);
            }
      
         if(BIT_TEST(AMA)){
            LCD_PUTC("AM");
         }
         else{
            LCD_PUTC("PM");
         }
         
         break;
         }
      
   }
}

void TIMERCALC(){
   //Rutina para calcular cuando pasa un segundo.
   if(DISPN == 8){
      
      COUNTSEC++;
      ALARMTONE();
      DISPN = 0;
      if(COUNTSEC == 50){
         COUNTSEC = 0;
         
       if(MODE < 2){
       SEC++;
        }
      
      
       if(SEC >= 60){
         SEC = 0;
         MIN++;
       TIMECALC();
       }
       
      DISPLCD(); //Se actualiza el LCD una vez por segundo, para no sobrecargarlo (además de que como muestra los segundos de la hora, es necesario actualizarlo una vez por segundo.
         }
      }
   }



   
void main(){ 
   #ZERO_RAM
  
   SET_TRIS_A(15); //Configuración de puertos
   SET_TRIS_B(0);
   SET_TRIS_C(0);
   SET_TRIS_D(0);
   
    //Inicializar el LCD
   LCD_INIT();
    
   //Cargar los datos de la EEPROM
   LOAD_EEPROM();
   DELAY_MS(1);
   //Y un milisegundo después de cargarlos, mostrar la información en la LCD.
   DISPLCD();
    
   PORTB = DISPLAY[0];
   DISPN = 0;
   COUNTSEC = 0;
   BTNCNT = 0;   
   SWFLAGS = 0;
   FLAGS = 0;
   SETUP_COUNTERS(RTCC_INTERNAL, RTCC_DIV_16);
   BIT_CLEAR(ALARM_ON); //Establecer la alarma en APAGADA por defecto.
   SET_RTCC(212);
   ENABLE_INTERRUPTS(INT_RTCC);
   ENABLE_INTERRUPTS(GLOBAL);
   setup_ccp2(CCP_PWM);                                       //Configura el CCP2 para trabajar en PWM 
   setup_timer_2(T2_DIV_BY_4,0xF9,1);                         //Prescalador del TMR2 en 16 y contador 249 para f=1KHz
   SET_PWM2_DUTY(0);
   MODE = 0;
   
   while(TRUE){
      
   TIMERCALC();
   DET_BTNS();
   
      if(BIT_TEST(SW_OK)){ //Verifica los otros 4 ifs solo si está activa la bandera de que se presionó un switch correctamente.
    /* Explicación resumida del comportamiento de los ifs en la tabla al final del documento */
    
   if(BIT_TEST(SW1)){
   MODECYCLE();
    BIT_CLEAR(SW1);
   }
      
   if(BIT_TEST(SW2)){
       BIT_CLEAR(SW2);
      switch (MODE){
         
         case(1):{
       BIT_SET(ALARM_ON);
       BIT_CLEAR(ALARM_MUTE);
       break;
         }
         
         case(2):{
       HOUR++;
       if(HOUR >= 12){
          HOUR = 0;
          }
          BIT_SET(CHANGES_MADE);
         break;
       }
       
      case(3):{
         DAY++;
         if(DAY > MONTHARRAY[MONTH]){
            DAY = 1;
            }   
        BIT_SET(CHANGES_MADE);
       break;
         }
         
      case(4):{
         AHOUR++;
         if(AHOUR >= 12){
            AHOUR = 0;
            }
            BIT_SET(CHANGES_MADE);
         break;
         }
      }
   
      }

     if(BIT_TEST(SW3)){
      BIT_CLEAR(SW3);
      switch (MODE){
         
         case(1):{
       BIT_CLEAR(ALARM_ON);
       BIT_CLEAR(LED);
       BIT_CLEAR(BUZZER);
       DELAY_US(10);
       SET_PWM2_DUTY(0);
       break;
         }
         
         case(2):{
       MIN++;
       if(MIN > 59){
          MIN = 0;
          }
          BIT_SET(CHANGES_MADE);
         break;
       }
       
      case(3):{
         MONTH++;
         if(MONTH > 12){
            MONTH = 1;
            }   
            
          if(DAY > MONTHARRAY[MONTH - 1]){
             DAY = MONTHARRAY[MONTH - 1];
             }
             BIT_SET(CHANGES_MADE);
       break;
         }
         
      case(4):{
         AMIN++;
         if(AMIN > 59){
            AMIN = 0;
            }
            BIT_SET(CHANGES_MADE);
         break;
         }
      }
      
      }
      
      if(BIT_TEST(SW4)){
         
      BIT_CLEAR(SW4);
      switch (MODE){
         
         case(1):{
       if(BIT_TEST(ALARM_MUTE)){
       BIT_CLEAR(ALARM_MUTE);
         delay_us(10);
       BIT_CLEAR(BUZZER);
     
       }
       else{
       
       BIT_SET(ALARM_MUTE);
    
       BIT_SET(BUZZER);
       }
       
       break;
         }
         
         case(2):{
      if(BIT_TEST(AM)){
         BIT_CLEAR(AM);
         }
       else{
          BIT_SET(AM);
          }
          BIT_SET(CHANGES_MADE);
          break;
       }
       
      case(3):{
         
       YEAR++;
        if(YEAR > 100){
           YEAR = 0;
           }
         if((YEAR % 4 == 0 && YEAR % 100 != 0) || (YEAR % 100 == 0 && YEAR % 400 == 0)) { //Verificación 100% precisa para años bisiestos. 
        MONTHARRAY[1] = 29;
        }
        BIT_SET(CHANGES_MADE);
       break;
         }
         
      case(4):{
         
         if(BIT_TEST(AMA)){
         BIT_CLEAR(AMA);
         }
       else{
          BIT_SET(AMA);
          }
          BIT_SET(CHANGES_MADE);
         break;
         }
      }
      }
      BIT_CLEAR(SW_OK);
   }
 
   }
 }
 
 /*         set_pwm2_duty(500);                                      //Enciende el PWM
            set_pwm2_duty(0);                                       //Apaga salida PWM
*/
