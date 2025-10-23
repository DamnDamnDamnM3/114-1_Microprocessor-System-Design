/*
 * ================================================================
 * Lab 4 - Question 1: 隨機數產生與餘數計算顯示
 * 功能：產生隨機數並計算不同除數的餘數，在七段顯示器上顯示結果
 * 硬體：NUC100系列微控制器
 * 作者：damnm3@googlegroups.com 共同作者
 * ================================================================
 * 
 * 硬體連接：
 * - PA0,1,2,3,4,5 連接至3x3按鍵矩陣
 * - PC4,5,6,7 連接至七段顯示器位選
 * - PE0-7 連接至七段顯示器段選
 * 
 * 功能說明：
 * 1. 按鍵3按下時產生0-99的隨機數
 * 2. 按鍵1,4,7分別計算餘數(除以2,3,5)
 * 3. 按鍵9清除顯示
 * 4. 在七段顯示器上顯示餘數和原數
 * 
 * 顯示格式：
 * 位置3: 餘數的個位數
 * 位置2: 空白
 * 位置1: 原數的十位數
 * 位置0: 原數的個位數
 */

// 包含必要的標頭檔
#include <stdio.h>              // 標準輸入輸出函數
#include <stdlib.h>             // 標準函數庫（包含rand, srand）
#include "NUC100Series.h"       // NUC100系列微控制器定義
#include "MCU_init.h"          // 微控制器初始化函數
#include "SYS_init.h"          // 系統初始化函數
#include "Seven_Segment.h"      // 七段顯示器控制函數
#include "Scankey.h"           // 按鍵掃描函數

/*
 * ================================================================
 * 七段顯示器顯示函數
 * 功能：在七段顯示器上顯示餘數和原數
 * 參數：remainder - 餘數, randomNum - 原隨機數
 * ================================================================
 */
void Display_Lab4_1(int remainder, int randomNum)
{
    uint8_t d0, d1, d3;         // 數位變數
    
    // 計算原數的十位數和個位數
    d1 = (randomNum / 10) % 10;  // 十位數
    d0 = randomNum % 10;         // 個位數
    
    // 計算餘數的個位數
    d3 = remainder % 10;         // 餘數個位數

    // 顯示餘數個位數（位置3）
    CloseSevenSegment();
    ShowSevenSegment(3, d3);
    CLK_SysTickDelay(5000);

    // 位置2保持空白
    CloseSevenSegment();
    CLK_SysTickDelay(5000);

    // 顯示原數十位數（位置1）
    CloseSevenSegment();
    ShowSevenSegment(1, d1);
    CLK_SysTickDelay(5000);

    // 顯示原數個位數（位置0）
    CloseSevenSegment();
    ShowSevenSegment(0, d0);
    CLK_SysTickDelay(5000);
}

/*
 * ================================================================
 * 主程式
 * 功能：系統初始化和主迴圈，實作隨機數產生與餘數計算
 * ================================================================
 */
int main(void)
{
    int key = 0;                 // 當前按鍵值
    int lastKey = 0;             // 上次按鍵值
    int randomNum = 0;           // 隨機數
    int remainder = 0;           // 餘數
    int seedCounter = 0;         // 種子計數器
    int clearFlag = 1;           // 清除標記

    // ================================================================
    // 系統初始化階段
    // ================================================================
    SYS_Init();                  // 系統初始化
    OpenSevenSegment();          // 開啟七段顯示器
    OpenKeyPad();                // 開啟按鍵掃描
    CloseSevenSegment();         // 初始關閉顯示器

    // ================================================================
    // 主程式迴圈
    // ================================================================
    while (1)
    {
        seedCounter++;           // 增加種子計數器
        key = ScanKey();         // 掃描按鍵狀態

        // 按鍵3按下時產生隨機數
        if (key == 0 && lastKey == 3)
        {
            srand(seedCounter);              // 設定隨機數種子
            randomNum = rand() % 100;         // 產生0-99的隨機數
            clearFlag = 0;                   // 清除標記設為0（顯示模式）
        }

        // 處理其他按鍵
        if (key != 0)
        {
            if (key == 1)
                // 按鍵1：計算除以2的餘數
                remainder = randomNum % 2;
            else if (key == 4)
                // 按鍵4：計算除以3的餘數
                remainder = randomNum % 3;
            else if (key == 7)
                // 按鍵7：計算除以5的餘數
                remainder = randomNum % 5;
            else if (key == 9)
            {
                // 按鍵9：清除所有顯示
                randomNum = 0;
                remainder = 0;
                clearFlag = 1;               // 設為清除模式
                CloseSevenSegment();         // 關閉顯示器
            }
        }

        lastKey = key;           // 記錄當前按鍵值

        // 根據清除標記決定是否顯示
        if (!clearFlag)
            // 顯示模式：顯示餘數和原數
            Display_Lab4_1(remainder, randomNum);
        else
        {
            // 清除模式：保持顯示器關閉
            CloseSevenSegment();
            CLK_SysTickDelay(20000);         // 延遲20ms
        }
    }
}
