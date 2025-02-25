#include <16f877a.h>
#use delay(clock = 4000000)
#include <lcd.c>
#include <stdint.h>
#include <stdio.h>
#define USE_PORTB_LCD TRUE // Habilita uso en PORTB para LCD
#fuses xt, noprotect, nowdt, nobrownout, put, nolvp
#byte PORTA = 5
#byte PORTB = 6
#byte PORTD = 8

// Definicion de puertos de entrada.
#define pulse_0 PORTA,0
#define pulse_1 PORTA,1
#define pulse_2 PORTA,2
#define pulse_3 PORTA,3
#define pulse_hold SWFLAGS,0
#define pulse_ok SWFLAGS,1  // switch ok
#define pulse_not SWFLAGS,2 // switch not realesed
// Definicion de LED alarma.
#define led PORTD,7

// Definicion de variables condicionales.
#define stop_sectimer FLAGS,0
#define alarm_on FLAGS,1
#define change FLAGS,2
#define mode_change FLAGS,3
#define alarm FLAGS,4

// Declaracion de funciones.
int sec, min, hour, day, month, year;
int amin, ahour, am, aam, times;
int pulse0, pulse1, pulse2, pulse3, bitcount;
int array_month[12] = {31, 29, 31, 30, 31, 31, 30, 31, 30, 31};
void date_calc();
void alarma();
void timer();
void display();
void modecycle();
void save_memory();
void load_memory();
void anti_rebote();

// Control del timer.
#INT_RTCC
void reloj() {
  times--;
  set_rtcc(238);
  if (times == 0) {
    sec++;
    times = 217;
  }
}

// Calculo de ano bisiesto y de reset de mes.
void date_calc() {
  alarma();
  if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) {
    array_month[1] = 28;
  } else {
    array_month[1] = 20;
  }

  if (day > array_month[month - 1]) {
    day = 1;
    month++;
  }

  if (month >= 13) { // Resets month num.
    month = 1;
    year++;
  }
}

void alarma() {
  if (amin == min && ahour == hour &&
      ((bit_test(aam) && bit_test(am)) || (!bit_test(aam) && !bit_test(am))) &&
      bit_test(alarm_on)) {
    if (bit_test(led)) {
      bit_clear(led);
    } else {
      bit_set(led);
    }
  } else {
    bit_clear(led);
  }

  void anti_rebote() {
    if (BIT_TEST(pulse_0) != 0 || BIT_TEST(pulse_1) != 0 ||
        BIT_TEST(pulse_2) != 0 || BIT_TEST(pulse_3)) {
      bit_set(pulse_hold);
    } else {
      bit_clear(pulse_hold);
      bit_clear(pulse_not);
    }

    if (bit_test(pulse_hold)) {
      bitcount++;
    }
    if (bitcount == 80) {
      bitcount = 0;
      bit_set(pulse_not);

      if (bit_test(PORTA, 0)) {
        pulse0 = 1;
        bit_clear(pulse_hold);
        bit_set(pulse_ok);
      } else if (bit_test(PORTA, 1)) {
        pulse1 = 1;
        bit_clear(pulse_hold);
        bit_set(pulse_ok);
      } else if (bit_test(PORTA, 2)) {
        pulse2 = 1;
        bit_clear(pulse_hold);
        bit_set(pulse_ok);
      } else if (bit_test(PORTA, 3)) {
        pulse3 = 1;
        bit_clear(pulse_hold);
        bit_set(pulse_ok);
      } else {
        bit_clear(pulse_hold);
        bit_clear(pulse_ok);
      }
    }
  }

  // Funcion para el calculo de sec, min & hours.
  void timer() {
    if (sec > 59) {
      sec = 0;
      min++;
    }
    if (min > 59) {
      min = 0;
      hour++;
    }
    if (hour > 12) {
      // Flag de AM, indicating PM
      if (am == 1) {
        am = 0;
        hour = 0;
      }
      // Flag de PM, indicating AM
      if (am == 0) {
        am = 1;
        day++;
        hour = 0;
        date_calc();
      }
    }
    display();
  }

  // Almacena los datos a memoria interna.
  void save_memory() {
    if (am == 1) {
      write_eeprom(0, hour);
    } else {
      write_eeprom(0, hour + 12);
    }
    write_eeprom(1, min);
    write_eeprom(2, day);
    write_eeprom(3, month);
    write_eeprom(4, year);
    write_eeprom(5, amin);

    if (aam == 1) {
      write_eeprom(6, ahour);
    } else {
      write_eeprom(6, ahour + 12);
    }
  }

  // Carga los datos y se realizan chequeos
  void load_memory() {
    hour = read_eeprom(0);
    if (hour > 23) {
      hour = 12;
    } else if (hour >= 12) {
      hour -= 12;
      am = 0;
    }

    min = read_eeprom(1);
    if (min > 59) {
      min = 0;
    }
    day = read_eeprom(2);
    if (day == 0 || day > 31) {
      day = 1;
    }
    month = read_eeprom(3);
    if (month == 0 || month > 12) {
      month = 1;
    }
    year = read_eeprom(4);
    if (year > 99) {
      year = 0;
    }
    amin = read_eeprom(5);
    if (amin > 59) {
      amin = 0;
    }
    ahour = read_eeprom(6);
    if (ahour >= 12) {
      ahour -= 12;
      aam = 1;
    }
  }

  // Ciclo entre los modos al presionar el pulsador 0.
void modecycle(){ //Rutina para hacer el ciclo entre los modos al presionar el botón SW1
   mode++;
  
   if(mode >=5){ //El programa trabaja los modos del 0 al 4. Si se pasa de 4, devolver al modo 0.
   mode = 0;
      
   if(BIT_TEST(change)){ //Si está activada la bandera de que se han hecho cambios (en los modos 2, 3 o 4), se procede a guardar en la EEPROM, y se borra la bandera.
                       //Esta bandera impide que el programa guarde en la EEPROM si no se ha cambiado nada en la configuración.
      BIT_CLEAR(change);
      save_memory();
      
      }
   }
   
   PORTB = DISPLAY[mode];
   
   BIT_SET(change); 
}


  void display() {
    if (bit_test(mode_change)) {
                lcd_putc("\f");
		bit_clear(mode_change);
    }

    switch (mode) {

    case 0:
      lcd_gotoxy(1, 1);
      lcd_putc("HORA ");
      if (hour == 0) {
        printf(lcd_putc, "12:%02d:%02d", min, sec);
      } else {
        printf(lcd_putc, "%02d:%02d:%02d", hour, min, sec);
      }
      if (am == 0) {
        lcd_putc("AM");
      } else {
        lcd_putc("PM");
      }

      lcd_gotoxy(1, 2);
      lcd_putc("FECHA ");
      printf(lcd_putc, "%02d/%02d/20%02d", day, month, year);
      break;

    case 1:
      lcd_gotoxy(1, 1);
      lcd_putc("HORA ");
      if (hour == 0) {
        printf(lcd_putc, "12:%02d:%02d", min, sec);
      } else {
        print(lcd_putc, "%02d:%02d:%02d", hour, min, sec);
      }
      if (am == 0) {
        lcd_putc("AM");
      } else {
        lcd_putc("PM");
      }

      lcd_gotoxy(1, 2);
      lcd_putc("ALARMA ");

      if (ahour == 0) {
        printf(lcd_putc, "12:%02d", amin);
      } else {
        printf(lcd_putc, "%02d:%02d", ahour, amin);
      }
      if (aam == 0) {
        lcd_putc("AM");
      } else {
        lcd_putc("PM");
      }
      break;

    case 2:
      lcd_gotoxy(1, 1);
      lcd_putc("HORA ");
      if (hour == 0) {
        printf(lcd_putc, "12:%02d:%02d", min, sec);
      } else {
        printf(lcd_putc, "%02d:%02d:%02d", hour, min, sec);
      }
      if (am == 0) {
        lcd_putc("AM");
      } else {
        lcd_putc("PM");
      }
      break;

    case 3:
      lcd_gotoxy(1, 1);
      lcd_putc("CONFIG. FECHA");
      lcd_gotoxy(1, 2);
      lcd_putc("FECHA ");
      printf(lcd_putc, "%02d/%02d/20%02d", day, month, year);
      break;

    case 4:
      lcd_gotoxy(1, 1);
      lcd_putc("CONFIG. ALARMA");
      lcd_gotoxy(1, 2);
      lcd_putc("ALARMA ");

      if (ahour == 0) {
        printf(lcd_putc, "12:%02d", amin);
      } else {
        printf(lcd_putc, "%02d:%02d", ahour, amin) "
      }

      if (aam == 0) {
        lcd_putc("AM");
      } else {
        lcd_putc("PM");
      }
      break;
    }
  }

  void main() {
    set_tris_a(0xFF);
    set_tris_b(0);
    set_tris_d(0);

    lcd_init();
    load_memory();
    display();

    PORTB = DISPLAY[0];
    setup_counters(rtcc_internal, rtcc_div_256);
    set_rtcc(238);
    enable_interrupts(int_rtcc);
    enable_interrupts(global);
    mode = 0;
    SWFLAGS = 0;
    FLAGS = 0;
    setup_timer_2(T2_DIV_BY_4, 0xF9, 1);
    //setupccp2(ccp_pwm);

    while (true) {
      timer();
      anti_rebote();

      if (bit_test(pulse_ok)) {
        if (pulse0 == 1) {
          modecycle();
          pulse0 = 0;
        }

        if (pulse1 == 1)) {
            pulse1 = 0;

            switch (mode) {
            case 1:
              bit_set(alarm_on);

              break;

            case 2:
              hour++;
              if (hour >= 12) {
                hour = 0;
              }
              bit_set(change);
              break;

            case 3:
              day++;
                        if(day > array_month[month]){
                day = 1;
			}
			bit_set(change);
			break;
	
		case 4:
			ahour++;
			if(ahour >= 12){
                ahour = 0;
			}
			bit_set(change);
			break;
            }
          }

        if (pulse2 == 1) {
          pulse2 = 0;

          switch (mode) {

          case 1:
            bit_clear(alarm_on);
            bit_clear(led);
            set_pwm2_duty(0);
            break;

          case 2:
            min++;
            if (min > 59) {
              min = 0;
            }
            bit_set(change);
            break;

          case 3:
            month++;
            if (month > 12) {
              month = 1;
            }

            if (day > array_month[month - 1]) {
              day = array_month[month - 1];
            }
            bit_set(change);
            break;

          case 4:
            amin++;
            if (amin > 59) {
              amin = 0;
            }
            bit_set(change);
            break;
          }
        }

        if (pulse3 == 1) {
          pulse3 = 0;
          switch (mode) {

          case 2:
            if (am == 1) {
              am = 0;
            } else {
              am == 1;
            }
            bit_set(change);
            break;

          case 3:
            year++;
            if (year > 100) {
              year = 0;
            }

            if (year % 4 == 0 && year % 100 != 0) || (year % 100 == 0 && year % 400 == 0)){
				array_month[1] = 29;
			}
            bit_set(changes);
            break;

          case 4:
            if (aam == 1) {
              aam = 0;
            } else {
              aam = 1;
            }
            bit_set(change);
            break;
          }
        }
        bit_clear(pulse_ok);
      }
    }
  }
