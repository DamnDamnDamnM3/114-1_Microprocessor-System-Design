//
// Lab7-2: 彈跳球體與目標方塊碰撞遊戲（Lance版本）
// 功能：控制球體移動，碰撞目標方塊使其消失，所有方塊消失後重置
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

// 像素狀態定義
#define PIXEL_ON 1	// 像素開啟（顯示）
#define PIXEL_OFF 0 // 像素關閉（隱藏）

// 球體初始位置定義
#define X0 64 // 球體初始X座標（螢幕中央，LCD寬度128/2=64）
#define Y0 60 // 球體初始Y座標（底部區域）

// 遊戲物件尺寸定義
#define RADIUS 3		// 球體半徑（像素）
#define BLOCK_SIZE 5	// 目標方塊尺寸（5x5像素）
#define MIN_DISTANCE 20 // 兩個方塊之間的最小水平距離（像素），避免方塊過於接近

/**
 * 產生兩個隨機位置目標方塊的函數
 * @param block1_x 方塊1的X座標指標（輸出參數）
 * @param block1_y 方塊1的Y座標指標（輸出參數）
 * @param block2_x 方塊2的X座標指標（輸出參數）
 * @param block2_y 方塊2的Y座標指標（輸出參數）
 * @param seed 隨機數種子指標（輸入/輸出參數）
 * 功能：使用線性同餘生成器產生兩個隨機位置的方塊，確保它們之間至少有MIN_DISTANCE的距離
 * 說明：使用LCG演算法（Linear Congruential Generator）產生偽隨機數
 */
void GenerateTwoBlocks(int16_t *block1_x, int16_t *block1_y, int16_t *block2_x, int16_t *block2_y, uint32_t *seed)
{
	int16_t distance; // 兩個方塊之間的水平距離

	// 產生第一個方塊的X座標
	// LCG公式：seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF
	// 0x7FFFFFFF確保結果為正數（最大32位元正整數）
	*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
	// 限制X座標範圍在2~125之間（避免方塊超出螢幕或貼邊）
	*block1_x = (*seed % (125 - 2 + 1)) + 2;

	// 產生第一個方塊的Y座標
	*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
	// 限制Y座標範圍在2~29之間（上半部區域，避免與球體初始位置重疊）
	*block1_y = (*seed % (29 - 2 + 1)) + 2;

	// 產生第二個方塊，並確保與第一個方塊的距離足夠
	do
	{
		// 產生第二個方塊的X座標
		*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
		*block2_x = (*seed % (125 - 2 + 1)) + 2;

		// 產生第二個方塊的Y座標
		*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
		*block2_y = (*seed % (29 - 2 + 1)) + 2;

		// 計算兩個方塊之間的水平距離（絕對值）
		if (*block2_x > *block1_x)
		{
			distance = *block2_x - *block1_x; // block2在右邊
		}
		else
		{
			distance = *block1_x - *block2_x; // block2在左邊
		}

	} while (distance < MIN_DISTANCE); // 如果距離不夠，重新產生直到滿足條件
}

/**
 * 檢查圓形球體與方形方塊是否發生碰撞
 * @param cx 圓心X座標
 * @param cy 圓心Y座標
 * @param cr 圓的半徑
 * @param bx 方塊中心X座標
 * @param by 方塊中心Y座標
 * @param bsize 方塊尺寸
 * @return 1=發生碰撞，0=未發生碰撞
 * 功能：使用軸對齊邊界框（AABB）碰撞檢測方法
 * 說明：將圓形視為正方形邊界框進行簡化的碰撞檢測，提高計算效率
 */
int CheckOverlap(int16_t cx, int16_t cy, int16_t cr, int16_t bx, int16_t by, int16_t bsize)
{
	int16_t circle_left, circle_right, circle_top, circle_bottom; // 圓形邊界框座標
	int16_t block_left, block_right, block_top, block_bottom;	  // 方塊邊界框座標
	int16_t half_size;											  // 方塊半邊長度

	half_size = bsize / 2; // 計算方塊半邊長度（用於計算邊界）

	// 計算圓形的邊界框（將圓形視為正方形）
	circle_left = cx - cr;	 // 左邊界 = 圓心X - 半徑
	circle_right = cx + cr;	 // 右邊界 = 圓心X + 半徑
	circle_top = cy - cr;	 // 上邊界 = 圓心Y - 半徑
	circle_bottom = cy + cr; // 下邊界 = 圓心Y + 半徑

	// 計算方塊的邊界框
	block_left = bx - half_size;   // 左邊界 = 方塊中心X - 半邊長
	block_right = bx + half_size;  // 右邊界 = 方塊中心X + 半邊長
	block_top = by - half_size;	   // 上邊界 = 方塊中心Y - 半邊長
	block_bottom = by + half_size; // 下邊界 = 方塊中心Y + 半邊長

	// AABB碰撞檢測：檢查兩個矩形邊界框是否重疊
	// 條件：圓形右邊界 >= 方塊左邊界 且 圓形左邊界 <= 方塊右邊界
	//       圓形下邊界 >= 方塊上邊界 且 圓形上邊界 <= 方塊下邊界
	if (circle_right >= block_left &&
		circle_left <= block_right &&
		circle_bottom >= block_top &&
		circle_top <= block_bottom)
	{
		return 1; // 發生碰撞
	}

	return 0; // 未發生碰撞
}

/**
 * 主程式入口
 * 功能：實作彈跳球體與目標方塊碰撞遊戲
 */
int32_t main(void)
{
	// --- 球體運動參數 ---
	int dirX, dirY;		  // 球體移動方向（-1/0/1分別表示負方向/停止/正方向）
	int movX, movY;		  // 球體每次移動的距離（像素）
	uint16_t r;			  // 球體半徑
	int16_t x, y;		  // 球體當前位置座標
	int16_t new_x, new_y; // 球體下一個位置的座標

	// --- 顯示顏色定義 ---
	uint16_t fgColor, bgColor; // 前景色和背景色

	// --- 按鍵處理變數 ---
	uint8_t keyin, last_key; // 當前按鍵值和上一個按鍵值（用於檢測按鍵釋放）

	// --- 遊戲狀態變數 ---
	int is_moving; // 球體是否正在移動（0=停止，1=移動中）
	int bounced;   // 是否發生邊界反彈（0=未反彈，1=已反彈）

	// --- 目標方塊相關變數 ---
	int16_t block1_x, block1_y, block2_x, block2_y; // 兩個方塊的位置座標
	int block1_visible, block2_visible;				// 兩個方塊的可見性標誌（0=已消失，1=可見）

	// --- 隨機數相關變數 ---
	uint32_t seedCounter; // 隨機數種子計數器

	// --- 碰撞檢測變數 ---
	int overlap; // 是否發生碰撞（0=未碰撞，1=已碰撞）

	// --- 迴圈計數變數 ---
	int16_t i, j; // 用於繪製方塊的迴圈計數變數

	// --- 初始化遊戲狀態 ---
	last_key = 0;		// 初始化上一個按鍵值為0（無按鍵）
	is_moving = 0;		// 初始化為停止狀態
	block1_visible = 1; // 方塊1初始為可見
	block2_visible = 1; // 方塊2初始為可見
	seedCounter = 0;	// 初始化隨機數種子計數器

	// --- 系統初始化 ---
	SYS_Init();	  // 系統初始化（時鐘、GPIO等基本設定）
	init_LCD();	  // LCD顯示器初始化
	clear_LCD();  // 清除LCD螢幕內容
	OpenKeyPad(); // 按鍵掃描功能初始化

	// --- 球體初始狀態設定 ---
	x = X0;		// 設定球體初始X座標（螢幕中央）
	y = Y0;		// 設定球體初始Y座標（底部區域）
	r = RADIUS; // 設定球體半徑
	movX = 0;	// 初始X方向移動距離為0
	movY = 0;	// 初始Y方向移動距離為0
	dirX = 0;	// 初始X方向為0（無方向）
	dirY = 0;	// 初始Y方向為0（無方向）

	// --- 顏色設定 ---
	bgColor = BG_COLOR; // 設定背景色
	fgColor = FG_COLOR; // 設定前景色

	// --- 初始化隨機數種子（使用SysTick計數器獲取隨機初始值） ---
	seedCounter = SysTick->VAL; // 使用系統計數器的當前值作為初始種子

	// --- 在啟動時產生兩個隨機位置的目標方塊 ---
	GenerateTwoBlocks(&block1_x, &block1_y, &block2_x, &block2_y, &seedCounter);

	// --- 繪製第一個方塊（5x5像素方塊） ---
	// 從方塊中心向四周繪製BLOCK_SIZE x BLOCK_SIZE的像素區域
	for (i = block1_x - BLOCK_SIZE / 2; i <= block1_x + BLOCK_SIZE / 2; i++)
	{
		for (j = block1_y - BLOCK_SIZE / 2; j <= block1_y + BLOCK_SIZE / 2; j++)
		{
			draw_Pixel(i, j, FG_COLOR, bgColor); // 繪製前景色像素（黑色方塊）
		}
	}

	// --- 繪製第二個方塊（5x5像素方塊） ---
	for (i = block2_x - BLOCK_SIZE / 2; i <= block2_x + BLOCK_SIZE / 2; i++)
	{
		for (j = block2_y - BLOCK_SIZE / 2; j <= block2_y + BLOCK_SIZE / 2; j++)
		{
			draw_Pixel(i, j, FG_COLOR, bgColor); // 繪製前景色像素（黑色方塊）
		}
	}

	// --- 繪製初始位置的球體 ---
	draw_Circle(x, y, r, fgColor, bgColor);

	// --- 主程式迴圈 ---
	while (1)
	{
		seedCounter++;	   // 持續遞增種子計數器，增加隨機性
		keyin = ScanKey(); // 掃描按鍵狀態

		// --- 按鍵處理邏輯（僅在按鍵按下且與上次不同時執行，實現按鍵釋放檢測） ---
		if (keyin != 0 && keyin != last_key)
		{
			switch (keyin)
			{
			case 4: // 按鍵4：向左移動
				dirX = -1;
				dirY = 0; // 設定X方向向左，Y方向為0
				movX = 3;
				movY = 0;	   // 每次移動3像素（僅X方向）
				is_moving = 1; // 標記為移動狀態
				break;

			case 6: // 按鍵6：向右移動
				dirX = 1;
				dirY = 0; // 設定X方向向右，Y方向為0
				movX = 3;
				movY = 0;	   // 每次移動3像素（僅X方向）
				is_moving = 1; // 標記為移動狀態
				break;

			case 3: // 按鍵3：右上45度方向移動
				dirX = 1;
				dirY = -1; // 設定X方向向右，Y方向向上
				movX = 3;
				movY = 3;	   // 每次移動3像素（對角線移動）
				is_moving = 1; // 標記為移動狀態
				break;

			case 9: // 按鍵9：右下45度方向移動
				dirX = 1;
				dirY = 1; // 設定X方向向右，Y方向向下
				movX = 3;
				movY = 3;	   // 每次移動3像素（對角線移動）
				is_moving = 1; // 標記為移動狀態
				break;

			case 1: // 按鍵1：左上45度方向移動
				dirX = -1;
				dirY = -1; // 設定X方向向左，Y方向向上
				movX = 3;
				movY = 3;	   // 每次移動3像素（對角線移動）
				is_moving = 1; // 標記為移動狀態
				break;

			case 7: // 按鍵7：左下45度方向移動
				dirX = -1;
				dirY = 1; // 設定X方向向左，Y方向向下
				movX = 3;
				movY = 3;	   // 每次移動3像素（對角線移動）
				is_moving = 1; // 標記為移動狀態
				break;

			case 5:			   // 按鍵5：停止移動
				is_moving = 0; // 標記為停止狀態
				movX = 0;	   // 清除X方向移動距離
				movY = 0;	   // 清除Y方向移動距離
				break;

			case 8: // 按鍵8：重新產生方塊（僅在兩個方塊都消失時有效）
				if (!block1_visible && !block2_visible)
				{
					// 重新產生兩個新的隨機位置方塊
					GenerateTwoBlocks(&block1_x, &block1_y, &block2_x, &block2_y, &seedCounter);
					block1_visible = 1; // 標記方塊1為可見
					block2_visible = 1; // 標記方塊2為可見

					// 清除整個LCD並重新繪製所有物件
					clear_LCD();

					// 重新繪製第一個方塊
					for (i = block1_x - BLOCK_SIZE / 2; i <= block1_x + BLOCK_SIZE / 2; i++)
						for (j = block1_y - BLOCK_SIZE / 2; j <= block1_y + BLOCK_SIZE / 2; j++)
							draw_Pixel(i, j, FG_COLOR, bgColor);

					// 重新繪製第二個方塊
					for (i = block2_x - BLOCK_SIZE / 2; i <= block2_x + BLOCK_SIZE / 2; i++)
						for (j = block2_y - BLOCK_SIZE / 2; j <= block2_y + BLOCK_SIZE / 2; j++)
							draw_Pixel(i, j, FG_COLOR, bgColor);

					// 重新繪製球體
					draw_Circle(x, y, r, FG_COLOR, bgColor);
				}
				break;
			}
		}
		last_key = keyin; // 記錄當前按鍵值，供下次比較使用

		// --- 球體移動處理邏輯（僅在移動狀態時執行） ---
		if (is_moving)
		{
			// 計算球體的下一個位置
			new_x = x + dirX * movX; // 新X座標 = 當前X + 方向 * 移動距離
			new_y = y + dirY * movY; // 新Y座標 = 當前Y + 方向 * 移動距離
			bounced = 0;			 // 初始化反彈標誌為未反彈

			// --- 邊界碰撞檢測與反彈處理 ---
			// 檢查左邊界（X座標不能小於半徑）
			if (new_x <= r)
			{
				dirX = 1;	 // 反彈向右
				new_x = r;	 // 限制在邊界位置
				bounced = 1; // 標記為已反彈
			}
			// 檢查右邊界（X座標不能大於127-半徑，LCD寬度為128，最大X為127）
			else if (new_x >= (127 - r))
			{
				dirX = -1;		 // 反彈向左
				new_x = 127 - r; // 限制在邊界位置
				bounced = 1;	 // 標記為已反彈
			}

			// 檢查上邊界（Y座標不能小於半徑）
			if (new_y <= r)
			{
				dirY = 1;	 // 反彈向下
				new_y = r;	 // 限制在邊界位置
				bounced = 1; // 標記為已反彈
			}
			// 檢查下邊界（Y座標不能大於63-半徑，LCD高度為64，最大Y為63）
			else if (new_y >= (63 - r))
			{
				dirY = -1;		// 反彈向上
				new_y = 63 - r; // 限制在邊界位置
				bounced = 1;	// 標記為已反彈
			}

			// --- 方塊碰撞檢測與處理 ---
			overlap = 0; // 初始化碰撞標誌為未碰撞

			// 檢查第一個方塊是否與球體碰撞
			if (block1_visible && CheckOverlap(new_x, new_y, r, block1_x, block1_y, BLOCK_SIZE))
			{
				// 碰撞發生：清除第一個方塊（用背景色繪製整個方塊區域）
				for (i = block1_x - BLOCK_SIZE / 2; i <= block1_x + BLOCK_SIZE / 2; i++)
					for (j = block1_y - BLOCK_SIZE / 2; j <= block1_y + BLOCK_SIZE / 2; j++)
						draw_Pixel(i, j, bgColor, bgColor); // 用背景色繪製，相當於清除
				block1_visible = 0;							// 標記方塊1為已消失
				overlap = 1;								// 標記為已碰撞
			}

			// 檢查第二個方塊是否與球體碰撞
			if (block2_visible && CheckOverlap(new_x, new_y, r, block2_x, block2_y, BLOCK_SIZE))
			{
				// 碰撞發生：清除第二個方塊
				for (i = block2_x - BLOCK_SIZE / 2; i <= block2_x + BLOCK_SIZE / 2; i++)
					for (j = block2_y - BLOCK_SIZE / 2; j <= block2_y + BLOCK_SIZE / 2; j++)
						draw_Pixel(i, j, bgColor, bgColor); // 用背景色繪製，相當於清除
				block2_visible = 0;							// 標記方塊2為已消失
				overlap = 1;								// 標記為已碰撞
			}

			// --- 檢查是否所有方塊都已消失（遊戲結束條件） ---
			if (!block1_visible && !block2_visible)
			{
				// 清除當前位置的球體
				draw_Circle(x, y, r, BG_COLOR, bgColor);

				// 重置球體到初始位置
				x = X0;
				y = Y0;

				// 停止移動並清除移動參數
				is_moving = 0;
				movX = 0;
				movY = 0;
				dirX = 0;
				dirY = 0;

				// 在初始位置重新繪製球體
				draw_Circle(x, y, r, FG_COLOR, bgColor);

				// 短延遲後繼續
				CLK_SysTickDelay(10000);
				continue; // 跳過後續繪製步驟，直接進入下一次迴圈
			}

			// --- 正常移動處理：清除舊位置並繪製新位置 ---
			// 清除舊位置的球體
			draw_Circle(x, y, r, BG_COLOR, bgColor);

			// 重新繪製仍然可見的方塊（避免球體移動時擦除方塊）
			if (block1_visible)
				for (i = block1_x - BLOCK_SIZE / 2; i <= block1_x + BLOCK_SIZE / 2; i++)
					for (j = block1_y - BLOCK_SIZE / 2; j <= block1_y + BLOCK_SIZE / 2; j++)
						draw_Pixel(i, j, FG_COLOR, bgColor);
			if (block2_visible)
				for (i = block2_x - BLOCK_SIZE / 2; i <= block2_x + BLOCK_SIZE / 2; i++)
					for (j = block2_y - BLOCK_SIZE / 2; j <= block2_y + BLOCK_SIZE / 2; j++)
						draw_Pixel(i, j, FG_COLOR, bgColor);

			// 更新球體位置
			x = new_x;
			y = new_y;

			// 在新位置繪製球體
			draw_Circle(x, y, r, FG_COLOR, bgColor);

			// 動畫延遲（控制移動速度）
			CLK_SysTickDelay(100000); // 延遲100毫秒
		}
		// 當球體停止時
		else
		{
			CLK_SysTickDelay(10000); // 短延遲10毫秒，降低CPU使用率
		}
	}
}
