#include <stdio.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "Seven_Segment.h"
#include "Scankey.h"

void Init_GPIO(void)
{
    GPIO_SetMode(PA, BIT12, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PA, BIT13, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PA, BIT14, GPIO_MODE_OUTPUT);
    PA12 = 1;
    PA13 = 1;
    PA14 = 1;
}

void Display_7seg(uint16_t value)
{
    uint8_t digit;

    digit = value / 1000;
    CloseSevenSegment();
    ShowSevenSegment(3, digit);
    CLK_SysTickDelay(200);

    value %= 1000;
    digit = value / 100;
    CloseSevenSegment();
    ShowSevenSegment(2, digit);
    CLK_SysTickDelay(200);

    value %= 100;
    digit = value / 10;
    CloseSevenSegment();
    ShowSevenSegment(1, digit);
    CLK_SysTickDelay(200);

    digit = value % 10;
    CloseSevenSegment();
    ShowSevenSegment(0, digit);
    CLK_SysTickDelay(200);
}

enum State { GREEN, YELLOW, RED };

struct TrafficSignal {
    enum State state;
    int greenDuration, yellowDuration, redDuration;
    int timer;
};

void TrafficSignal_initialize(struct TrafficSignal *ts, int g, int y, int r)
{
    ts->state = GREEN;
    ts->greenDuration = g;
    ts->yellowDuration = y;
    ts->redDuration = r;
    ts->timer = g;
}

void TrafficSignal_countDown(struct TrafficSignal *ts)
{
    ts->timer--;
    if (ts->timer <= 0)
    {
        switch (ts->state)
        {
        case GREEN:
            ts->state = YELLOW;
            ts->timer = ts->yellowDuration;
            break;
        case YELLOW:
            ts->state = RED;
            ts->timer = ts->redDuration;
            break;
        case RED:
        default:
            ts->state = GREEN;
            ts->timer = ts->greenDuration;
            break;
        }
    }
}

void Show_LED(enum State s)
{
    switch (s)
    {
    case GREEN:
        PA12 = 1;
        PA13 = 0;
        PA14 = 1;
        break;
    case YELLOW:
        PA12 = 1;
        PA13 = 0;
        PA14 = 0;
        break;
    case RED:
        PA12 = 1;
        PA13 = 1;
        PA14 = 0;
        break;
    }
}

int main(void)
{
    struct TrafficSignal ts;
    int key = 0, last_key = 0;
    int tick = 0;

    SYS_Init();
    Init_GPIO();
    OpenSevenSegment();
    OpenKeyPad();

    TrafficSignal_initialize(&ts, 8, 5, 13);

    while (1)
    {
        key = ScanKey();
        if (key != 0 && key != last_key)
        {
            last_key = key;
            switch (key)
            {
            case 1:
                ts.state = GREEN;
                ts.timer = ts.greenDuration;
                break;
            case 2:
                ts.state = YELLOW;
                ts.timer = ts.yellowDuration;
                break;
            case 3:
                ts.state = RED;
                ts.timer = ts.redDuration;
                break;
            case 9:
                ts.timer += 5;
                if (ts.timer > 99) ts.timer = 99;
                break;
            default:
                break;
            }
        }
        else if (key == 0)
        {
            last_key = 0;
        }

        Display_7seg(ts.timer);
        Show_LED(ts.state);

        tick++;
        if (tick >= 900)
        {
            TrafficSignal_countDown(&ts);
            tick = 0;
        }
    }
}
