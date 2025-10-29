#include <stdio.h>
#include <math.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "LCD.h"
#include "Scankey.h"
#include "Draw2D.h"

#define LCD_W 128
#define LCD_H 64

#define RADIUS   4
#define STEP_X   8          // ? ??:?? 2 ??
#define TICK_US  500000     // 0.5 ?/?(???)

// --- Buzzer on PB11 (Active-Low) ---
#define BUZZ_PIN_MASK   BIT11
static void BUZZ_Init(void){
    GPIO_SetMode(PB, BUZZ_PIN_MASK, GPIO_PMD_OUTPUT);
    PB11 = 1; // OFF
}
static void BUZZ_Beep(uint32_t us){
    PB11 = 0;                 // ON
    CLK_SysTickDelay(us);
    PB11 = 1;                 // OFF
}

static void draw_ball(int x, int y, uint16_t color){
    draw_Circle(x, y, RADIUS, color, BG_COLOR);
}

int main(void)
{
    int active = 0;         // ????
    int paused = 0;         // ????
    int cx = RADIUS;        // ?? x
    const int cy = 32;      // ?? y=32
    const int max_cx = (LCD_W - 1) - RADIUS; // ?????????

    SYS_Init();
    init_LCD();
    clear_LCD();
    OpenKeyPad();
    BUZZ_Init();

    while (1){
        int next_cx;              // ? ??????????
        int key = ScanKey();      // 1/2/3

        if (key == 1){
            if (!active){
                cx = RADIUS;
                draw_ball(cx, cy, FG_COLOR);
                active = 1;
                paused = 0;
            }
        } else if (key == 2){
            if (active) paused = 1;
        } else if (key == 3){
            if (active) paused = 0;
        }

        if (active && !paused){
            CLK_SysTickDelay(TICK_US);

            // ????
            draw_ball(cx, cy, BG_COLOR);

            // ?????(???????,????)
            next_cx = cx + STEP_X;
            if (next_cx <= max_cx){
                cx = next_cx;
                draw_ball(cx, cy, FG_COLOR);
            } else {
                // ??????,???????????
                cx = max_cx;
                draw_ball(cx, cy, FG_COLOR);

                // ??? ? ?? ? ????
                BUZZ_Beep(100000);          // 0.1 ?
                draw_ball(cx, cy, BG_COLOR);
                active = 0;
                paused = 0;
            }
        } else {
            CLK_SysTickDelay(10000); // 10ms
        }
    }
}
