#include <stdio.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "Scankey.h"

void Buzz(int number) {
    int i;
    for (i = 0; i < number; i++) {
        PB11 = 0;
        CLK_SysTickDelay(100000);
        PB11 = 1;
        CLK_SysTickDelay(100000);
    }
}


void Display_binary(int value) {
    switch(value) {
			// 1
			case 1 : PC12=1; PC13=1; PC14=1; PC15=0; break;
			// 2
			case 2 : PC12=1; PC13=1; PC14=0; PC15=1; break;
			// 3
			case 3 : PC12=1; PC13=1; PC14=0; PC15=0; break;
			// 4
			case 4 : PC12=1; PC13=0; PC14=1; PC15=1; break;
			// 5
			case 5 : PC12=1; PC13=0; PC14=1; PC15=0; break;
			// 6
			case 6 : PC12=1; PC13=0; PC14=0; PC15=1; break;
			// 7
			case 7 : PC12=1; PC13=0; PC14=0; PC15=0; break;
			// 8
			case 8 : PC12=0; PC13=1; PC14=1; PC15=1; break;
			// 9
			case 9 : PC12=0; PC13=1; PC14=1; PC15=0; break;
	}
}

int main(void) {
    int key, lastKey = 0;

    SYS_Init();
    OpenKeyPad();


    GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PA, BIT12 | BIT13 | BIT14 | BIT15, GPIO_MODE_OUTPUT);

    PB11 = 1;

    while (1) {
        key = ScanKey();

        if (key != 0 && lastKey == 0) {
            while (ScanKey() != 0);
						Display_binary(key);
            Buzz(key);
        }
        lastKey = key;
    }
}
