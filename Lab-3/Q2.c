/*
 * ================================================================
 * Lab 3 - Question 2: 七段顯示器HOLA文字滾動控制
 * 功能：在七段顯示器上顯示滾動的HOLA文字，並使用按鍵控制滾動
 * 硬體：NUC100系列微控制器
 * 作者：damnm3@googlegroups.com 共同作者
 * ================================================================
 * 
 * 硬體連接：
 * - PA0,1,2,3,4,5 連接至3x3按鍵矩陣
 * - PC4,5,6,7 連接至七段顯示器位選
 * - PE0-7 連接至七段顯示器段選
 * 
 * 按鍵控制：
 * - 按鍵4: 向右滾動 (HOLA → AHOL → LAHO → OLAH)
 * - 按鍵6: 向左滾動 (HOLA → OLAH → LAHO → AHOL)
 * - 按鍵5: 暫停滾動
 * - 按鍵8: 重置為預設HOLA
 * 
 * 七段顯示器段對應：
 * PE7=G, PE6=E, PE5=D, PE4=B, PE3=A, PE2=F, PE1=DOT, PE0=C
 */

// 包含必要的標頭檔
#include <stdio.h>              // 標準輸入輸出函數
#include <math.h>               // 數學函數庫
#include "NUC100Series.h"       // NUC100系列微控制器定義
#include "MCU_init.h"          // 微控制器初始化函數
#include "SYS_init.h"          // 系統初始化函數
#include "Seven_Segment.h"      // 七段顯示器控制函數
#include "Scankey.h"           // 按鍵掃描函數

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
    CLK_SysTickDelay(5000);
    
    // 顯示百位數
    value = value - digit * 1000;
    digit = value / 100;
    CloseSevenSegment();
    ShowSevenSegment(2, digit);
    CLK_SysTickDelay(5000);

    // 顯示十位數
    value = value - digit * 100;
    digit = value / 10;
    CloseSevenSegment();
    ShowSevenSegment(1, digit);
    CLK_SysTickDelay(5000);

    // 顯示個位數
    value = value - digit * 10;
    digit = value;
    CloseSevenSegment();
    ShowSevenSegment(0, digit);
    CLK_SysTickDelay(5000);
}

/*
 * ================================================================
 * 自定義圖案顯示函數
 * 功能：在指定的七段顯示器上顯示自定義圖案
 * 參數：i - 顯示器位置(0-3), pattern - 圖案位元模式
 * ================================================================
 */
void Segment_showPattern(int i, unsigned char pattern)
{
    CloseSevenSegment();
    
    // 檢查位置是否有效
    if (i >= 0 && i <= 3) {
        uint8_t temp, j;
        temp = pattern;
        
        // 根據圖案位元設定七段顯示器段
        for (j = 0; j < 8; j++) {
            if ((temp & 0x01) == 0x01) {
                // 段開啟
                switch (j) {
                    case 0: PE0 = 1; break;  // C段
                    case 1: PE1 = 1; break;  // DOT段
                    case 2: PE2 = 1; break;  // F段
                    case 3: PE3 = 1; break;  // A段
                    case 4: PE4 = 1; break;  // B段
                    case 5: PE5 = 1; break;  // D段
                    case 6: PE6 = 1; break;  // E段
                    case 7: PE7 = 1; break;  // G段
                }
            } else {
                // 段關閉
                switch (j) {
                    case 0: PE0 = 0; break;  // C段
                    case 1: PE1 = 0; break;  // DOT段
                    case 2: PE2 = 0; break;  // F段
                    case 3: PE3 = 0; break;  // A段
                    case 4: PE4 = 0; break;  // B段
                    case 5: PE5 = 0; break;  // D段
                    case 6: PE6 = 0; break;  // E段
                    case 7: PE7 = 0; break;  // G段
                }
            }
            temp = temp >> 1;  // 右移一位處理下一個位元
        }
        
        // 啟用指定的顯示器位置
        switch (i) {
            case 0: PC4 = 1; break;  // 第1個顯示器
            case 1: PC5 = 1; break;  // 第2個顯示器
            case 2: PC6 = 1; break;  // 第3個顯示器
            case 3: PC7 = 1; break;  // 第4個顯示器
        }
    }
}

/*
 * ================================================================
 * 主程式
 * 功能：系統初始化和主迴圈，實作HOLA文字滾動顯示
 * ================================================================
 */
int main(void)
{
    int k = 0;                  // 按鍵掃描結果
    int j = 0;                  // 迴圈計數器
    
    // HOLA滾動相關變數
    // 根據實際段對應：PE7=G, PE6=E, PE5=D, PE4=B, PE3=A, PE2=F, PE1=DOT, PE0=C
    // 圖案格式：G-E-D-B-A-F-DOT-C (位元7到位元0)
    // 使用正確值：H=0x2A, O=0x82, L=0x9B, A=0x22
    unsigned char hola_patterns[4] = {0x2A, 0x82, 0x9B, 0x22}; // H, O, L, A圖案
    int hola_position = 0;       // HOLA當前起始位置(0-3)
    int scrolling = 0;           // 滾動狀態：1=滾動中, 0=暫停(預設暫停)
    int scroll_direction = 1;    // 滾動方向：1=向右, -1=向左
    unsigned int scroll_counter = 0; // 滾動計數器
    
    // ================================================================
    // 系統初始化階段
    // ================================================================
    SYS_Init();                 // 系統初始化

    OpenSevenSegment();         // 開啟七段顯示器
    OpenKeyPad();               // 開啟按鍵掃描
    
    // ================================================================
    // 主程式迴圈
    // ================================================================
    while(1) {
        k = ScanKey();          // 掃描按鍵狀態
        
        // 處理HOLA滾動控制
        if (k == 4) { 
            // 向右滾動 - HOLA → AHOL → LAHO → OLAH
            if (scrolling == 0) { 
                // 首次按下或暫停後，立即移動
                hola_position = (hola_position + 1) % 4;
            }
            scroll_direction = 1;
            scrolling = 1;
            scroll_counter = 0;  // 重置計數器準備下次滾動
        } else if (k == 6) { 
            // 向左滾動 - HOLA → OLAH → LAHO → AHOL
            if (scrolling == 0) { 
                // 首次按下或暫停後，立即移動
                hola_position = (hola_position - 1 + 4) % 4;
            }
            scroll_direction = -1;
            scrolling = 1;
            scroll_counter = 0;  // 重置計數器準備下次滾動
        } else if (k == 5) { 
            // 暫停滾動
            scrolling = 0;
        } else if (k == 8) { 
            // 重置為預設HOLA
            hola_position = 0;
            scrolling = 0;       // 重置為暫停狀態
            scroll_direction = 1;
            scroll_counter = 0;  // 重置計數器
        }
        
        // 在七段顯示器上顯示HOLA（使用多工顯示）
        for (j = 0; j < 4; j++) {
            int char_index = (hola_position + (3 - j)) % 4;
            uint8_t pattern = hola_patterns[char_index];
            
            CloseSevenSegment();
            
            // 使用PE腳位直接設定HOLA字元圖案
            // 實際對應：PE7=G, PE6=E, PE5=D, PE4=B, PE3=A, PE2=F, PE1=DOT, PE0=C
            PE0 = (pattern & 0x01) ? 1 : 0;  // C段
            PE1 = (pattern & 0x02) ? 1 : 0;  // DOT段
            PE2 = (pattern & 0x04) ? 1 : 0;  // F段
            PE3 = (pattern & 0x08) ? 1 : 0;  // A段
            PE4 = (pattern & 0x10) ? 1 : 0;  // B段
            PE5 = (pattern & 0x20) ? 1 : 0;  // D段
            PE6 = (pattern & 0x40) ? 1 : 0;  // E段
            PE7 = (pattern & 0x80) ? 1 : 0;  // G段
            
            // 啟用顯示器位置
            switch(j) {
                case 0: PC4 = 1; break;  // 第1個顯示器
                case 1: PC5 = 1; break;  // 第2個顯示器
                case 2: PC6 = 1; break;  // 第3個顯示器
                case 3: PC7 = 1; break;  // 第4個顯示器
            }
            
            CLK_SysTickDelay(5000);  // 每個段的顯示時間
        }
        
        // 如果滾動啟用，每秒自動滾動HOLA
        if (scrolling) {
            scroll_counter++;
            if (scroll_counter >= 45) { // 約1秒(45週期 × ~22ms/週期)
                scroll_counter = 0;
                hola_position = (hola_position + scroll_direction + 4) % 4;
            }
        }
        
        CLK_SysTickDelay(1000); // 主迴圈小延遲
    }
}
