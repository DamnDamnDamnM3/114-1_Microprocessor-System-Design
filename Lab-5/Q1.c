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








/*
 * ================================================================
 * Lab 5 - Question 1: 密碼鎖系統
 * 功能：實作一個4位數密碼鎖，包含LCD顯示、七段顯示器、LED和蜂鳴器
 * 硬體：NUC100系列微控制器
 * 作者：damnm3@googlegroups.com 共同作者
 * ================================================================
 * 
 * 硬體連接：
 * - PA0,1,2,3,4,5 連接至3x3按鍵矩陣
 * - PA12,13,14 連接至RGB LED
 * - PB8,9,10,12,13,14,15 連接至右側LED陣列
 * - PB11 連接至蜂鳴器
 * - PC4,5,6,7 連接至七段顯示器位選
 * - PE0-7 連接至七段顯示器段選
 * - LCD顯示器連接
 * 
 * 功能說明：
 * 1. 系統產生4位數隨機密碼(1-6)
 * 2. 使用者輸入4位數密碼
 * 3. 密碼正確：LED跑馬燈效果 + 成功提示
 * 4. 密碼錯誤：蜂鳴器響聲 + 錯誤提示
 * 5. 支援重新產生密碼和清除輸入
 * 
 * 按鍵對應：
 * - 按鍵1-6: 輸入數字1-6
 * - 按鍵7: 重新產生密碼
 * - 按鍵8: 清除輸入
 * - 按鍵9: 確認輸入
 */

// 包含必要的標頭檔
#include <stdio.h>              // 標準輸入輸出函數
#include <stdlib.h>             // 標準函數庫
#include <string.h>             // 字串處理函數
#include "NUC100Series.h"       // NUC100系列微控制器定義
#include "GPIO.h"               // GPIO控制函數
#include "SYS.h"                // 系統函數
#include "SYS_init.h"          // 系統初始化函數
#include "LCD.h"                // LCD顯示器控制函數
#include "Scankey.h"           // 按鍵掃描函數
#include "Seven_Segment.h"      // 七段顯示器控制函數

// ================================================================
// 硬體定義
// ================================================================
#define PIN_LED_R   PA12        // 紅色LED腳位
#define PIN_LED_G   PA13        // 綠色LED腳位
#define PIN_LED_B   PA14        // 藍色LED腳位
#define PIN_BUZZER  PB11        // 蜂鳴器腳位

// LED控制巨集定義
#define LED_ACTIVE_LOW 1        // LED為低電位觸發
#if LED_ACTIVE_LOW
  #define LED_ON(p)   ((p)=0)    // LED開啟
  #define LED_OFF(p)  ((p)=1)   // LED關閉
#else
  #define LED_ON(p)   ((p)=1)   // LED開啟
  #define LED_OFF(p)  ((p)=0)   // LED關閉
#endif

// 按鍵定義
#define K1 1                    // 數字1
#define K2 2                    // 數字2
#define K3 3                    // 數字3
#define K4 4                    // 數字4
#define K5 5                    // 數字5
#define K6 6                    // 數字6
#define KR 7                    // 重新產生密碼
#define KC 8                    // 清除輸入
#define KO 9                    // 確認輸入

// 右側LED控制定義
#define RLED_ACTIVE_LOW 1       // 右側LED為低電位觸發
static uint8_t secret[4] = {1,2,3,4};  // 密碼陣列
static uint8_t inbuf[4];                // 輸入緩衝區
static uint8_t inlen = 0;               // 輸入長度
static uint32_t count_seed = 0;         // 計數種子
static uint8_t lcd_line_now = 0;       // LCD當前行數
static const uint16_t RLED_MASK =       // 右側LED遮罩
    (BIT8 | BIT9 | BIT10 | /* skip BIT11 */ BIT12 | BIT13 | BIT14 | BIT15);

/*
 * ================================================================
 * 右側LED初始化函數
 * 功能：初始化右側LED陣列為輸出模式
 * ================================================================
 */
static void right_leds_init(void){
    GPIO_SetMode(PB, RLED_MASK, GPIO_MODE_OUTPUT);
#if RLED_ACTIVE_LOW
    /* 所有LED關閉 = 高電位 */
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

/*
 * ================================================================
 * 右側LED全部關閉函數
 * 功能：關閉所有右側LED
 * ================================================================
 */
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

/*
 * ================================================================
 * 右側LED單個點亮函數
 * 功能：點亮指定的右側LED
 * 參數：bit - 要點亮的LED位元
 * ================================================================
 */
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

/*
 * ================================================================
 * 右側LED跑馬燈函數
 * 功能：執行右側LED跑馬燈效果（PB8到PB10到PB12到PB15，跳過PB11）
 * ================================================================
 */
static void right_led_chaser(void){
    uint16_t seq[7];
    int n = 0, i;

    // 建立LED序列
    if (RLED_MASK & BIT8)  seq[n++] = BIT8;
    if (RLED_MASK & BIT9)  seq[n++] = BIT9;
    if (RLED_MASK & BIT10) seq[n++] = BIT10;
    if (RLED_MASK & BIT12) seq[n++] = BIT12;
    if (RLED_MASK & BIT13) seq[n++] = BIT13;
    if (RLED_MASK & BIT14) seq[n++] = BIT14;
    if (RLED_MASK & BIT15) seq[n++] = BIT15;

    // 正向跑馬燈
    for (i = 0; i < n; i++){
        right_led_on_bit(seq[i]);
        CLK_SysTickDelay(80000);
    }
    // 反向跑馬燈（跳過最後一個）
    for (i = n - 2; i >= 1; i--){
        right_led_on_bit(seq[i]);
        CLK_SysTickDelay(80000);
    }
    right_leds_all_off();
}

/*
 * ================================================================
 * RGB LED關閉函數
 * 功能：關閉所有RGB LED
 * ================================================================
 */
static void leds_off(void){
    GPIO_SetMode(PA, BIT12|BIT13|BIT14, GPIO_MODE_OUTPUT);
    LED_OFF(PIN_LED_R); LED_OFF(PIN_LED_G); LED_OFF(PIN_LED_B);
}

/*
 * ================================================================
 * 蜂鳴器控制函數
 * 功能：控制蜂鳴器響指定時間
 * 參數：ms - 響聲時間（毫秒）
 * ================================================================
 */
static void beep(uint32_t ms){
    GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT);
    PIN_BUZZER = 0; CLK_SysTickDelay(ms*1000); PIN_BUZZER = 1;
}

/*
 * ================================================================
 * RGB LED跑馬燈函數
 * 功能：執行RGB LED跑馬燈效果
 * ================================================================
 */
static void led_pass_chaser(void){
    int i;
    for (i=0; i<3; i++){
        LED_ON(PIN_LED_R); CLK_SysTickDelay(120000); LED_OFF(PIN_LED_R);
        LED_ON(PIN_LED_G); CLK_SysTickDelay(120000); LED_OFF(PIN_LED_G);
        LED_ON(PIN_LED_B); CLK_SysTickDelay(120000); LED_OFF(PIN_LED_B);
    }
}

/*
 * ================================================================
 * 七段顯示器四位數顯示函數
 * 功能：在七段顯示器上顯示四位數
 * 參數：d - 四位數陣列
 * ================================================================
 */
static void seg_show4_digits(const uint8_t d[4]){
    CloseSevenSegment(); ShowSevenSegment(3, d[0]); CLK_SysTickDelay(3000);
    CloseSevenSegment(); ShowSevenSegment(2, d[1]); CLK_SysTickDelay(3000);
    CloseSevenSegment(); ShowSevenSegment(1, d[2]); CLK_SysTickDelay(3000);
    CloseSevenSegment(); ShowSevenSegment(0, d[3]); CLK_SysTickDelay(3000);
}

/*
 * ================================================================
 * LCD寫入16字元行函數
 * 功能：在LCD指定行寫入字串
 * 參數：line - 行數, s - 字串
 * ================================================================
 */
static void lcd_write_line16(uint32_t line, const char* s){
    char buf[17];
    int n = 0, i = 0;
    while (s[n] && n < 16) { buf[n] = s[n]; n++; }
    for (i = n; i < 16; i++) buf[i] = ' ';
    buf[16] = '\0';
    print_Line(line, buf);
}

/*
 * ================================================================
 * LCD顯示輸入函數
 * 功能：在LCD上顯示當前輸入的數字
 * ================================================================
 */
static void lcd_show_input(void){
    char s[17];
    uint8_t i;
    for (i=0; i<inlen && i<16; i++) s[i] = (char)('0' + inbuf[i]);
    s[i] = '\0';
    print_Line(lcd_line_now, s);
}

/*
 * ================================================================
 * LCD顯示狀態函數
 * 功能：在LCD第2行顯示狀態訊息
 * 參數：msg - 狀態訊息
 * ================================================================
 */
static void lcd_show_status(const char* msg){
    print_Line(2, (char*)msg);
}

/*
 * ================================================================
 * LCD清除狀態函數
 * 功能：清除LCD第2行的狀態訊息
 * ================================================================
 */
static void lcd_clear_status(void){
    print_Line(2, "");
}

/*
 * ================================================================
 * 產生新密碼並顯示函數
 * 功能：產生新的4位數密碼並在七段顯示器上顯示
 * ================================================================
 */
static void new_secret_and_show(void){
    int i, t;
    unsigned seed;
    // 使用多個種子來源產生隨機數
    seed = (count_seed * 1664525u) + 1013904223u;
    seed ^= (unsigned)SysTick->VAL;
    seed ^= ((unsigned)PE->PIN << 8) | (unsigned)PC->PIN;
    srand(seed);
    // 產生4位數密碼（1-6）
    for (i=0; i<4; i++) secret[i] = (rand() % 6) + 1;
    // 在七段顯示器上顯示密碼80次
    for (t=0; t<80; t++) seg_show4_digits(secret);
}

/*
 * ================================================================
 * 接受數字輸入函數
 * 功能：接受使用者輸入的數字
 * 參數：d - 輸入的數字
 * ================================================================
 */
static void accept_digit(uint8_t d){
    if (inlen < 4) {
        inbuf[inlen++] = d;
        lcd_show_input();
        lcd_clear_status();
    }
}

/*
 * ================================================================
 * LCD顯示狀態附加函數
 * 功能：在LCD上顯示輸入數字和狀態
 * 參數：status - 狀態訊息
 * ================================================================
 */
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

/*
 * ================================================================
 * LCD顯示結果行函數
 * 功能：在LCD上顯示輸入數字和結果狀態
 * 參數：digits - 數字字串, status - 狀態字串
 * ================================================================
 */
static void lcd_show_result_line(const char* digits, const char* status)
{
    char line[17];
    int i = 0, pos = 0;

    if (lcd_line_now >= 4) return;  

    for (i = 0; i < 16; i++) line[i] = ' ';
    line[16] = '\0';

    // 顯示數字
    while (digits[pos] && pos < 11) {
        line[pos] = digits[pos];
        pos++;
    }

    // 顯示狀態
    i = 11;
    while (*status && i < 16) line[i++] = *status++;

    print_Line(lcd_line_now, line);
    lcd_line_now++;
}

/*
 * ================================================================
 * LED跑馬燈函數
 * 功能：執行PC12-15的LED跑馬燈效果
 * ================================================================
 */
void LED_Running(void)
{
    int i, bit;
    int dir = 1;  // 方向：1 = 向右, -1 = 向左
    int start = 12;
    int end = 15;
    int current;

    for (i = 0; i < 6; i++)  // 總共執行3次來回（6個方向）
    {
        if (dir == 1)  // 向右
        {
            for (current = start; current <= end; current++)
            {
                PC->DOUT &= ~(1 << current);   // LED開啟
                CLK_SysTickDelay(100000);
                PC->DOUT |= (1 << current);    // LED關閉
            }
        }
        else  // 向左
        {
            for (current = end; current >= start; current--)
            {
                PC->DOUT &= ~(1 << current);   // LED開啟
                CLK_SysTickDelay(100000);
                PC->DOUT |= (1 << current);    // LED關閉
            }
        }
        dir = -dir;  // 改變方向
    }
}

/*
 * ================================================================
 * 密碼驗證函數
 * 功能：驗證輸入的密碼是否正確
 * ================================================================
 */
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

    // 將輸入數字轉換為字串
    for (i=0; i<inlen && i<4; i++) digits[i] = '0' + inbuf[i];
    digits[i] = '\0';

    // 檢查密碼長度和內容
    if (inlen != 4) pass = 0;
    else {
        for (i=0; i<4; i++) {
            if (inbuf[i] != secret[i]) { pass = 0; break; }
        }
    }

    if (pass) {
        // 密碼正確：顯示成功訊息並執行LED效果
        lcd_show_result_line(digits, "PASS");
        right_led_chaser();      // 右側LED跑馬燈
        LED_Running();           // PC LED跑馬燈
    } else {
        // 密碼錯誤：顯示錯誤訊息並響蜂鳴器
        lcd_show_result_line(digits, "ERROR");
        beep(120);               // 蜂鳴器響120ms
    }

    inlen = 0;  // 重置輸入長度
}

/*
 * ================================================================
 * 清除函數
 * 功能：清除LCD顯示和輸入緩衝區
 * ================================================================
 */
static void do_clear(void){
    clear_LCD();
    inlen = 0;
    lcd_line_now = 0;
}

/*
 * ================================================================
 * 主程式
 * 功能：系統初始化和主迴圈，實作密碼鎖系統
 * ================================================================
 */
int main(void)
{
    uint8_t key;

    // ================================================================
    // 系統初始化階段
    // ================================================================
    SYS_Init();

    // 設定PA14和PA15為GPIO模式
    SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA14_Msk | SYS_GPA_MFP_PA15_Msk);
    SYS->ALT_MFP &= ~(SYS_ALT_MFP_PA14_Msk | SYS_ALT_MFP_PA15_Msk);
    SYS->ALT_MFP1 &= ~(SYS_ALT_MFP1_PA14_Msk | SYS_ALT_MFP1_PA15_Msk);

    init_LCD(); clear_LCD();     // LCD初始化
    OpenKeyPad();                // 按鍵掃描初始化
    OpenSevenSegment();          // 七段顯示器初始化
    leds_off();                  // RGB LED關閉
    GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT); PIN_BUZZER = 1;  // 蜂鳴器初始化
    right_leds_init();           // 右側LED初始化
    new_secret_and_show();       // 產生並顯示新密碼

    // ================================================================
    // 主程式迴圈
    // ================================================================
    while(1){
        seg_show4_digits(secret);  // 持續顯示密碼
        count_seed++;              // 增加計數種子

        key = ScanKey();
        if (key==0) continue;

        // 處理按鍵事件
        switch (key){
            case K1: accept_digit(1); break;    // 輸入數字1
            case K2: accept_digit(2); break;    // 輸入數字2
            case K3: accept_digit(3); break;    // 輸入數字3
            case K4: accept_digit(4); break;    // 輸入數字4
            case K5: accept_digit(5); break;    // 輸入數字5
            case K6: accept_digit(6); break;    // 輸入數字6
            case KR: new_secret_and_show(); break;  // 重新產生密碼
            case KC: do_clear(); break;         // 清除輸入
            case KO: do_open(); break;          // 確認輸入
            default: break;
        }

        CLK_SysTickDelay(120000);              // 延遲120ms
        while (ScanKey()!=0);                  // 等待按鍵釋放
    }
}
