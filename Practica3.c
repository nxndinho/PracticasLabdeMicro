#INCLUDE < 16F877A.h>
#USE DELAY(CLOCK = 4000000) // 4MHz Clock
#FUSES XT, NOPROTECT, NOWDT, NOBROWNOUT, PUT, NOLVP
#BYTE PORTC = 7
#BYTE PORTA = 5
#BYTE PORTB = 6
#DEFINE U PORTA, 0
#DEFINE D PORTA, 1
Byte CONST display[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66,
                          0x6d, 0x7d, 0x07, 0x7f, 0x67};
int times, sec, uni, dec, i;

#int_rtcc clock() {
times--;
set_rtcc(238);

if (times == 0) {
  sec++;
  times = 217;
}
}

void show() {
  i = sec; // Tentativo hasta la linea 30.
  uni = 0;
  dec = 0;

  while (i >= 10) {
    i = i - 10;
    dec++; // Hasta aca!
  }
  uni = i;

  PORTC = display[uni];
  bit_set(U);
  bit_clear(D);
  delay_ms(1);

  PORTC = display[dec];
  bit_set(D);
  bit_clear(U);
  delay_ms(1);
}

int main() {
  SET_TRIS_B(0);
  SET_TRIS_A(0);          // Chequear si es entrada o salida!
  SET_TRIS_C(0B11111111); // ENTRADA BJT
  uni = 0;
  times = 217;
  sec = 0;

  set_rtcc(238);
  setup_counters(rtcc_internal, rtcc_div_256);
  enable_interrupts(int_rtcc);
  enable_interrupts(global);

  while (1) {
    if (sec == 60)
      sec = 0;
    else
      show();
  }
}
