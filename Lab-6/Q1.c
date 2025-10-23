/*
 * ================================================================
 * Lab 6 - Question 1: 數字選擇與求和系統
 * 功能：從4個隨機數中選擇數字並計算總和，使用LCD顯示和LED指示
 * 硬體：NUC100系列微控制器
 * 作者：damnm3@googlegroups.com 共同作者
 * ================================================================
 * 
 * 硬體連接：
 * - PA0,1,2,3,4,5 連接至3x3按鍵矩陣
 * - PC12,13,14,15 連接至LED指示器
 * - PB11 連接至蜂鳴器
 * - LCD顯示器連接
 * 
 * 功能說明：
 * 1. 系統產生4個10-99的隨機數
 * 2. 使用者可以選擇最多4個數字
 * 3. LED指示已選擇的數字數量
 * 4. LCD顯示當前總和和可選擇的數字
 * 5. 支援重置、清除和返回功能
 * 
 * 按鍵對應：
 * - 按鍵4: 向上移動游標
 * - 按鍵6: 向下移動游標
 * - 按鍵5: 選擇當前數字
 * - 按鍵7: 重置系統
 * - 按鍵8: 清除選擇
 * - 按鍵9: 清除所有
 */

// 包含必要的標頭檔
#include <string.h>             // 字串處理函數
#include "NUC100Series.h"       // NUC100系列微控制器定義
#include "MCU_init.h"          // 微控制器初始化函數
#include "SYS_init.h"          // 系統初始化函數
#include "LCD.h"                // LCD顯示器控制函數
#include "Scankey.h"           // 按鍵掃描函數
#include "clk.h"               // 時鐘控制函數

// ================================================================
// 按鍵定義
// ================================================================
#define KEY_UP 4                // 向上鍵
#define KEY_DOWN 6              // 向下鍵
#define KEY_S 5                 // 選擇鍵
#define KEY_R 7                 // 重置鍵
#define KEY_B 8                 // 返回鍵
#define KEY_C 9                 // 清除鍵

// ================================================================
// 全域變數
// ================================================================
static uint32_t g_seed;         // 全域隨機數種子
/*
 * ================================================================
 * 自定義隨機數種子設定函數
 * 功能：設定隨機數種子
 * 參數：seed - 種子值
 * ================================================================
 */
void my_srand(uint32_t seed) { 
    g_seed = seed; 
}

/*
 * ================================================================
 * 自定義隨機數產生函數
 * 功能：產生隨機數
 * 返回值：隨機數
 * ================================================================
 */
int my_rand(void)
{
    // 如果種子為0（未初始化），設為1
    if (g_seed == 0) g_seed = 1;
   
    // 使用線性同餘生成器(LCG)產生隨機數
    g_seed = (1103515245 * g_seed + 12345) & 0x7FFFFFFF; // 標準LCG值
    return (int)g_seed;
}

/*
 * ================================================================
 * 簡單整數轉字串函數
 * 功能：將整數轉換為字串
 * 參數：val - 要轉換的整數, buf - 輸出緩衝區
 * ================================================================
 */
void simple_itoa(int val, char *buf)
{
    int i = 0;
    char temp[10];
    int j = 0;
   
    // 處理0的特殊情況
    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
   
    // 將數字轉換為字串（反向）
    while (val > 0) {
        temp[j++] = (val % 10) + '0';
        val /= 10;
    }
   
    // 反轉字串
    while (j > 0) {
        buf[i++] = temp[--j];
    }
   
    // 字串結尾
    buf[i] = '\0';
}

// ================================================================
// 系統狀態變數
// ================================================================
int numbers[4];                 // 4個隨機數
int sum = 0;                    // 當前總和
int cursor_pos = 0;             // 游標位置
int view_offset = 0;            // 視窗偏移
int selected_numbers[4];        // 已選擇的數字
int selected_count = 0;        // 已選擇數量


void init_leds(void)
{
    GPIO_SetMode(PC, BIT12 | BIT13 | BIT14 | BIT15, GPIO_PMD_OUTPUT);
    PC12 = 1; PC13 = 1; PC14 = 1; PC15 = 1; // LEDs off (assuming active-low)
}


void update_leds(void)
{
    // LEDs turn on based on the number of selected items (active-low)
    PC12 = (selected_count > 0) ? 0 : 1;
    PC13 = (selected_count > 1) ? 0 : 1;
    PC14 = (selected_count > 2) ? 0 : 1;
    PC15 = (selected_count > 3) ? 0 : 1;
}


void init_buzzer(void)
{
    GPIO_SetMode(PB, BIT11, GPIO_PMD_OUTPUT);
    PB11 = 1;
}


void Buzz(int number)
{
    int i;
    for (i=0; i<number; i++) {
        PB11=0; // PB11 = 0 to turn ON Buzzer
        CLK_SysTickDelay(100000);
        PB11=1; // PB11 = 1 to turn OFF Buzzer
        CLK_SysTickDelay(100000);
    }
}


void generate_numbers(void)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        numbers[i] = my_rand() % 90 + 10;
    }
}


void update_display(void)
{
    char line_buffer[16 + 1];
    char num_buffer[10];
    int i, j;

    
    for(i=0; i<16; i++) line_buffer[i] = ' ';
   

    line_buffer[0] = 'S'; line_buffer[1] = 'U'; line_buffer[2] = 'M';
    line_buffer[3] = ' '; line_buffer[4] = '='; line_buffer[5] = ' ';
   

    simple_itoa(sum, num_buffer);
   

    i = 0;
    while (num_buffer[i] != '\0' && (6 + i) < 16) {
        line_buffer[6 + i] = num_buffer[i];
        i++;
    }
    line_buffer[16] = '\0';
    print_Line(0, line_buffer);


    for (j = 0; j < 3; j++)
    {
        int num_index = j + view_offset;
        int has_cursor = (num_index == cursor_pos);

        for(i=0; i<16; i++) line_buffer[i] = ' ';

        line_buffer[0] = (has_cursor ? '>' : ' ');
        line_buffer[1] = ' '; // Space after cursor

        simple_itoa(numbers[num_index], num_buffer);

        i = 0;
        while (num_buffer[i] != '\0' && (2 + i) < 16) {
            line_buffer[2 + i] = num_buffer[i];
            i++;
        }
        line_buffer[16] = '\0';
        print_Line(j + 1, line_buffer);
    }
}

int main(void)
{
    uint8_t keyin;
    uint8_t last_keyin = 0;
    uint32_t count = 0;

    SYS_Init();
    init_LCD();
    clear_LCD();
    OpenKeyPad();
    init_leds();
    init_buzzer();

    generate_numbers();
    update_display();

    while (1)
    {
        count++;
        keyin = ScanKey();
       
        if (keyin == 0 && last_keyin != 0)
        {
            // Process the key that was just released (stored in last_keyin)
            switch (last_keyin)
            {
            case KEY_UP: // Up arrow
                if (cursor_pos > 0)
                {
                    cursor_pos--;
                    // Adjust view offset if cursor moves out of the 3 visible lines
                    view_offset = (cursor_pos == 3) ? 1 : 0;
                }
                break;

            case KEY_DOWN: // Down arrow
                if (cursor_pos < 3)
                {
                    cursor_pos++;
                    // Adjust view offset if cursor moves to the 4th item
                    view_offset = (cursor_pos == 3) ? 1 : 0;
                }
                break;

            case KEY_S: // Select
                // Only add if less than 4 numbers are selected
                if (selected_count < 4)
                {
                    int selected_val = numbers[cursor_pos];
                    sum += selected_val;
                    // Store the selected number for Backspace functionality
                    selected_numbers[selected_count] = selected_val;
                    selected_count++;
                    update_leds(); // Update LED status
                    Buzz(1);       // Beep once on selection (when key is released)
                }
                break;

            case KEY_R: // Reset
                my_srand(count); // Use the current count as the random seed
                generate_numbers(); // Generate new random numbers
                sum = 0;            // Reset sum
                selected_count = 0; // Reset selected count
                cursor_pos = 0;     // Reset cursor position
                view_offset = 0;    // Reset view offset
                update_leds();      // Turn off LEDs
                break;

            case KEY_B: // Backspace
                // Only perform backspace if numbers have been selected
                if (selected_count > 0)
                {
                    selected_count--; // Decrement selected count
                    // Subtract the last selected number from the sum
                    sum -= selected_numbers[selected_count];
                    update_leds(); // Update LED status
                }
                break;

            case KEY_C: // Clear
                sum = 0;            // Reset sum
                selected_count = 0; // Reset selected count
                // Cursor position and generated numbers remain unchanged
                update_leds();      // Turn off LEDs
                break;
               
            default:
                // Ignore other keys (1, 2, 3)
                break;
            }
           
            // Update the display only after processing the key release
            update_display();
        }
       
        // Update last_keyin for the next loop iteration
        last_keyin = keyin;
       
        // Small delay to reduce CPU load from constant polling
        CLK_SysTickDelay(10000); // 10ms delay
    }
    // The program should never exit the while(1) loop
    // return 0; // Usually unreachable in embedded systems
}
