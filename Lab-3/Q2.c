//
// GPIO_7seg_keypad_HOLA : 
//     Display scrolling HOLA text on 7-segment displays
//     Control scrolling with keypad: 4=left, 6=right, 5=pause, 8=reset
//
#include <stdio.h>
#include <math.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "Seven_Segment.h"
#include "Scankey.h"

// display an integer on four 7-segment LEDs
void Display_7seg(uint16_t value)
{
  uint8_t digit;
	digit = value / 1000;
	CloseSevenSegment();
	ShowSevenSegment(3,digit);
	CLK_SysTickDelay(5000);
			
	value = value - digit * 1000;
	digit = value / 100;
	CloseSevenSegment();
	ShowSevenSegment(2,digit);
	CLK_SysTickDelay(5000);

	value = value - digit * 100;
	digit = value / 10;
	CloseSevenSegment();
	ShowSevenSegment(1,digit);
	CLK_SysTickDelay(5000);

	value = value - digit * 10;
	digit = value;
	CloseSevenSegment();
	ShowSevenSegment(0,digit);
	CLK_SysTickDelay(5000);
}

// Function to display custom pattern on specified 7-segment display
void Segment_showPattern(int i, unsigned char pattern)
{
	CloseSevenSegment();
	// Set the pattern directly to the specified segment
	if (i >= 0 && i <= 3) {
		uint8_t temp, j;
		temp = pattern;
		
		// Set segment pattern bits (PE0-PE7) based on the library logic
		for (j = 0; j < 8; j++) {
			if ((temp & 0x01) == 0x01) {
				switch (j) {
					case 0: PE0 = 1; break;
					case 1: PE1 = 1; break;
					case 2: PE2 = 1; break;
					case 3: PE3 = 1; break;
					case 4: PE4 = 1; break;
					case 5: PE5 = 1; break;
					case 6: PE6 = 1; break;
					case 7: PE7 = 1; break;
				}
			} else {
				switch (j) {
					case 0: PE0 = 0; break;
					case 1: PE1 = 0; break;
					case 2: PE2 = 0; break;
					case 3: PE3 = 0; break;
					case 4: PE4 = 0; break;
					case 5: PE5 = 0; break;
					case 6: PE6 = 0; break;
					case 7: PE7 = 0; break;
				}
			}
			temp = temp >> 1;
		}
		
		// Enable the specific display position
		switch (i) {
			case 0: PC4 = 1; break;
			case 1: PC5 = 1; break;
			case 2: PC6 = 1; break;
			case 3: PC7 = 1; break;
		}
	}
}

int main(void)
{
	int k=0;
	int j=0;
	
	// HOLA scrolling variables - corrected patterns based on actual segment mapping
	// From image: PE7=G, PE6=E, PE5=D, PE4=B, PE3=A, PE2=F, PE1=DOT, PE0=C
	// Pattern format: G-E-D-B-A-F-DOT-C (bit 7 to bit 0)
	// Using correct values: H=0x2A, O=0x82, L=0x9B, A=0x22
	unsigned char hola_patterns[4] = {0x2A, 0x82, 0x9B, 0x22}; // H, O, L, A patterns
	int hola_position = 0; // Current starting position of HOLA (0-3)
	int scrolling = 0; // 1 = scrolling, 0 = paused (default: paused)
	int scroll_direction = 1; // 1 = right, -1 = left
	unsigned int scroll_counter = 0;
	
    SYS_Init();

    OpenSevenSegment(); // for 7-segment
	  OpenKeyPad();       // for keypad
	
 	  while(1) {
		  k=ScanKey();
			
			// Handle HOLA scrolling controls
			if (k == 4) { // Right scroll - HOLA -> AHOL -> LAHO -> OLAH
				if (scrolling == 0) { // First time pressed or after pause
					hola_position = (hola_position + 1) % 4; // Move immediately
				}
				scroll_direction = 1;
				scrolling = 1;
				scroll_counter = 0; // Reset counter for next scroll
			} else if (k == 6) { // Left scroll - HOLA -> OLAH -> LAHO -> AHOL  
				if (scrolling == 0) { // First time pressed or after pause
					hola_position = (hola_position - 1 + 4) % 4; // Move immediately
				}
				scroll_direction = -1;
				scrolling = 1;
				scroll_counter = 0; // Reset counter for next scroll
			} else if (k == 5) { // Pause
				scrolling = 0;
			} else if (k == 8) { // Reset to default HOLA
				hola_position = 0;
				scrolling = 0; // Reset to paused state
				scroll_direction = 1;
				scroll_counter = 0; // Reset counter
			}
			
			// Display HOLA on 7-segment displays (one at a time for multiplexing)
			for (j = 0; j < 4; j++) {
				int char_index = (hola_position + (3 - j)) % 4;
				uint8_t pattern = hola_patterns[char_index];
				
				CloseSevenSegment();
				
				// Set pattern for HOLA character directly using PE pins
				// Actual mapping: PE7=G, PE6=E, PE5=D, PE4=B, PE3=A, PE2=F, PE1=DOT, PE0=C
				PE0 = (pattern & 0x01) ? 1 : 0;  // C segment
				PE1 = (pattern & 0x02) ? 1 : 0;  // DOT segment  
				PE2 = (pattern & 0x04) ? 1 : 0;  // F segment
				PE3 = (pattern & 0x08) ? 1 : 0;  // A segment
				PE4 = (pattern & 0x10) ? 1 : 0;  // B segment
				PE5 = (pattern & 0x20) ? 1 : 0;  // D segment
				PE6 = (pattern & 0x40) ? 1 : 0;  // E segment
				PE7 = (pattern & 0x80) ? 1 : 0;  // G segment
				
				// Enable display position
				switch(j) {
					case 0: PC4 = 1; break;
					case 1: PC5 = 1; break; 
					case 2: PC6 = 1; break;
					case 3: PC7 = 1; break;
				}
				
				CLK_SysTickDelay(5000);  // Display time for each segment
			}
			
			// Auto-scroll HOLA every 1 second if scrolling is enabled
			if (scrolling) {
				scroll_counter++;
				if (scroll_counter >= 45) { // Approximately 1 second (45 cycles × ~22ms/cycle)
					scroll_counter = 0;
					hola_position = (hola_position + scroll_direction + 4) % 4;
				}
			}
			
			CLK_SysTickDelay(1000); // Small delay for main loop
	  }
}
