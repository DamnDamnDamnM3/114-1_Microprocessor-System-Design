#include <stdio.h>
#include <stdlib.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "Seven_Segment.h"
#include "Scankey.h"

void Display_Lab4_1(int remainder, int randomNum)
{
    uint8_t d0, d1, d3;
    d1 = (randomNum / 10) % 10;
    d0 = randomNum % 10;
    d3 = remainder % 10;

    CloseSevenSegment();
    ShowSevenSegment(3, d3);
    CLK_SysTickDelay(5000);

    CloseSevenSegment();
    CLK_SysTickDelay(5000);

    CloseSevenSegment();
    ShowSevenSegment(1, d1);
    CLK_SysTickDelay(5000);

    CloseSevenSegment();
    ShowSevenSegment(0, d0);
    CLK_SysTickDelay(5000);
}

int main(void)
{
    int key = 0;
    int randomNum = 0;
    int remainder = 0;
    int seedCounter = 0;
    int clearFlag = 1;

    SYS_Init();
    OpenSevenSegment();
    OpenKeyPad();
    CloseSevenSegment();

    while (1)
    {
        seedCounter++;
        key = ScanKey();

        if (key != 0)
        {
            if (key == 3)
            {
                srand(seedCounter);
                randomNum = rand() % 100;
                clearFlag = 0;
            }
            else if (key == 1)
                remainder = randomNum % 2;
            else if (key == 4)
                remainder = randomNum % 3;
            else if (key == 7)
                remainder = randomNum % 5;
            else if (key == 9)
            {
                randomNum = 0;
                remainder = 0;
                clearFlag = 1;
                CloseSevenSegment();
            }
        }

        if (!clearFlag)
            Display_Lab4_1(remainder, randomNum);
        else
        {
            CloseSevenSegment();
            CLK_SysTickDelay(20000);
        }
    }
}
