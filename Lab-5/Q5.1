#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NUC100Series.h"
#include "GPIO.h"
#include "SYS.h"
#include "SYS_init.h"
#include "LCD.h"
#include "Scankey.h"
#include "Seven_Segment.h"

#define PIN_LED_R   PA12
#define PIN_LED_G   PA13
#define PIN_LED_B   PA14
#define PIN_BUZZER  PB11

#define LED_ACTIVE_LOW 1
#if LED_ACTIVE_LOW
  #define LED_ON(p)   ((p)=0)
  #define LED_OFF(p)  ((p)=1)
#else
  #define LED_ON(p)   ((p)=1)
  #define LED_OFF(p)  ((p)=0)
#endif

#define K1 1
#define K2 2
#define K3 3
#define K4 4
#define K5 5
#define K6 6
#define KR 7
#define KC 8
#define KO 9
#define RLED_ACTIVE_LOW 1
static uint8_t secret[4] = {1,2,3,4};
static uint8_t inbuf[4];
static uint8_t inlen = 0;
static uint32_t count_seed = 0;
static uint8_t lcd_line_now = 0;
static const uint16_t RLED_MASK =
    (BIT8 | BIT9 | BIT10 | /* skip BIT11 */ BIT12 | BIT13 | BIT14 | BIT15);

static void right_leds_init(void){
  GPIO_SetMode(PB, RLED_MASK, GPIO_MODE_OUTPUT);
#if RLED_ACTIVE_LOW
  /* all OFF = high */
  if (RLED_MASK & BIT8)  PB8  = 1;
  if (RLED_MASK & BIT9)  PB9  = 1;
  if (RLED_MASK & BIT10) PB10 = 1;
  if (RLED_MASK & BIT12) PB12 = 1;
  if (RLED_MASK & BIT13) PB13 = 1;
  if (RLED_MASK & BIT14) PB14 = 1;
  if (RLED_MASK & BIT15) PB15 = 1;
#else
  if (RLED_MASK & BIT8)  PB8  = 0;
  if (RLED_MASK & BIT9)  PB9  = 0;
  if (RLED_MASK & BIT10) PB10 = 0;
  if (RLED_MASK & BIT12) PB12 = 0;
  if (RLED_MASK & BIT13) PB13 = 0;
  if (RLED_MASK & BIT14) PB14 = 0;
  if (RLED_MASK & BIT15) PB15 = 0;
#endif
}

static void right_leds_all_off(void){
#if RLED_ACTIVE_LOW
  if (RLED_MASK & BIT8)  PB8  = 1;
  if (RLED_MASK & BIT9)  PB9  = 1;
  if (RLED_MASK & BIT10) PB10 = 1;
  if (RLED_MASK & BIT12) PB12 = 1;
  if (RLED_MASK & BIT13) PB13 = 1;
  if (RLED_MASK & BIT14) PB14 = 1;
  if (RLED_MASK & BIT15) PB15 = 1;
#else
  if (RLED_MASK & BIT8)  PB8  = 0;
  if (RLED_MASK & BIT9)  PB9  = 0;
  if (RLED_MASK & BIT10) PB10 = 0;
  if (RLED_MASK & BIT12) PB12 = 0;
  if (RLED_MASK & BIT13) PB13 = 0;
  if (RLED_MASK & BIT14) PB14 = 0;
  if (RLED_MASK & BIT15) PB15 = 0;
#endif
}

static void right_led_on_bit(uint16_t bit){
  right_leds_all_off();
#if RLED_ACTIVE_LOW
  if (bit == BIT8)  PB8  = 0;
  if (bit == BIT9)  PB9  = 0;
  if (bit == BIT10) PB10 = 0;
  if (bit == BIT12) PB12 = 0;
  if (bit == BIT13) PB13 = 0;
  if (bit == BIT14) PB14 = 0;
  if (bit == BIT15) PB15 = 0;
#else
  if (bit == BIT8)  PB8  = 1;
  if (bit == BIT9)  PB9  = 1;
  if (bit == BIT10) PB10 = 1;
  if (bit == BIT12) PB12 = 1;
  if (bit == BIT13) PB13 = 1;
  if (bit == BIT14) PB14 = 1;
  if (bit == BIT15) PB15 = 1;
#endif
}

/* ???:? PB8?PB10?PB12?PB13?PB14?PB15(?? PB11)???? */
static void right_led_chaser(void){
  uint16_t seq[7];
  int n = 0, i;

  if (RLED_MASK & BIT8)  seq[n++] = BIT8;
  if (RLED_MASK & BIT9)  seq[n++] = BIT9;
  if (RLED_MASK & BIT10) seq[n++] = BIT10;
  if (RLED_MASK & BIT12) seq[n++] = BIT12;
  if (RLED_MASK & BIT13) seq[n++] = BIT13;
  if (RLED_MASK & BIT14) seq[n++] = BIT14;
  if (RLED_MASK & BIT15) seq[n++] = BIT15;

  for (i = 0; i < n; i++){           /* ??? */
    right_led_on_bit(seq[i]);
    CLK_SysTickDelay(80000);
  }
  for (i = n - 2; i >= 1; i--){      /* ???(?????) */
    right_led_on_bit(seq[i]);
    CLK_SysTickDelay(80000);
  }
  right_leds_all_off();
}
static void leds_off(void){
  GPIO_SetMode(PA, BIT12|BIT13|BIT14, GPIO_MODE_OUTPUT);
  LED_OFF(PIN_LED_R); LED_OFF(PIN_LED_G); LED_OFF(PIN_LED_B);
}
static void beep(uint32_t ms){
  GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT);
  PIN_BUZZER = 0; CLK_SysTickDelay(ms*1000); PIN_BUZZER = 1;
}
static void led_pass_chaser(void){
  int i;
  for (i=0; i<3; i++){
    LED_ON(PIN_LED_R); CLK_SysTickDelay(120000); LED_OFF(PIN_LED_R);
    LED_ON(PIN_LED_G); CLK_SysTickDelay(120000); LED_OFF(PIN_LED_G);
    LED_ON(PIN_LED_B); CLK_SysTickDelay(120000); LED_OFF(PIN_LED_B);
  }
}

static void seg_show4_digits(const uint8_t d[4]){
  CloseSevenSegment(); ShowSevenSegment(3, d[0]); CLK_SysTickDelay(3000);
  CloseSevenSegment(); ShowSevenSegment(2, d[1]); CLK_SysTickDelay(3000);
  CloseSevenSegment(); ShowSevenSegment(1, d[2]); CLK_SysTickDelay(3000);
  CloseSevenSegment(); ShowSevenSegment(0, d[3]); CLK_SysTickDelay(3000);
}
static void lcd_write_line16(uint32_t line, const char* s){
  char buf[17];
  int n = 0, i = 0;
  while (s[n] && n < 16) { buf[n] = s[n]; n++; }
  for (i = n; i < 16; i++) buf[i] = ' ';
  buf[16] = '\0';
  print_Line(line, buf);
}
static void lcd_show_input(void){
  char s[17];
  uint8_t i;
  for (i=0; i<inlen && i<16; i++) s[i] = (char)('0' + inbuf[i]);
  s[i] = '\0';
  print_Line(lcd_line_now, s);
}
static void lcd_show_status(const char* msg){
  print_Line(2, (char*)msg);
}
static void lcd_clear_status(void){
  print_Line(2, "");
}

static void new_secret_and_show(void){
  int i, t;
  unsigned seed;
  seed = (count_seed * 1664525u) + 1013904223u;
  seed ^= (unsigned)SysTick->VAL;
  seed ^= ((unsigned)PE->PIN << 8) | (unsigned)PC->PIN;
  srand(seed);
  for (i=0; i<4; i++) secret[i] = (rand() % 6) + 1;
  for (t=0; t<80; t++) seg_show4_digits(secret);
}

static void accept_digit(uint8_t d){
  if (inlen < 4) {
    inbuf[inlen++] = d;
    lcd_show_input();
    lcd_clear_status();
  }
}
static void lcd_show_status_append(const char* status){
  char s[17];
  uint8_t i;
  uint8_t pos;

  for (i=0; i<inlen && i<16; i++) s[i] = (char)('0' + inbuf[i]);
  pos = i;
  while (*status && pos < 16) { s[pos++] = *status++; }
  s[pos] = '\0';

  lcd_write_line16(1, s);
}

static void lcd_show_result_line(const char* digits, const char* status)
{
  char line[17];
  int i = 0, pos = 0;

  if (lcd_line_now >= 4) return;  

  for (i = 0; i < 16; i++) line[i] = ' ';
  line[16] = '\0';


  while (digits[pos] && pos < 11) {
    line[pos] = digits[pos];
    pos++;
  }

 
  i = 11;
  while (*status && i < 16) line[i++] = *status++;

  print_Line(lcd_line_now, line);
  lcd_line_now++;
}

void LED_Running(void)
{
    int i, bit;
    int dir = 1;  // ??:1 = ???,-1 = ???
    int start = 12;
    int end = 15;
    int current;

    for (i = 0; i < 6; i++)  // ????? 3 ? (6 ?????)
    {
        if (dir == 1)  // ???
        {
            for (current = start; current <= end; current++)
            {
                PC->DOUT &= ~(1 << current);   // LED ?
                CLK_SysTickDelay(100000);
                PC->DOUT |= (1 << current);    // LED ?
            }
        }
        else  // ???
        {
            for (current = end; current >= start; current--)
            {
                PC->DOUT &= ~(1 << current);   // LED ?
                CLK_SysTickDelay(100000);
                PC->DOUT |= (1 << current);    // LED ?
            }
        }
        dir = -dir;  // ????
    }
}



static void do_open(void)
{
    uint8_t pass = 1;
    uint8_t i;
    char digits[8];

    if (lcd_line_now >= 4) return;

    if (inlen == 0) {
        lcd_show_result_line("", "NULL");
        return;
    }

    for (i=0; i<inlen && i<4; i++) digits[i] = '0' + inbuf[i];
    digits[i] = '\0';

    if (inlen != 4) pass = 0;
    else {
        for (i=0; i<4; i++) {
            if (inbuf[i] != secret[i]) { pass = 0; break; }
        }
    }

    if (pass) {
				
        lcd_show_result_line(digits, "PASS");
        right_led_chaser();
				LED_Running();

    } else {
        lcd_show_result_line(digits, "ERROR");
        beep(120);
    }

    inlen = 0;
}



static void do_clear(void){
  clear_LCD();
  inlen = 0;
  lcd_line_now = 0;
}








int main(void)
{
  uint8_t key;

  SYS_Init();

SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA14_Msk | SYS_GPA_MFP_PA15_Msk);
SYS->ALT_MFP &= ~(SYS_ALT_MFP_PA14_Msk | SYS_ALT_MFP_PA15_Msk);
SYS->ALT_MFP1 &= ~(SYS_ALT_MFP1_PA14_Msk | SYS_ALT_MFP1_PA15_Msk);



  init_LCD(); clear_LCD();
  OpenKeyPad();
  OpenSevenSegment();
  leds_off();
  GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT); PIN_BUZZER = 1;
right_leds_init();
  new_secret_and_show();

  while(1){
    seg_show4_digits(secret);
    count_seed++;


    key = ScanKey();
    if (key==0) continue;

    switch (key){
      case K1: accept_digit(1); break;
      case K2: accept_digit(2); break;
      case K3: accept_digit(3); break;
      case K4: accept_digit(4); break;
      case K5: accept_digit(5); break;
      case K6: accept_digit(6); break;
      case KR: new_secret_and_show(); break;
      case KC: do_clear(); break;
      case KO: do_open(); break;
      default: break;
    }

    CLK_SysTickDelay(120000);
    while (ScanKey()!=0);
  }
}
