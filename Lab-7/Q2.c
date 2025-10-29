//
// Lab7-2: 彈跳球體與目標方塊碰撞遊戲
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
#define X0 64	 // 球體初始X座標（螢幕中央，LCD寬度128/2=64）
#define Y0 60	 // 球體初始Y座標（底部區域，從原本的32改為60）
#define RADIUS 3 // 球體半徑（像素）

// 目標方塊相關定義
#define BLOCK_SIZE 5	// 目標方塊尺寸（5x5像素）
#define MIN_DISTANCE 20 // 兩個方塊之間的最小水平距離（像素），避免方塊過於接近

/**
 * 蜂鳴器響聲控制函數（目前為預留功能，程式碼被註解）
 * @param number 響聲次數（目前未使用）
 * 功能：控制蜂鳴器發出響聲
 * 說明：此函數預留給未來擴展使用，目前函數體內程式碼已被註解
 */
void Buzz(int number)
{
	int i;
	// for (i = 0; i < number; i++) {
	// PB11 = 0; // PB11 = 0 時開啟蜂鳴器
	// CLK_SysTickDelay(100000);  // 延遲
	// PB11 = 1; // PB11 = 1 時關閉蜂鳴器
	// CLK_SysTickDelay(100000);  // 延遲
	//}
}

/**
 * 產生兩個隨機位置目標方塊的函數
 * @param block1_x 方塊1的X座標指標（輸出參數）
 * @param block1_y 方塊1的Y座標指標（輸出參數）
 * @param block2_x 方塊2的X座標指標（輸出參數）
 * @param block2_y 方塊2的Y座標指標（輸出參數）
 * @param seed 隨機數種子指標（輸入/輸出參數）
 * 功能：使用線性同餘生成器產生兩個隨機位置的方塊，確保它們之間至少有MIN_DISTANCE的距離
 * 演算法：使用LCG（Linear Congruential Generator）偽隨機數產生器
 * 公式：seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF
 */
void GenerateTwoBlocks(int16_t *block1_x, int16_t *block1_y, int16_t *block2_x, int16_t *block2_y, uint32_t *seed)
{
	int16_t distance; // 兩個方塊之間的水平距離

	// 產生第一個方塊的X座標
	// 使用LCG演算法更新種子值，確保每次產生的值都不同
	*seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
	// 限制X座標範圍在2~125之間（避免方塊超出螢幕或貼邊）
	// 公式：(seed % (最大值-最小值+1)) + 最小值
	*block1_x = (*seed % (125 - 2 + 1)) + 2;

	// 產生第一個方塊的Y座標
	// 再次使用LCG演算法更新種子值
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
			distance = *block2_x - *block1_x; // block2在右邊，計算距離
		}
		else
		{
			distance = *block1_x - *block2_x; // block2在左邊，計算距離
		}

	} while (distance < MIN_DISTANCE); // 如果距離小於最小距離要求，重新產生直到滿足條件
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
 * 功能：使用軸對齊邊界框（AABB - Axis-Aligned Bounding Box）碰撞檢測方法
 * 說明：將圓形視為正方形邊界框進行簡化的碰撞檢測，提高計算效率
 * 原理：檢測兩個矩形邊界框是否重疊，比精確的圓形-矩形碰撞檢測更快
 */
int CheckOverlap(int16_t cx, int16_t cy, int16_t cr, int16_t bx, int16_t by, int16_t bsize)
{
	int16_t circle_left, circle_right, circle_top, circle_bottom; // 圓形邊界框座標
	int16_t block_left, block_right, block_top, block_bottom;	  // 方塊邊界框座標
	int16_t half_size;											  // 方塊半邊長度

	half_size = bsize / 2; // 計算方塊半邊長度（用於計算邊界範圍）

	// 計算圓形的邊界框（將圓形視為正方形，以圓心為中心，邊長為2*半徑）
	circle_left = cx - cr;	 // 左邊界 = 圓心X - 半徑
	circle_right = cx + cr;	 // 右邊界 = 圓心X + 半徑
	circle_top = cy - cr;	 // 上邊界 = 圓心Y - 半徑
	circle_bottom = cy + cr; // 下邊界 = 圓心Y + 半徑

	// 計算方塊的邊界框（以方塊中心為基準，向四周擴展半邊長度）
	block_left = bx - half_size;   // 左邊界 = 方塊中心X - 半邊長
	block_right = bx + half_size;  // 右邊界 = 方塊中心X + 半邊長
	block_top = by - half_size;	   // 上邊界 = 方塊中心Y - 半邊長
	block_bottom = by + half_size; // 下邊界 = 方塊中心Y + 半邊長

	// AABB碰撞檢測：檢查兩個矩形邊界框是否重疊
	// 兩個矩形重疊的條件：
	// 1. 圓形右邊界 >= 方塊左邊界 且 圓形左邊界 <= 方塊右邊界（水平方向重疊）
	// 2. 圓形下邊界 >= 方塊上邊界 且 圓形上邊界 <= 方塊下邊界（垂直方向重疊）
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
 * 遊戲規則：
 * 1. 使用按鍵控制球體移動方向
 * 2. 球體碰撞到目標方塊時，方塊消失
 * 3. 當所有方塊都消失時，球體重置到初始位置
 * 4. 按鍵8可在方塊都消失後重新產生新的方塊
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
	uint8_t keyin, last_key; // 當前按鍵值和上一個按鍵值（用於檢測按鍵釋放，避免重複觸發）

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

	// --- 初始化遊戲狀態變數 ---
	last_key = 0;		// 初始化上一個按鍵值為0（無按鍵）
	is_moving = 0;		// 初始化為停止狀態（球體不會自動移動）
	block1_visible = 1; // 方塊1初始為可見狀態
	block2_visible = 1; // 方塊2初始為可見狀態
	seedCounter = 0;	// 初始化隨機數種子計數器

	// --- 系統初始化 ---
	SYS_Init();	  // 系統初始化（時鐘、GPIO等基本設定）
	init_LCD();	  // LCD顯示器初始化
	clear_LCD();  // 清除LCD螢幕內容
	OpenKeyPad(); // 按鍵掃描功能初始化

	// --- 蜂鳴器初始化 ---
	GPIO_SetMode(PB, BIT11, GPIO_PMD_OUTPUT); // 設定PB11為輸出模式
	PB11 = 1;								  // 設定為高電位，關閉蜂鳴器（低電位觸發）

	// --- 球體初始狀態設定 ---
	x = X0;		// 設定球體初始X座標（螢幕中央，64）
	y = Y0;		// 設定球體初始Y座標（底部區域，60）
	r = RADIUS; // 設定球體半徑為3像素
	movX = 0;	// 初始X方向移動距離為0
	movY = 0;	// 初始Y方向移動距離為0
	dirX = 0;	// 初始X方向為0（無方向）
	dirY = 0;	// 初始Y方向為0（無方向）

	// --- 顏色設定 ---
	bgColor = BG_COLOR; // 設定背景色
	fgColor = FG_COLOR; // 設定前景色

	// --- 初始化隨機數種子（使用SysTick計數器獲取隨機初始值） ---
	// SysTick->VAL是系統計數器的當前值，每次啟動時都不同，可作為隨機種子
	seedCounter = SysTick->VAL;

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
		seedCounter++; // 持續遞增種子計數器，增加隨機性（供未來使用）

		// --- 掃描按鍵狀態 ---
		keyin = ScanKey();

		// --- 按鍵處理邏輯（僅在按鍵按下且與上次不同時執行，實現按鍵釋放檢測） ---
		// 這可以避免按鍵長按時重複觸發，只有按鍵從無到有（按下）時才執行
		if (keyin != 0 && keyin != last_key)
		{
			// 根據按下的按鍵執行對應動作
			switch (keyin)
			{
			case 4: // 按鍵4：向左移動（水平移動）
				if (!is_moving)
				{				   // 只有在未移動時才能啟動新移動（避免重複觸發）
					dirX = -1;	   // 設定X方向向左
					dirY = 0;	   // Y方向為0（僅水平移動）
					movX = 3;	   // 每次移動3像素（X方向）
					movY = 0;	   // Y方向移動距離為0
					is_moving = 1; // 標記為移動狀態
				}
				break;

			case 6: // 按鍵6：向右移動（水平移動）
				if (!is_moving)
				{
					dirX = 1;	   // 設定X方向向右
					dirY = 0;	   // Y方向為0（僅水平移動）
					movX = 3;	   // 每次移動3像素（X方向）
					movY = 0;	   // Y方向移動距離為0
					is_moving = 1; // 標記為移動狀態
				}
				break;

			case 3: // 按鍵3：右上45度方向移動（對角線移動）
				if (!is_moving)
				{
					dirX = 1;	   // 設定X方向向右
					dirY = -1;	   // 設定Y方向向上（螢幕座標：上為負）
					movX = 3;	   // 每次移動3像素（X方向）
					movY = 3;	   // 每次移動3像素（Y方向）
					is_moving = 1; // 標記為移動狀態
				}
				break;

			case 9: // 按鍵9：右下45度方向移動（對角線移動）
				if (!is_moving)
				{
					dirX = 1;	   // 設定X方向向右
					dirY = 1;	   // 設定Y方向向下（螢幕座標：下為正）
					movX = 3;	   // 每次移動3像素（X方向）
					movY = 3;	   // 每次移動3像素（Y方向）
					is_moving = 1; // 標記為移動狀態
				}
				break;

			case 1: // 按鍵1：左上45度方向移動（對角線移動）
				if (!is_moving)
				{
					dirX = -1;	   // 設定X方向向左
					dirY = -1;	   // 設定Y方向向上
					movX = 3;	   // 每次移動3像素（X方向）
					movY = 3;	   // 每次移動3像素（Y方向）
					is_moving = 1; // 標記為移動狀態
				}
				break;

			case 7: // 按鍵7：左下45度方向移動（對角線移動）
				if (!is_moving)
				{
					dirX = -1;	   // 設定X方向向左
					dirY = 1;	   // 設定Y方向向下
					movX = 3;	   // 每次移動3像素（X方向）
					movY = 3;	   // 每次移動3像素（Y方向）
					is_moving = 1; // 標記為移動狀態
				}
				break;

			case 5:			   // 按鍵5：停止移動（S鍵）
				is_moving = 0; // 標記為停止狀態
				movX = 0;	   // 清除X方向移動距離
				movY = 0;	   // 清除Y方向移動距離
				break;

			case 8: // 按鍵8：重新產生方塊（R鍵 - Random）
				// 只有在兩個方塊都消失時才能重新產生
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
					{
						for (j = block1_y - BLOCK_SIZE / 2; j <= block1_y + BLOCK_SIZE / 2; j++)
						{
							draw_Pixel(i, j, FG_COLOR, bgColor);
						}
					}

					// 重新繪製第二個方塊
					for (i = block2_x - BLOCK_SIZE / 2; i <= block2_x + BLOCK_SIZE / 2; i++)
					{
						for (j = block2_y - BLOCK_SIZE / 2; j <= block2_y + BLOCK_SIZE / 2; j++)
						{
							draw_Pixel(i, j, FG_COLOR, bgColor);
						}
					}

					// 重新繪製球體（保持當前位置）
					fgColor = FG_COLOR;
					draw_Circle(x, y, r, fgColor, bgColor);
				}
				break;
			}
		}

		last_key = keyin; // 記錄當前按鍵值，供下次比較使用（實現按鍵釋放檢測）

		// --- 球體移動處理邏輯（僅在移動狀態時執行） ---
		if (is_moving)
		{
			// 計算球體的下一個位置
			new_x = x + dirX * movX; // 新X座標 = 當前X + 方向 * 移動距離
			new_y = y + dirY * movY; // 新Y座標 = 當前Y + 方向 * 移動距離

			// --- 邊界碰撞檢測與反彈處理 ---
			// LCD尺寸為128x64，座標範圍：X(0-127), Y(0-63)
			bounced = 0; // 初始化反彈標誌為未反彈

			// 檢查X軸邊界（左右邊界）
			if (new_x <= r)
			{
				// 碰撞左邊界：球體左邊緣觸碰到螢幕左邊緣
				if (dirX != 0)
				{
					dirX = 1; // 反彈向右（改變X方向）
				}
				new_x = r;	 // 限制X座標在邊界位置（避免超出螢幕）
				bounced = 1; // 標記為已反彈
			}
			else if (new_x >= (127 - r))
			{
				// 碰撞右邊界：球體右邊緣觸碰到螢幕右邊緣（最大X為127）
				if (dirX != 0)
				{
					dirX = -1; // 反彈向左（改變X方向）
				}
				new_x = 127 - r; // 限制X座標在邊界位置
				bounced = 1;	 // 標記為已反彈
			}

			// 檢查Y軸邊界（上下邊界）
			if (new_y <= r)
			{
				// 碰撞上邊界：球體上邊緣觸碰到螢幕上邊緣
				if (dirY != 0)
				{
					dirY = 1; // 反彈向下（改變Y方向）
				}
				new_y = r;	 // 限制Y座標在邊界位置
				bounced = 1; // 標記為已反彈
			}
			else if (new_y >= (63 - r))
			{
				// 碰撞下邊界：球體下邊緣觸碰到螢幕下邊緣（最大Y為63）
				if (dirY != 0)
				{
					dirY = -1; // 反彈向上（改變Y方向）
				}
				new_y = 63 - r; // 限制Y座標在邊界位置
				bounced = 1;	// 標記為已反彈
			}

			// --- 方塊碰撞檢測與處理（分為兩步驟） ---
			overlap = 0; // 初始化碰撞標誌為未碰撞

			// 步驟1：檢查當前位置是否與方塊重疊（防止球體停駐在方塊內）
			if (block1_visible)
			{
				if (CheckOverlap(x, y, r, block1_x, block1_y, BLOCK_SIZE))
				{
					// 當前位置已重疊！立即清除方塊1
					for (i = block1_x - BLOCK_SIZE / 2; i <= block1_x + BLOCK_SIZE / 2; i++)
					{
						for (j = block1_y - BLOCK_SIZE / 2; j <= block1_y + BLOCK_SIZE / 2; j++)
						{
							draw_Pixel(i, j, bgColor, bgColor); // 用背景色繪製，清除方塊
						}
					}
					block1_visible = 0; // 標記方塊1為已消失
					overlap = 1;		// 標記為已碰撞
				}
			}

			if (block2_visible)
			{
				if (CheckOverlap(x, y, r, block2_x, block2_y, BLOCK_SIZE))
				{
					// 當前位置已重疊！立即清除方塊2
					for (i = block2_x - BLOCK_SIZE / 2; i <= block2_x + BLOCK_SIZE / 2; i++)
					{
						for (j = block2_y - BLOCK_SIZE / 2; j <= block2_y + BLOCK_SIZE / 2; j++)
						{
							draw_Pixel(i, j, bgColor, bgColor); // 用背景色繪製，清除方塊
						}
					}
					block2_visible = 0; // 標記方塊2為已消失
					overlap = 1;		// 標記為已碰撞
				}
			}

			// 步驟2：檢查下一位置是否與方塊重疊（預測碰撞）
			if (block1_visible)
			{
				if (CheckOverlap(new_x, new_y, r, block1_x, block1_y, BLOCK_SIZE))
				{
					// 將要發生碰撞！清除方塊1
					for (i = block1_x - BLOCK_SIZE / 2; i <= block1_x + BLOCK_SIZE / 2; i++)
					{
						for (j = block1_y - BLOCK_SIZE / 2; j <= block1_y + BLOCK_SIZE / 2; j++)
						{
							draw_Pixel(i, j, bgColor, bgColor); // 用背景色繪製，清除方塊
						}
					}
					block1_visible = 0; // 標記方塊1為已消失
					overlap = 1;		// 標記為已碰撞
				}
			}

			if (block2_visible)
			{
				if (CheckOverlap(new_x, new_y, r, block2_x, block2_y, BLOCK_SIZE))
				{
					// 將要發生碰撞！清除方塊2
					for (i = block2_x - BLOCK_SIZE / 2; i <= block2_x + BLOCK_SIZE / 2; i++)
					{
						for (j = block2_y - BLOCK_SIZE / 2; j <= block2_y + BLOCK_SIZE / 2; j++)
						{
							draw_Pixel(i, j, bgColor, bgColor); // 用背景色繪製，清除方塊
						}
					}
					block2_visible = 0; // 標記方塊2為已消失
					overlap = 1;		// 標記為已碰撞
				}
			}

			// --- 步驟3：檢查是否所有方塊都已消失（遊戲結束條件） ---
			if (!block1_visible && !block2_visible)
			{
				// 所有方塊都已消失，重置遊戲狀態
				// 重要：在移動之前先重置，避免球體繼續移動

				// 清除當前位置的球體
				fgColor = BG_COLOR; // 使用背景色清除
				draw_Circle(x, y, r, fgColor, bgColor);

				// 重置球體到初始位置
				x = X0; // 重置X座標到中央
				y = Y0; // 重置Y座標到底部

				// 停止移動並清除所有移動參數
				is_moving = 0; // 停止移動
				movX = 0;	   // 清除X移動距離
				movY = 0;	   // 清除Y移動距離
				dirX = 0;	   // 清除X方向
				dirY = 0;	   // 清除Y方向

				// 在初始位置重新繪製球體
				fgColor = FG_COLOR; // 使用前景色繪製
				draw_Circle(x, y, r, fgColor, bgColor);

				// 如果發生碰撞，觸發蜂鳴器（目前Buzz函數未實作）
				if (overlap)
				{
					Buzz(1);
				}

				// 短延遲後繼續，避免重複處理
				CLK_SysTickDelay(10000);
				continue; // 跳過後續繪製步驟，直接進入下一次迴圈
			}

			// --- 步驟4：正常移動處理 - 清除舊位置並繪製新位置 ---
			// 只有在所有檢查完成後才執行移動，確保不會在異常狀態下移動
			fgColor = BG_COLOR; // 使用背景色清除舊球體
			draw_Circle(x, y, r, fgColor, bgColor);

			// --- 步驟5：重新繪製仍然可見的方塊（避免球體移動時擦除方塊） ---
			// 這確保方塊在球體移動後仍然正確顯示
			if (block1_visible)
			{
				for (i = block1_x - BLOCK_SIZE / 2; i <= block1_x + BLOCK_SIZE / 2; i++)
				{
					for (j = block1_y - BLOCK_SIZE / 2; j <= block1_y + BLOCK_SIZE / 2; j++)
					{
						draw_Pixel(i, j, FG_COLOR, bgColor); // 重新繪製方塊1
					}
				}
			}

			if (block2_visible)
			{
				for (i = block2_x - BLOCK_SIZE / 2; i <= block2_x + BLOCK_SIZE / 2; i++)
				{
					for (j = block2_y - BLOCK_SIZE / 2; j <= block2_y + BLOCK_SIZE / 2; j++)
					{
						draw_Pixel(i, j, FG_COLOR, bgColor); // 重新繪製方塊2
					}
				}
			}

			// 更新球體位置到新位置
			x = new_x; // 更新X座標
			y = new_y; // 更新Y座標

			// 在新位置繪製球體
			fgColor = FG_COLOR; // 使用前景色繪製
			draw_Circle(x, y, r, fgColor, bgColor);

			// 如果發生反彈或碰撞方塊，觸發蜂鳴器（目前Buzz函數未實作）
			if (bounced || overlap)
			{
				Buzz(1);
			}

			// 動畫延遲（控制移動速度）
			CLK_SysTickDelay(50000); // 延遲50毫秒，控制球體移動的視覺速度
		}
		// 當球體停止時
		else
		{
			CLK_SysTickDelay(10000); // 短延遲10毫秒，降低CPU使用率
		}
	}
}
