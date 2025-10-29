//
// Lab7-1: 單向移動球體系統
// 功能：球體從左向右移動，可透過按鍵控制開始、暫停、繼續
// EVB : Nu-LB-NUC140
// MCU : NUC140VE3CN
//
#include <stdio.h>
#include <math.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "LCD.h"
#include "Scankey.h"
#include "Draw2D.h"

// LCD顯示器尺寸定義（寬度128像素，高度64像素）
#define LCD_W 128
#define LCD_H 64

// 球體相關參數定義
#define RADIUS 4       // 球體半徑（像素）
#define STEP_X 8       // 每次移動的X軸步進距離（像素），每次移動2個半徑的距離
#define TICK_US 500000 // 每次移動之間的延遲時間（微秒），約0.5秒

// --- 蜂鳴器控制相關定義（PB11腳位，低電位觸發） ---
#define BUZZ_PIN_MASK BIT11 // 蜂鳴器控制腳位遮罩（PB11）

/**
 * 蜂鳴器初始化函數
 * 功能：設定PB11為輸出模式並初始化為關閉狀態
 * 說明：PB11 = 1 表示蜂鳴器關閉（低電位觸發型蜂鳴器）
 */
static void BUZZ_Init(void)
{
    GPIO_SetMode(PB, BUZZ_PIN_MASK, GPIO_PMD_OUTPUT); // 設定PB11為輸出模式
    PB11 = 1;                                         // 設定為高電位，關閉蜂鳴器（因為是Active-Low設計）
}

/**
 * 蜂鳴器響聲控制函數
 * @param us 蜂鳴器響聲持續時間（微秒）
 * 功能：讓蜂鳴器響聲指定時間長度
 * 流程：開啟蜂鳴器 → 延遲指定時間 → 關閉蜂鳴器
 */
static void BUZZ_Beep(uint32_t us)
{
    PB11 = 0;             // 設定為低電位，開啟蜂鳴器
    CLK_SysTickDelay(us); // 延遲指定時間（微秒）
    PB11 = 1;             // 設定為高電位，關閉蜂鳴器
}

/**
 * 繪製球體函數
 * @param x 球心X座標
 * @param y 球心Y座標
 * @param color 球體顏色（FG_COLOR或BG_COLOR）
 * 功能：在指定位置繪製一個圓形球體
 * 說明：使用draw_Circle函數繪製，背景色為BG_COLOR以便清除
 */
static void draw_ball(int x, int y, uint16_t color)
{
    draw_Circle(x, y, RADIUS, color, BG_COLOR); // 繪製圓形，背景色用於清除
}

/**
 * 主程式入口
 * 功能：實作球體單向移動系統，支援開始、暫停、繼續控制
 */
int main(void)
{
    // --- 系統狀態變數 ---
    int active = 0; // 球體運動啟用標誌（0=未啟用，1=已啟用）
    int paused = 0; // 球體運動暫停標誌（0=執行中，1=暫停）

    // --- 球體位置變數 ---
    int cx = RADIUS;                         // 當前球體圓心X座標，初始值為半徑（從左邊緣開始）
    const int cy = 32;                       // 球體圓心Y座標（固定在中間位置，LCD高度64/2=32）
    const int max_cx = (LCD_W - 1) - RADIUS; // 球體能移動的最大X座標（右邊界，避免超出螢幕）

    // --- 系統初始化 ---
    SYS_Init();   // 系統初始化（時鐘、GPIO等基本設定）
    init_LCD();   // LCD顯示器初始化
    clear_LCD();  // 清除LCD螢幕內容
    OpenKeyPad(); // 按鍵掃描功能初始化
    BUZZ_Init();  // 蜂鳴器初始化

    // --- 主程式迴圈 ---
    while (1)
    {
        int next_cx;         // 下一次移動後的預期X座標
        int key = ScanKey(); // 掃描按鍵狀態，返回值：0=無按鍵，1/2/3=對應按鍵

        // --- 按鍵處理邏輯 ---
        // 按鍵1：開始球體運動（僅在未啟用時有效）
        if (key == 1)
        {
            if (!active)
            {                                // 只有在球體未啟用時才執行
                cx = RADIUS;                 // 重置球體位置到左邊緣
                draw_ball(cx, cy, FG_COLOR); // 在起始位置繪製球體（前景色）
                active = 1;                  // 標記為已啟用
                paused = 0;                  // 標記為非暫停狀態
            }
        }
        // 按鍵2：暫停球體運動（僅在已啟用時有效）
        else if (key == 2)
        {
            if (active)
                paused = 1; // 如果球體已啟用，則設定為暫停狀態
        }
        // 按鍵3：繼續球體運動（僅在已啟用時有效）
        else if (key == 3)
        {
            if (active)
                paused = 0; // 如果球體已啟用，則取消暫停狀態
        }

        // --- 球體運動處理邏輯 ---
        // 只有在球體已啟用且未暫停時才執行運動
        if (active && !paused)
        {
            CLK_SysTickDelay(TICK_US); // 延遲0.5秒，控制移動速度

            // 第一步：清除當前位置的球體（用背景色繪製，相當於清除）
            draw_ball(cx, cy, BG_COLOR);

            // 第二步：計算下一位置並判斷是否超出邊界
            next_cx = cx + STEP_X; // 計算下一次移動後的X座標（向右移動STEP_X像素）

            // 判斷是否還在螢幕範圍內
            if (next_cx <= max_cx)
            {
                // 仍在範圍內：更新位置並繪製新球體
                cx = next_cx;                // 更新當前X座標
                draw_ball(cx, cy, FG_COLOR); // 在新位置繪製球體
            }
            else
            {
                // 已超出右邊界：執行結束流程
                cx = max_cx;                 // 將座標限制在最大邊界位置
                draw_ball(cx, cy, FG_COLOR); // 在邊界位置繪製球體

                // 觸發蜂鳴器響聲，提示已到達終點
                BUZZ_Beep(100000); // 響聲0.1秒（100000微秒）

                // 清除球體並重置狀態
                draw_ball(cx, cy, BG_COLOR); // 清除球體顯示
                active = 0;                  // 標記為未啟用
                paused = 0;                  // 清除暫停狀態
            }
        }
        // 當球體未啟用或處於暫停狀態時
        else
        {
            CLK_SysTickDelay(10000); // 短延遲10毫秒，降低CPU使用率
        }
    }
}
