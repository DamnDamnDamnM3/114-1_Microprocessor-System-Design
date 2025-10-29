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
#define Y0 60       // Circle initial Y (bottom area)
#define RADIUS 3    // Circle radius
#define BLOCK_SIZE 5 // Black block size (5x5)
#define MIN_DISTANCE 20 // Minimum horizontal distance between blocks

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

// Check if circle boundary overlaps with block boundary
int CheckOverlap(int16_t cx, int16_t cy, int16_t cr, int16_t bx, int16_t by, int16_t bsize)
{
	int16_t circle_left, circle_right, circle_top, circle_bottom;
	int16_t block_left, block_right, block_top, block_bottom;
	int16_t half_size;
	
	half_size = bsize / 2;
	
	circle_left = cx - cr;
	circle_right = cx + cr;
	circle_top = cy - cr;
	circle_bottom = cy + cr;
	
	block_left = bx - half_size;
	block_right = bx + half_size;
	block_top = by - half_size;
	block_bottom = by + half_size;
	
	if (circle_right >= block_left && 
	    circle_left <= block_right && 
	    circle_bottom >= block_top && 
	    circle_top <= block_bottom) {
		return 1;
	}
	
	return 0;
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
	
	last_key = 0;
	is_moving = 0;
	block1_visible = 1;
	block2_visible = 1;
	seedCounter = 0;
	
	SYS_Init();
	init_LCD();  
	clear_LCD();
	OpenKeyPad();
	
	x = X0;
	y = Y0;
	r = RADIUS;
	movX = 0;
	movY = 0;
	dirX = 0;
	dirY = 0;
	
	bgColor = BG_COLOR;
	fgColor = FG_COLOR;
	
	// Initialize seedCounter with SysTick
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
	
	draw_Circle(x, y, r, fgColor, bgColor);
	
	while(1) {
		seedCounter++;
		keyin = ScanKey();
		
		if (keyin != 0 && keyin != last_key) {
			switch(keyin) {
				case 4: dirX=-1; dirY=0; movX=3; movY=0; is_moving=1; break;
				case 6: dirX=1; dirY=0; movX=3; movY=0; is_moving=1; break;
				case 3: dirX=1; dirY=-1; movX=3; movY=3; is_moving=1; break;
				case 9: dirX=1; dirY=1; movX=3; movY=3; is_moving=1; break;
				case 1: dirX=-1; dirY=-1; movX=3; movY=3; is_moving=1; break;
				case 7: dirX=-1; dirY=1; movX=3; movY=3; is_moving=1; break;
				case 5: is_moving=0; movX=0; movY=0; break;
				case 8: // ??????
					if (!block1_visible && !block2_visible) {
						GenerateTwoBlocks(&block1_x, &block1_y, &block2_x, &block2_y, &seedCounter);
						block1_visible = 1;
						block2_visible = 1;
						clear_LCD();
						
						for (i = block1_x - BLOCK_SIZE/2; i <= block1_x + BLOCK_SIZE/2; i++)
							for (j = block1_y - BLOCK_SIZE/2; j <= block1_y + BLOCK_SIZE/2; j++)
								draw_Pixel(i, j, FG_COLOR, bgColor);
						
						for (i = block2_x - BLOCK_SIZE/2; i <= block2_x + BLOCK_SIZE/2; i++)
							for (j = block2_y - BLOCK_SIZE/2; j <= block2_y + BLOCK_SIZE/2; j++)
								draw_Pixel(i, j, FG_COLOR, bgColor);
						
						draw_Circle(x, y, r, FG_COLOR, bgColor);
					}
					break;
			}
		}
		last_key = keyin;
		
		if (is_moving) {
			new_x = x + dirX * movX;
			new_y = y + dirY * movY;
			bounced = 0;
			
			if (new_x <= r) { dirX=1; new_x=r; bounced=1; }
			else if (new_x >= (127 - r)) { dirX=-1; new_x=127 - r; bounced=1; }
			if (new_y <= r) { dirY=1; new_y=r; bounced=1; }
			else if (new_y >= (63 - r)) { dirY=-1; new_y=63 - r; bounced=1; }
			
			overlap = 0;
			if (block1_visible && CheckOverlap(new_x, new_y, r, block1_x, block1_y, BLOCK_SIZE)) {
				for (i = block1_x - BLOCK_SIZE/2; i <= block1_x + BLOCK_SIZE/2; i++)
					for (j = block1_y - BLOCK_SIZE/2; j <= block1_y + BLOCK_SIZE/2; j++)
						draw_Pixel(i, j, bgColor, bgColor);
				block1_visible = 0; overlap = 1;
			}
			if (block2_visible && CheckOverlap(new_x, new_y, r, block2_x, block2_y, BLOCK_SIZE)) {
				for (i = block2_x - BLOCK_SIZE/2; i <= block2_x + BLOCK_SIZE/2; i++)
					for (j = block2_y - BLOCK_SIZE/2; j <= block2_y + BLOCK_SIZE/2; j++)
						draw_Pixel(i, j, bgColor, bgColor);
				block2_visible = 0; overlap = 1;
			}
			
			if (!block1_visible && !block2_visible) {
				draw_Circle(x, y, r, BG_COLOR, bgColor);
				x = X0; y = Y0;
				is_moving = 0; dirX = dirY = 0;
				draw_Circle(x, y, r, FG_COLOR, bgColor);
				CLK_SysTickDelay(10000);
				continue;
			}
			
			draw_Circle(x, y, r, BG_COLOR, bgColor);
			if (block1_visible)
				for (i = block1_x - BLOCK_SIZE/2; i <= block1_x + BLOCK_SIZE/2; i++)
					for (j = block1_y - BLOCK_SIZE/2; j <= block1_y + BLOCK_SIZE/2; j++)
						draw_Pixel(i, j, FG_COLOR, bgColor);
			if (block2_visible)
				for (i = block2_x - BLOCK_SIZE/2; i <= block2_x + BLOCK_SIZE/2; i++)
					for (j = block2_y - BLOCK_SIZE/2; j <= block2_y + BLOCK_SIZE/2; j++)
						draw_Pixel(i, j, FG_COLOR, bgColor);
			
			x = new_x; y = new_y;
			draw_Circle(x, y, r, FG_COLOR, bgColor);
			
			CLK_SysTickDelay(100000);
		} else {
			CLK_SysTickDelay(10000);
		}
	}
}
