/*
 * ================================================================
 * Lab 4 - Question 2: 交通號誌控制系統
 * 功能：實作交通號誌的狀態控制和倒數計時顯示
 * 硬體：NUC100系列微控制器
 * 作者：damnm3@googlegroups.com 共同作者
 * ================================================================
 * 
 * 硬體連接：
 * - PA0,1,2,3,4,5 連接至3x3按鍵矩陣
 * - PA12,13,14 連接至交通號誌LED
 * - PC4,5,6,7 連接至七段顯示器位選
 * - PE0-7 連接至七段顯示器段選
 * 
 * 交通號誌狀態：
 * 1. 綠燈：8秒 (PA12=1, PA13=0, PA14=1)
 * 2. 黃燈：5秒 (PA12=1, PA13=0, PA14=0)
 * 3. 紅燈：13秒 (PA12=1, PA13=1, PA14=0)
 * 
 * 按鍵控制：
 * - 按鍵1: 強制綠燈
 * - 按鍵2: 強制黃燈
 * - 按鍵3: 強制紅燈
 * - 按鍵9: 增加5秒倒數時間
 */

// 包含必要的標頭檔
#include <stdio.h>              // 標準輸入輸出函數
#include "NUC100Series.h"       // NUC100系列微控制器定義
#include "MCU_init.h"          // 微控制器初始化函數
#include "SYS_init.h"          // 系統初始化函數
#include "Seven_Segment.h"      // 七段顯示器控制函數
#include "Scankey.h"           // 按鍵掃描函數

/*
 * ================================================================
 * GPIO初始化函數
 * 功能：設定交通號誌LED為輸出模式
 * ================================================================
 */
void Init_GPIO(void)
{
    // 設定PA12為輸出模式（藍色LED）
    GPIO_SetMode(PA, BIT12, GPIO_MODE_OUTPUT);
    // 設定PA13為輸出模式（綠色LED）
    GPIO_SetMode(PA, BIT13, GPIO_MODE_OUTPUT);
    // 設定PA14為輸出模式（紅色LED）
    GPIO_SetMode(PA, BIT14, GPIO_MODE_OUTPUT);
    
    // 初始化所有LED為關閉狀態（高電位）
    PA12 = 1;
    PA13 = 1;
    PA14 = 1;
}

/*
 * ================================================================
 * 七段顯示器數字顯示函數
 * 功能：在四個七段顯示器上顯示四位數數字
 * 參數：value - 要顯示的數值(0-9999)
 * ================================================================
 */
void Display_7seg(uint16_t value)
{
    uint8_t digit;

    // 顯示千位數
    digit = value / 1000;
    CloseSevenSegment();
    ShowSevenSegment(3, digit);
    CLK_SysTickDelay(200);

    // 顯示百位數
    value %= 1000;
    digit = value / 100;
    CloseSevenSegment();
    ShowSevenSegment(2, digit);
    CLK_SysTickDelay(200);

    // 顯示十位數
    value %= 100;
    digit = value / 10;
    CloseSevenSegment();
    ShowSevenSegment(1, digit);
    CLK_SysTickDelay(200);

    // 顯示個位數
    digit = value % 10;
    CloseSevenSegment();
    ShowSevenSegment(0, digit);
    CLK_SysTickDelay(200);
}

/*
 * ================================================================
 * 交通號誌狀態列舉
 * ================================================================
 */
enum State { 
    GREEN,      // 綠燈狀態
    YELLOW,     // 黃燈狀態
    RED         // 紅燈狀態
};

/*
 * ================================================================
 * 交通號誌結構體
 * ================================================================
 */
struct TrafficSignal {
    enum State state;           // 當前狀態
    int greenDuration;          // 綠燈持續時間
    int yellowDuration;         // 黃燈持續時間
    int redDuration;           // 紅燈持續時間
    int timer;                 // 倒數計時器
};

/*
 * ================================================================
 * 交通號誌初始化函數
 * 功能：初始化交通號誌結構體
 * 參數：ts - 交通號誌結構體指標
 *       g - 綠燈持續時間
 *       y - 黃燈持續時間
 *       r - 紅燈持續時間
 * ================================================================
 */
void TrafficSignal_initialize(struct TrafficSignal *ts, int g, int y, int r)
{
    ts->state = GREEN;          // 初始狀態為綠燈
    ts->greenDuration = g;      // 設定綠燈時間
    ts->yellowDuration = y;     // 設定黃燈時間
    ts->redDuration = r;       // 設定紅燈時間
    ts->timer = g;             // 初始倒數時間
}

/*
 * ================================================================
 * 交通號誌倒數計時函數
 * 功能：處理交通號誌的倒數計時和狀態轉換
 * 參數：ts - 交通號誌結構體指標
 * ================================================================
 */
void TrafficSignal_countDown(struct TrafficSignal *ts)
{
    ts->timer--;                // 倒數計時減1
    
    if (ts->timer <= 0)
    {
        // 根據當前狀態轉換到下一個狀態
        switch (ts->state)
        {
        case GREEN:
            // 綠燈結束，轉為黃燈
            ts->state = YELLOW;
            ts->timer = ts->yellowDuration;
            break;
        case YELLOW:
            // 黃燈結束，轉為紅燈
            ts->state = RED;
            ts->timer = ts->redDuration;
            break;
        case RED:
        default:
            // 紅燈結束，轉為綠燈
            ts->state = GREEN;
            ts->timer = ts->greenDuration;
            break;
        }
    }
}

/*
 * ================================================================
 * LED顯示函數
 * 功能：根據交通號誌狀態控制LED顯示
 * 參數：s - 交通號誌狀態
 * ================================================================
 */
void Show_LED(enum State s)
{
    switch (s)
    {
    case GREEN:
        // 綠燈：PA12=1(藍), PA13=0(綠), PA14=1(紅)
        PA12 = 1;
        PA13 = 0;
        PA14 = 1;
        break;
    case YELLOW:
        // 黃燈：PA12=1(藍), PA13=0(綠), PA14=0(紅)
        PA12 = 1;
        PA13 = 0;
        PA14 = 0;
        break;
    case RED:
        // 紅燈：PA12=1(藍), PA13=1(綠), PA14=0(紅)
        PA12 = 1;
        PA13 = 1;
        PA14 = 0;
        break;
    }
}

/*
 * ================================================================
 * 主程式
 * 功能：系統初始化和主迴圈，實作交通號誌控制系統
 * ================================================================
 */
int main(void)
{
    struct TrafficSignal ts;    // 交通號誌結構體
    int key = 0, last_key = 0;  // 按鍵變數
    int tick = 0;               // 時鐘計數器

    // ================================================================
    // 系統初始化階段
    // ================================================================
    SYS_Init();                 // 系統初始化
    Init_GPIO();                // GPIO初始化
    OpenSevenSegment();         // 開啟七段顯示器
    OpenKeyPad();               // 開啟按鍵掃描

    // 初始化交通號誌：綠燈8秒，黃燈5秒，紅燈13秒
    TrafficSignal_initialize(&ts, 8, 5, 13);

    // ================================================================
    // 主程式迴圈
    // ================================================================
    while (1)
    {
        key = ScanKey();        // 掃描按鍵狀態
        
        // 處理按鍵事件（避免重複觸發）
        if (key != 0 && key != last_key)
        {
            last_key = key;
            switch (key)
            {
            case 1:
                // 按鍵1：強制綠燈
                ts.state = GREEN;
                ts.timer = ts.greenDuration;
                break;
            case 2:
                // 按鍵2：強制黃燈
                ts.state = YELLOW;
                ts.timer = ts.yellowDuration;
                break;
            case 3:
                // 按鍵3：強制紅燈
                ts.state = RED;
                ts.timer = ts.redDuration;
                break;
            case 9:
                // 按鍵9：增加5秒倒數時間
                ts.timer += 5;
                if (ts.timer > 99) ts.timer = 99;  // 限制最大值
                break;
            default:
                break;
            }
        }
        else if (key == 0)
        {
            last_key = 0;       // 重置按鍵狀態
        }

        // 更新顯示
        Display_7seg(ts.timer); // 顯示倒數時間
        Show_LED(ts.state);     // 顯示LED狀態

        // 時鐘計數器處理
        tick++;
        if (tick >= 900)        // 約1秒（900個週期）
        {
            TrafficSignal_countDown(&ts);  // 執行倒數計時
            tick = 0;           // 重置計數器
        }
    }
}
