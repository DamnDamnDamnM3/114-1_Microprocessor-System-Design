//
// GPIO_Keypad : 3x3 keypad input and control LEDs (or Relays)
//
// EVB : Nu-LB-NUC140
// MCU : NUC140VE3CN

// PA0,1,2,3,4,5 connected to 3x3 Keypad
// PC12,13,14,15 connected to LEDs (or Relays)

#include <stdio.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "Scankey.h"

void Init_GPIO(void)
{
	  GPIO_SetMode(PC, BIT12, GPIO_MODE_OUTPUT);
	  GPIO_SetMode(PC, BIT13, GPIO_MODE_OUTPUT);
	  GPIO_SetMode(PC, BIT14, GPIO_MODE_OUTPUT);
	  GPIO_SetMode(PC, BIT15, GPIO_MODE_OUTPUT);
	  PC12=1; PC13=1; PC14=1; PC15=1;
}

int main(void)
{
	uint32_t i =0;
	SYS_Init();
	OpenKeyPad();
	Init_GPIO();

 	while(1) 
  {
		i=ScanKey();
		switch(i) {
			// D1210799
			
			// 1
			case 1 : PC12=1; PC13=1; PC14=1; PC15=0; break;
			// 2
			case 2 : PC12=1; PC13=1; PC14=0; PC15=1; break;
			// 1
			case 3 : PC12=1; PC13=1; PC14=1; PC15=0; break;
			// 0
			case 4 : PC12=0; PC13=0; PC14=0; PC15=0; break;
			// 7
			case 5 : PC12=1; PC13=0; PC14=0; PC15=0; break;
			// 9
			case 6 : PC12=0; PC13=1; PC14=1; PC15=0; break;
			// 9
			case 7 : PC12=0; PC13=1; PC14=1; PC15=0; break;
			default: PC12=1; PC13=1; PC14=1; PC15=1; break;
		}	
	}
}
