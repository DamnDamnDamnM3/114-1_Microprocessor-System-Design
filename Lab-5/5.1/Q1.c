/*
 * ================================================================
 * Lab 5.1 - Question 1: 密碼鎖系統
 * 功能：實作一個4位數密碼鎖，包含LCD顯示、七段顯示器、LED和蜂鳴器
 * 硬體：NUC100系列微控制器
 * 作者：damnm3@googlegroups.com 共同作者
 * ================================================================
 *
 * 硬體連接：
 * - PA0,1,2,3,4,5 連接至3x3按鍵矩陣
 * - PC12,13,14,15 連接至LED指示器
 * - PB11 連接至蜂鳴器
 * - LCD顯示器連接
 * - PC4,5,6,7 連接至七段顯示器位選
 * - PE0-7 連接至七段顯示器段選
 *
 * 功能說明：
 * 1. 系統產生4位數隨機密碼(1-6)
 * 2. 使用者輸入4位數密碼
 * 3. 密碼正確：LED跑馬燈效果 + 成功提示
 * 4. 密碼錯誤：蜂鳴器響聲 + 錯誤提示
 * 5. 支援重新產生密碼和清除輸入
 * 6. 最多4次嘗試機會
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
#include "MCU_init.h"          // 微控制器初始化函數
#include "SYS_init.h"          // 系統初始化函數
#include "LCD.h"                // LCD顯示器控制函數
#include "Scankey.h"           // 按鍵掃描函數
#include "Seven_Segment.h"      // 七段顯示器控制函數

// ================================================================
// 常數定義
// ================================================================
#define SEED_STORAGE_ADDRESS 0x10000 // 種子儲存位址（範例位址，確保不與程式儲存區域衝突）

// ================================================================
// 全域變數
// ================================================================
char correct_password[5] = "1234";     // 正確密碼，將被隨機密碼覆蓋
char input_password[5] = "";           // 使用者輸入的密碼
int input_count = 0;                   // 輸入字元計數器
int password_set = 0;                  // 密碼是否已設定的標記
int display_password[4] = {0, 0, 0, 0}; // 用於七段顯示器的密碼陣列
int attempt_count = 0;                 // 嘗試次數計數器（最多4次）
static uint32_t entropy_accumulator = 0; // 熵累積器，用於產生隨機數

/*
 * ================================================================
 * 蜂鳴器控制函數
 * 功能：控制蜂鳴器響指定次數
 * 參數：number - 蜂鳴器響聲次數
 * ================================================================
 */
void Buzz(int number)
{
    int i;
    // 迴圈控制蜂鳴器響聲次數
    for (i=0; i<number; i++) {
        PB11=0;                         // PB11 = 0 開啟蜂鳴器
        CLK_SysTickDelay(100000);       // 延遲100ms
        PB11=1;                         // PB11 = 1 關閉蜂鳴器
        CLK_SysTickDelay(100000);       // 延遲100ms
    }
}

/*
 * ================================================================
 * 七段顯示器密碼顯示函數
 * 功能：持續在七段顯示器上顯示密碼
 * ================================================================
 */
void DisplayPasswordOn7Segment(void)
{
    // 只有在密碼已設定時才顯示
    if(password_set) {
        // 顯示最右邊的數字（個位數）
        ShowSevenSegment(0, display_password[3]);
        CLK_SysTickDelay(5000);
        CloseSevenSegment();
        
        // 顯示十位數
        ShowSevenSegment(1, display_password[2]);
        CLK_SysTickDelay(5000);
        CloseSevenSegment();
        
        // 顯示百位數
        ShowSevenSegment(2, display_password[1]);
        CLK_SysTickDelay(5000);
        CloseSevenSegment();
        
        // 顯示最左邊的數字（千位數）
        ShowSevenSegment(3, display_password[0]);
        CLK_SysTickDelay(5000);
        CloseSevenSegment();
    }
}

/*
 * ================================================================
 * 熵累積函數
 * 功能：從使用者互動和系統狀態累積熵值
 * ================================================================
 */
void AccumulateEntropy(void)
{
    // 使用系統時鐘值增加隨機性
    entropy_accumulator ^= SysTick->VAL;
    // 左旋轉操作增加熵的複雜度
    entropy_accumulator = (entropy_accumulator << 1) | (entropy_accumulator >> 31);
    // 加上質數增加隨機性
    entropy_accumulator += 1103515245;
}

/*
 * ================================================================
 * 隨機密碼產生函數
 * 功能：產生4位數隨機密碼（數字1-6）
 * ================================================================
 */
void GenerateRandomPassword(void)
{
    int i;
    uint32_t final_seed;
    
    // 使用多個熵源建立複雜的種子
    final_seed = SysTick->VAL ^
                 (entropy_accumulator * 1664525) ^
                 ((uint32_t)&i << 16) ^
                 (SysTick->LOAD) ^
                 0x5DEECE66D; // 大質數常數
    
    // 增加額外的混合操作
    final_seed ^= (final_seed >> 16);
    final_seed *= 0x85ebca6b;
    final_seed ^= (final_seed >> 13);
    
    // 設定隨機數種子
    srand(final_seed);
    
    // 產生多個隨機數以進一步推進狀態
    for(i = 0; i < 10; i++) {
        rand(); // 丟棄前幾個值
    }
    
    // 現在產生實際的密碼
    for(i = 0; i < 4; i++) {
        correct_password[i] = '1' + (rand() % 6); // 產生數字1-6
        display_password[i] = correct_password[i] - '0'; // 儲存用於七段顯示器
    }
    correct_password[4] = '\0';        // 字串結尾
    password_set = 1;                  // 標記密碼已設定
    
    // 更新熵值供下次使用
    entropy_accumulator ^= final_seed;
    
    // 清除LCD並重置嘗試次數
    clear_LCD();
    attempt_count = 0;
}

/*
 * ================================================================
 * LED跑馬燈函數
 * 功能：密碼正確時執行LED跑馬燈效果
 * ================================================================
 */
void RunningLight(void)
{
    int i, j;
    // 執行2個週期
    for(j = 0; j < 2; j++) {
        // 正向跑馬燈：PC12 → PC13 → PC14 → PC15
        for(i = 0; i < 2; i++) {
            PC12 = 0;                   // 開啟LED
            CLK_SysTickDelay(100000);
            PC12 = 1;                   // 關閉LED
            CLK_SysTickDelay(100000);
            PC13 = 0;                   // 開啟LED
            CLK_SysTickDelay(100000);
            PC13 = 1;                   // 關閉LED
            CLK_SysTickDelay(100000);
            PC14 = 0;                   // 開啟LED
            CLK_SysTickDelay(100000);
            PC14 = 1;                   // 關閉LED
            CLK_SysTickDelay(100000);
            PC15 = 0;                   // 開啟LED
            CLK_SysTickDelay(100000);
            PC15 = 1;                   // 關閉LED
            CLK_SysTickDelay(100000);
            
            // 反向跑馬燈：PC14 → PC13 → PC12
            CLK_SysTickDelay(100000);
            PC14 = 0;                   // 開啟LED
            CLK_SysTickDelay(100000);
            PC14 = 1;                   // 關閉LED
            CLK_SysTickDelay(100000);
            PC13 = 0;                   // 開啟LED
            CLK_SysTickDelay(100000);
            PC13 = 1;                   // 關閉LED
            PC12 = 0;                   // 開啟LED
            CLK_SysTickDelay(100000);
            PC12 = 1;                   // 關閉LED
        }
    }
}

/*
 * ================================================================
 * 清除輸入函數
 * 功能：清除使用者輸入的密碼
 * ================================================================
 */
void ClearInput(void)
{
    memset(input_password, 0, sizeof(input_password));
    input_count = 0;
}

/*
 * ================================================================
 * 密碼驗證函數
 * 功能：驗證密碼並顯示結果
 * ================================================================
 */
void VerifyPassword(void)
{
    char display_line[17]; // LCD行緩衝區
    
    // 檢查是否超過最大嘗試次數
    if(attempt_count >= 4) {
        return; // 超過嘗試次數，不執行任何動作
    }
    
    // 檢查是否有輸入
    if(input_count == 0) {
        // 沒有輸入 - 顯示NULL
        sprintf(display_line, " NULL");
        print_Line(attempt_count, display_line);
    }
    else if(strcmp(input_password, correct_password) == 0) {
        // 密碼正確 - 左邊顯示輸入，右邊顯示PASS
        sprintf(display_line, "%s PASS", input_password);
        print_Line(attempt_count, display_line);
        RunningLight(); // 執行LED跑馬燈
    }
    else {
        // 密碼錯誤 - 左邊顯示輸入，右邊顯示ERROR
        sprintf(display_line, "%s ERROR", input_password);
        print_Line(attempt_count, display_line);
        Buzz(1); // 蜂鳴器響一聲
    }
    
    attempt_count++;                   // 增加嘗試次數
    ClearInput();                      // 清除輸入供下次嘗試
}

/*
 * ================================================================
 * 主程式
 * 功能：系統初始化和主迴圈，實作密碼鎖系統
 * ================================================================
 */
int main(void)
{
    uint8_t keyin, last_key = 0;       // 按鍵輸入和上次按鍵
    int i;
    
    // ================================================================
    // GPIO初始化
    // ================================================================
    // PC12=0開啟LED，PC12=1關閉LED
    GPIO_SetMode(PC, BIT12, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT); // 蜂鳴器控制
    PC12 = 1;                          // 初始關閉LED
    PB11 = 1;                          // 初始關閉蜂鳴器
    
    // ================================================================
    // 系統初始化
    // ================================================================
    SYS_Init();
    
    // 使用多個變化源初始化熵值
    entropy_accumulator = SysTick->VAL ^ 0xDEADBEEF;
    
    // 等待不同時間以建立基於時間的熵
    for(i = 0; i < (SysTick->VAL % 1000); i++) {
        AccumulateEntropy();
    }
    
    // 設定初始隨機種子
    srand(entropy_accumulator ^ SysTick->VAL);
    
    // ================================================================
    // 週邊設備初始化
    // ================================================================
    init_LCD();                        // 初始化LCD
    clear_LCD();                       // 清除LCD
    OpenSevenSegment();                // 初始化七段顯示器
    clear_LCD();                       // 再次清除LCD
    OpenKeyPad();                      // 初始化3x3按鍵矩陣
    
    // ================================================================
    // 主程式迴圈
    // ================================================================
    while(1)
    {
        // 持續從系統狀態累積熵值
        AccumulateEntropy();
        
        // 持續在七段顯示器上顯示密碼
        DisplayPasswordOn7Segment();
        
        // 掃描按鍵矩陣輸入
        keyin = ScanKey();
        
        // 只有在按鍵被按下且與上次不同時才處理
        if(keyin != 0 && keyin != last_key) {
            // 從使用者互動累積熵值
            entropy_accumulator ^= (keyin << 24) | SysTick->VAL;
            
            if(keyin >= 1 && keyin <= 6) {
                // 密碼輸入(1-6) - 只有在少於4位數、密碼已設定且未超過嘗試次數時才接受
                if(input_count < 4 && password_set && attempt_count < 4) {
                    input_password[input_count] = '0' + keyin;
                    input_count++;
                    input_password[input_count] = '\0';
                    // 顯示當前輸入（尚未永久儲存）
                    print_Line(attempt_count, input_password);
                }
            }
            else if(keyin == 7) { // R鍵
                // 產生隨機密碼
                GenerateRandomPassword();
                ClearInput();
            }
            else if(keyin == 8) { // C鍵
                // 清除LCD螢幕並重置所有內容
                clear_LCD();
                ClearInput();
                attempt_count = 0;
            }
            else if(keyin == 9) { // O鍵
                // 驗證密碼
                if(password_set && attempt_count < 4) {
                    VerifyPassword();
                } else if(!password_set) {
                    if(attempt_count < 4) {
                        print_Line(attempt_count, " NULL");
                        attempt_count++;
                        ClearInput();
                    }
                }
            }
        }
        last_key = keyin;              // 記錄當前按鍵供下次比較
    }
}