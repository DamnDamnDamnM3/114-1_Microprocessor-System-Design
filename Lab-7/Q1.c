// have to add Scankey.c to User dir.

#include <stdio.h>
#include <stdlib.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "LCD.h"
#include "Draw2D.h"
#include "Scankey.h"

#define RADIUS 4
#define MOVE_DELAY 500000 // 0.5? (500ms)
#define STEP 8

void Buzz(void) {
    PB11 = 0; // ????
    CLK_SysTickDelay(50000);
    PB11 = 1; // ??
}

int main(void) {
    int x = 0, y = 32;
    int moveFlag = 0;  // 0=??, 1=??
    int exist = 0;     // 0=??, 1=???
    int key = 0;

    SYS_Init();
    init_LCD();
    clear_LCD();

    // ???????
    GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT);
    PB11 = 1;

    while (1) {
        key = ScanKey(); // ????
        CLK_SysTickDelay(10000); // ????

        if (key == 1 && exist == 0) { // ????
            x = -5;
            y = 32;
            exist = 1;
            moveFlag = 1;
        } 
        else if (key == 2) { // ??
            moveFlag = 0;
        } 
        else if (key == 3) { // ??
            moveFlag = 1;
        }

        if (exist && moveFlag) {
            clear_LCD();
            draw_Circle(x, y, RADIUS, FG_COLOR, BG_COLOR);
            
						x += STEP;
            if (x >= 128 + 4) { // ?????
                exist = 0;
                moveFlag = 0;
                clear_LCD();
                Buzz(); // ????
            }
						
            CLK_SysTickDelay(MOVE_DELAY);
						
        }
    }
}
