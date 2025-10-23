/*
 * ================================================================
 * Lab 5.2 - Seven_Segment.c: 七段顯示器控制函數庫（支援負號）
 * 功能：提供七段顯示器的初始化、顯示和關閉功能，支援負號顯示
 * 硬體：NUC100系列微控制器
 * 作者：damnm3@googlegroups.com 共同作者
 * ================================================================
 *
 * 硬體連接：
 * - PC4,5,6,7 連接至七段顯示器位選（數位選擇）
 * - PE0-7 連接至七段顯示器段選（段控制）
 *
 * 七段顯示器段對應：
 * PE0=A, PE1=B, PE2=C, PE3=D, PE4=E, PE5=F, PE6=G, PE7=DP
 *
 * 數字顯示模式：
 * 0-9: 標準數字顯示
 * 10-15: 十六進制A-F顯示
 * 16: 負號"-"顯示
 */

// 包含必要的標頭檔
#include <stdio.h>              // 標準輸入輸出函數
#include "NUC100Series.h"       // NUC100系列微控制器定義
#include "GPIO.h"               // GPIO控制函數
#include "SYS.h"                // 系統函數
#include "Seven_Segment.h"      // 七段顯示器控制函數

// ================================================================
// 七段顯示器數字模式定義
// ================================================================
// 每個數字對應的段模式（位元對應：PE7=DP, PE6=G, PE5=F, PE4=E, PE3=D, PE2=C, PE1=B, PE0=A）
#define SEG_N0 0x82             // 數字0的段模式
#define SEG_N1 0xEE             // 數字1的段模式
#define SEG_N2 0x07             // 數字2的段模式
#define SEG_N3 0x46             // 數字3的段模式
#define SEG_N4 0x6A             // 數字4的段模式
#define SEG_N5 0x52             // 數字5的段模式
#define SEG_N6 0x12             // 數字6的段模式
#define SEG_N7 0xE6             // 數字7的段模式
#define SEG_N8 0x02             // 數字8的段模式
#define SEG_N9 0x62             // 數字9的段模式
#define SEG_N10 0x22            // 數字A(10)的段模式
#define SEG_N11 0x1A            // 數字B(11)的段模式
#define SEG_N12 0x93            // 數字C(12)的段模式
#define SEG_N13 0x0E            // 數字D(13)的段模式
#define SEG_N14 0x13            // 數字E(14)的段模式
#define SEG_N15 0x33            // 數字F(15)的段模式
#define SEG_N16 0x7F            // 負號"-"的段模式

// 七段顯示器緩衝區陣列，包含所有數字模式和負號
uint8_t SEG_BUF[17]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5,
                     SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_N10, SEG_N11, 
                     SEG_N12, SEG_N13, SEG_N14, SEG_N15, SEG_N16};

/*
 * ================================================================
 * 七段顯示器初始化函數
 * 功能：設定七段顯示器的GPIO腳位為輸出模式
 * ================================================================
 */
void OpenSevenSegment(void)
{
    // ================================================================
    // 位選腳位初始化（PC4,5,6,7）
    // ================================================================
    GPIO_SetMode(PC, BIT4, GPIO_PMD_OUTPUT);  // PC4設為輸出模式
    GPIO_SetMode(PC, BIT5, GPIO_PMD_OUTPUT);  // PC5設為輸出模式
    GPIO_SetMode(PC, BIT6, GPIO_PMD_OUTPUT);  // PC6設為輸出模式
    GPIO_SetMode(PC, BIT7, GPIO_PMD_OUTPUT);  // PC7設為輸出模式
    
    // 初始關閉所有位選（低電位關閉）
    PC4=0;
    PC5=0;
    PC6=0;
    PC7=0;
    
    // ================================================================
    // 段選腳位初始化（PE0-7）
    // ================================================================
    GPIO_SetMode(PE, BIT0, GPIO_PMD_QUASI);   // PE0設為準雙向模式
    GPIO_SetMode(PE, BIT1, GPIO_PMD_QUASI);   // PE1設為準雙向模式
    GPIO_SetMode(PE, BIT2, GPIO_PMD_QUASI);   // PE2設為準雙向模式
    GPIO_SetMode(PE, BIT3, GPIO_PMD_QUASI);   // PE3設為準雙向模式
    GPIO_SetMode(PE, BIT4, GPIO_PMD_QUASI);   // PE4設為準雙向模式
    GPIO_SetMode(PE, BIT5, GPIO_PMD_QUASI);   // PE5設為準雙向模式
    GPIO_SetMode(PE, BIT6, GPIO_PMD_QUASI);   // PE6設為準雙向模式
    GPIO_SetMode(PE, BIT7, GPIO_PMD_QUASI);   // PE7設為準雙向模式
    
    // 初始關閉所有段（低電位關閉）
    PE0=0;
    PE1=0;
    PE2=0;
    PE3=0;
    PE4=0;
    PE5=0;
    PE6=0;
    PE7=0;
}

/*
 * ================================================================
 * 七段顯示器顯示函數
 * 功能：在指定的七段顯示器位置顯示指定數字或符號
 * 參數：no - 顯示器位置(0-3), number - 要顯示的數字(0-16)
 * ================================================================
 */
void ShowSevenSegment(uint8_t no, uint8_t number)
{
    uint8_t temp, i;
    
    // 從緩衝區取得對應數字的段模式
    temp = SEG_BUF[number];
    
    // ================================================================
    // 根據段模式設定各個段
    // ================================================================
    for(i=0; i<8; i++)
    {
        // 檢查當前位元是否為1
        if((temp & 0x01) == 0x01)
        {
            // 位元為1，開啟對應的段
            switch(i) {
                case 0: PE0=1; break;  // A段
                case 1: PE1=1; break;  // B段
                case 2: PE2=1; break;  // C段
                case 3: PE3=1; break;  // D段
                case 4: PE4=1; break;  // E段
                case 5: PE5=1; break;  // F段
                case 6: PE6=1; break;  // G段
                case 7: PE7=1; break;  // DP段（小數點）
            }
        }
        else
        {
            // 位元為0，關閉對應的段
            switch(i) {
                case 0: PE0=0; break;  // A段
                case 1: PE1=0; break;  // B段
                case 2: PE2=0; break;  // C段
                case 3: PE3=0; break;  // D段
                case 4: PE4=0; break;  // E段
                case 5: PE5=0; break;  // F段
                case 6: PE6=0; break;  // G段
                case 7: PE7=0; break;  // DP段（小數點）
            }
        }
        temp = temp >> 1;              // 右移一位處理下一個位元
    }
    
    // ================================================================
    // 啟用指定的顯示器位置
    // ================================================================
    switch(no) {
        case 0: PC4=1; break;  // 第1個顯示器（最右邊）
        case 1: PC5=1; break;  // 第2個顯示器
        case 2: PC6=1; break;  // 第3個顯示器
        case 3: PC7=1; break;  // 第4個顯示器（最左邊）
    }
}

/*
 * ================================================================
 * 七段顯示器關閉函數
 * 功能：關閉所有七段顯示器
 * ================================================================
 */
void CloseSevenSegment(void)
{
    // 關閉所有位選（低電位關閉）
    PC4=0;
    PC5=0;
    PC6=0;
    PC7=0;
}