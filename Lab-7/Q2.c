//
// Lab7-2: Bouncing Ball with Target
// EVB : Nu-LB-NUC140
// MCU : NUC140VE3CN
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "LCD.h"
#include "Draw2D.h"
#include "Scankey.h"

#define PIXEL_ON  1
#define PIXEL_OFF 0
#define X0 64       // Circle initial X (center bottom)
#define Y0 60       // Circle initial Y (bottom area, changed from 32)
#define RADIUS 3    // Circle radius
#define BLOCK_SIZE 5 // Black block size (5x5)
#define MIN_DISTANCE 20 // Minimum horizontal distance between blocks

// Buzzer function
void Buzz(int number)
{
	int i;
	//for (i = 0; i < number; i++) {
		//PB11 = 0; // PB11 = 0 to turn on Buzzer
		//CLK_SysTickDelay(10000);  // Delay
		//PB11 = 1; // PB11 = 1 to turn off Buzzer
		//CLK_SysTickDelay(10000);  // Delay
	//}
}

// Function to generate two random blocks with minimum distance
void GenerateTwoBlocks(int16_t* block1_x, int16_t* block1_y, int16_t* block2_x, int16_t* block2_y, uint32_t* seed)
{
	int16_t distance;
	
	// Generate first block
	*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
	*block1_x = (*seed % (125 - 2 + 1)) + 2;
	
	*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
	*block1_y = (*seed % (29 - 2 + 1)) + 2;
	
	// Generate second block with distance check
	do {
		*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
		*block2_x = (*seed % (125 - 2 + 1)) + 2;
		
		*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
		*block2_y = (*seed % (29 - 2 + 1)) + 2;
		
		// Calculate horizontal distance
		if (*block2_x > *block1_x) {
			distance = *block2_x - *block1_x;
		} else {
			distance = *block1_x - *block2_x;
		}
		
	} while (distance < MIN_DISTANCE);  // Repeat until distance >= 20
}

// Function to check if circle boundary overlaps with block boundary
// Treat circle as a square bounding box for simpler collision detection
int CheckOverlap(int16_t cx, int16_t cy, int16_t cr, int16_t bx, int16_t by, int16_t bsize)
{
	int16_t circle_left, circle_right, circle_top, circle_bottom;
	int16_t block_left, block_right, block_top, block_bottom;
	int16_t half_size;
	
	half_size = bsize / 2;
	
	// Calculate circle bounding box (treat circle as square)
	circle_left = cx - cr;
	circle_right = cx + cr;
	circle_top = cy - cr;
	circle_bottom = cy + cr;
	
	// Calculate block bounding box
	block_left = bx - half_size;
	block_right = bx + half_size;
	block_top = by - half_size;
	block_bottom = by + half_size;
	
	// Check if bounding boxes overlap (AABB collision)
	if (circle_right >= block_left && 
	    circle_left <= block_right && 
	    circle_bottom >= block_top && 
	    circle_top <= block_bottom) {
		return 1; // Collision detected
	}
	
	return 0; // No collision
}

int32_t main (void)
{
	int dirX, dirY;
	int movX, movY;
	uint16_t r;
	int16_t x, y;
	int16_t new_x, new_y;
	uint16_t fgColor, bgColor;
	uint8_t keyin, last_key;
	int is_moving;
	int bounced;
	int16_t block1_x, block1_y, block2_x, block2_y;
	int block1_visible, block2_visible;
	uint32_t seedCounter;
	int overlap;
	int16_t i, j;
	
	// Initialize variables
	last_key = 0;
	is_moving = 0;
	block1_visible = 1;
	block2_visible = 1;
	seedCounter = 0;
	
	SYS_Init();
	init_LCD();  
	clear_LCD();
	OpenKeyPad();
	
	// Initialize Buzzer
	GPIO_SetMode(PB, BIT11, GPIO_PMD_OUTPUT);
	PB11 = 1;
	
	x = X0;    // circle center x (64, center bottom)
	y = Y0;    // circle center y (60, bottom area)
	r = RADIUS;
	movX = 0;
	movY = 0;
	dirX = 0;
	dirY = 0;
	
	bgColor = BG_COLOR;
	fgColor = FG_COLOR;
	
	// Initialize seedCounter with SysTick for randomness
	seedCounter = SysTick->VAL;
	
	// Generate two random blocks at startup
	GenerateTwoBlocks(&block1_x, &block1_y, &block2_x, &block2_y, &seedCounter);
	
	// Draw first block
	for (i = block1_x - BLOCK_SIZE/2; i <= block1_x + BLOCK_SIZE/2; i++) {
		for (j = block1_y - BLOCK_SIZE/2; j <= block1_y + BLOCK_SIZE/2; j++) {
			draw_Pixel(i, j, FG_COLOR, bgColor);
		}
	}
	
	// Draw second block
	for (i = block2_x - BLOCK_SIZE/2; i <= block2_x + BLOCK_SIZE/2; i++) {
		for (j = block2_y - BLOCK_SIZE/2; j <= block2_y + BLOCK_SIZE/2; j++) {
			draw_Pixel(i, j, FG_COLOR, bgColor);
		}
	}
	
	// Draw initial circle
	draw_Circle(x, y, r, fgColor, bgColor);
	
	while(1) {
		seedCounter++; // Keep incrementing for randomness
		
		// Scan keypad
		keyin = ScanKey();
		
		// Key-release detection
		if (keyin != 0 && keyin != last_key) {
			// Process key press
			switch(keyin) {
				case 4: // Left arrow ?
					if (!is_moving) {
						dirX = -1;
						dirY = 0;
						movX = 3;
						movY = 0;
						is_moving = 1;
					}
					break;
					
				case 6: // Right arrow ?
					if (!is_moving) {
						dirX = 1;
						dirY = 0;
						movX = 3;
						movY = 0;
						is_moving = 1;
					}
					break;
					
				case 3: // Key 3: 45??????
					if (!is_moving) {
						dirX = 1;
						dirY = -1;
						movX = 3;
						movY = 3;
						is_moving = 1;
					}
					break;
					
				case 9: // Key 9: 45??????
					if (!is_moving) {
						dirX = 1;
						dirY = 1;
						movX = 3;
						movY = 3;
						is_moving = 1;
					}
					break;
					
				case 1: // Key 1: 45??????
					if (!is_moving) {
						dirX = -1;
						dirY = -1;
						movX = 3;
						movY = 3;
						is_moving = 1;
					}
					break;
					
				case 7: // Key 7: 45??????
					if (!is_moving) {
						dirX = -1;
						dirY = 1;
						movX = 3;
						movY = 3;
						is_moving = 1;
					}
					break;
					
				case 5: // Key S (Stop)
					is_moving = 0;
					movX = 0;
					movY = 0;
					break;
					
				case 8: // Key 8: R (Random) - Generate two new random blocks
					// Only generate new blocks if both blocks are destroyed
					if (!block1_visible && !block2_visible) {
						// Generate new random blocks using seedCounter
						GenerateTwoBlocks(&block1_x, &block1_y, &block2_x, &block2_y, &seedCounter);
						block1_visible = 1;
						block2_visible = 1;
						
						// Clear LCD and redraw everything
						clear_LCD();
						
						// Draw first block using FG_COLOR
						for (i = block1_x - BLOCK_SIZE/2; i <= block1_x + BLOCK_SIZE/2; i++) {
							for (j = block1_y - BLOCK_SIZE/2; j <= block1_y + BLOCK_SIZE/2; j++) {
								draw_Pixel(i, j, FG_COLOR, bgColor);
							}
						}
						
						// Draw second block using FG_COLOR
						for (i = block2_x - BLOCK_SIZE/2; i <= block2_x + BLOCK_SIZE/2; i++) {
							for (j = block2_y - BLOCK_SIZE/2; j <= block2_y + BLOCK_SIZE/2; j++) {
								draw_Pixel(i, j, FG_COLOR, bgColor);
							}
						}
						
						// Redraw circle at current position
						fgColor = FG_COLOR;
						draw_Circle(x, y, r, fgColor, bgColor);
					}
					break;
			}
		}
		
		last_key = keyin;
		
		// Only move and draw if moving
		if (is_moving) {
			// Calculate new position first
			new_x = x + dirX * movX;
			new_y = y + dirY * movY;
			
			// Boundary check and reflection (LCD is 128x64, so 0-127 for X, 0-63 for Y)
			bounced = 0;
			
			// Check X boundaries
			if (new_x <= r) {
				// Hit left boundary
				if (dirX != 0) {
					dirX = 1; // Bounce right (??)
				}
				new_x = r;
				bounced = 1;
			} else if (new_x >= (127 - r)) {
				// Hit right boundary (LCD X max = 127)
				if (dirX != 0) {
					dirX = -1; // Bounce left (??)
				}
				new_x = 127 - r;
				bounced = 1;
			}
			
			// Check Y boundaries
			if (new_y <= r) {
				// Hit top boundary
				if (dirY != 0) {
					dirY = 1; // Bounce down (??)
				}
				new_y = r;
				bounced = 1;
			} else if (new_y >= (63 - r)) {
				// Hit bottom boundary (LCD Y max = 63)
				if (dirY != 0) {
					dirY = -1; // Bounce up (??)
				}
				new_y = 63 - r;
				bounced = 1;
			}
			
			// Step 1: Check collision at CURRENT position and erase blocks if overlapping NOW
			overlap = 0;
			
			if (block1_visible) {
				if (CheckOverlap(x, y, r, block1_x, block1_y, BLOCK_SIZE)) {
					// Already overlapping! Erase block 1 immediately
					for (i = block1_x - BLOCK_SIZE/2; i <= block1_x + BLOCK_SIZE/2; i++) {
						for (j = block1_y - BLOCK_SIZE/2; j <= block1_y + BLOCK_SIZE/2; j++) {
							draw_Pixel(i, j, bgColor, bgColor);
						}
					}
					block1_visible = 0;
					overlap = 1;
				}
			}
			
			if (block2_visible) {
				if (CheckOverlap(x, y, r, block2_x, block2_y, BLOCK_SIZE)) {
					// Already overlapping! Erase block 2 immediately
					for (i = block2_x - BLOCK_SIZE/2; i <= block2_x + BLOCK_SIZE/2; i++) {
						for (j = block2_y - BLOCK_SIZE/2; j <= block2_y + BLOCK_SIZE/2; j++) {
							draw_Pixel(i, j, bgColor, bgColor);
						}
					}
					block2_visible = 0;
					overlap = 1;
				}
			}
			
			// Step 2: Check collision at NEW position and erase blocks if will overlap
			if (block1_visible) {
				if (CheckOverlap(new_x, new_y, r, block1_x, block1_y, BLOCK_SIZE)) {
					// Will collide! Erase block 1
					for (i = block1_x - BLOCK_SIZE/2; i <= block1_x + BLOCK_SIZE/2; i++) {
						for (j = block1_y - BLOCK_SIZE/2; j <= block1_y + BLOCK_SIZE/2; j++) {
							draw_Pixel(i, j, bgColor, bgColor);
						}
					}
					block1_visible = 0;
					overlap = 1;
				}
			}
			
			if (block2_visible) {
				if (CheckOverlap(new_x, new_y, r, block2_x, block2_y, BLOCK_SIZE)) {
					// Will collide! Erase block 2
					for (i = block2_x - BLOCK_SIZE/2; i <= block2_x + BLOCK_SIZE/2; i++) {
						for (j = block2_y - BLOCK_SIZE/2; j <= block2_y + BLOCK_SIZE/2; j++) {
							draw_Pixel(i, j, bgColor, bgColor);
						}
					}
					block2_visible = 0;
					overlap = 1;
				}
			}
			
			// Step 3: Check if both blocks destroyed - reset immediately WITHOUT moving
			if (!block1_visible && !block2_visible) {
				// Erase circle at current position
				fgColor = BG_COLOR;
				draw_Circle(x, y, r, fgColor, bgColor);
				
				// Reset to starting position
				x = X0;
				y = Y0;
				is_moving = 0;
				movX = 0;
				movY = 0;
				dirX = 0;
				dirY = 0;
				
				// Draw circle at starting position
				fgColor = FG_COLOR;
				draw_Circle(x, y, r, fgColor, bgColor);
				
				// Buzz for collision
				if (overlap) {
					Buzz(1);
				}
				
				// Delay and continue
				CLK_SysTickDelay(10000);
				continue;
			}
			
			// Step 4: Now safe to erase old circle and move
			fgColor = BG_COLOR;
			draw_Circle(x, y, r, fgColor, bgColor);
			
			// Step 5: Redraw any remaining visible blocks (in case circle erased them)
			if (block1_visible) {
				for (i = block1_x - BLOCK_SIZE/2; i <= block1_x + BLOCK_SIZE/2; i++) {
					for (j = block1_y - BLOCK_SIZE/2; j <= block1_y + BLOCK_SIZE/2; j++) {
						draw_Pixel(i, j, FG_COLOR, bgColor);
					}
				}
			}
			
			if (block2_visible) {
				for (i = block2_x - BLOCK_SIZE/2; i <= block2_x + BLOCK_SIZE/2; i++) {
					for (j = block2_y - BLOCK_SIZE/2; j <= block2_y + BLOCK_SIZE/2; j++) {
						draw_Pixel(i, j, FG_COLOR, bgColor);
					}
				}
			}
			
			// Update position to new position
			x = new_x;
			y = new_y;
			
			// Draw circle at new position
			fgColor = FG_COLOR;
			draw_Circle(x, y, r, fgColor, bgColor);
			
			// Buzz if bounced or hit block
			if (bounced || overlap) {
				Buzz(1);
			}
			
			// Delay for animation
			CLK_SysTickDelay(50000);
		} else {
			// Small delay when stopped to reduce CPU usage
			CLK_SysTickDelay(10000);
		}
	}
}


