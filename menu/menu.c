#include "menu.h"
#include "string.h"
#include "../GLCD/GLCD.h" 
#include "../render/render.h"
#include "../timer/timer.h"
#include "../game/game.h"
#include "../RIT/RIT.h"
#include "../button_EXINT/button.h"
#include "../joystick/joystick.h"

extern enum en_mod gameMode[2];

volatile uint8_t isMenu;

void generateMenu(){
	uint8_t i, j;

	gameMode[0] = menu_e;
	gameMode[1] = menu_e;
	
	isMenu = 1;
	
	LCD_Clear(Blue);

	for(i=0; i<TITLE_WIDTH; i++)
		for(j=0; j<TITLE_HEIGHT; j++)
			LCD_SetPoint(0+i, 16+j, TITLE_PIXEL_DATA[i+TITLE_WIDTH*j]);
	
	LCD_DrawRectFilled(0, MAX_SCREEN_Y-54, MAX_SCREEN_X, 32, Black);
	GUIText_X_Center(MAX_SCREEN_Y-54, (uint8_t *) "DI SANTO FRANCESCO", White, Black);
	GUIText_X_Center(MAX_SCREEN_Y-38, (uint8_t *) "S331336", White, Black);
	
	boardChoice();
}

void boardChoice(){
	GUIText_X_Center(66, (uint8_t *) boards_text[0], White, Blue);
	
	LCD_DrawRectBorder((MAX_SCREEN_X-114)/2, MAX_SCREEN_Y/2 - 3 - 32, (MAX_SCREEN_X-12)/2, 32, Black);
	LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 - 3 - 31, (MAX_SCREEN_X-16)/2, 30, White);
	GUI_Text(GUIText_CenterCX((uint8_t *) boards_text[1]), MAX_SCREEN_Y/2 - 3 - 24, (uint8_t *) boards_text[1], Black, White);
	
	LCD_DrawRectBorder((MAX_SCREEN_X-114)/2, MAX_SCREEN_Y/2 + 3, (MAX_SCREEN_X-12)/2, 32, Black);
	LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 + 3 + 1, (MAX_SCREEN_X-16)/2, 30, White);
	GUI_Text(GUIText_CenterCX((uint8_t *) boards_text[2]), MAX_SCREEN_Y/2 + 3 + 8, (uint8_t *) boards_text[2], Black, White);
}

void playerChoice(){
	LCD_DrawRectFilled(0, 66, MAX_SCREEN_X, 16, Blue);
	GUIText_X_Center(66, gameMode[0]==oneb_e ? (uint8_t *) player_text[0] : (uint8_t *) player_text[1], White, Blue);
	
	LCD_DrawRectBorder((MAX_SCREEN_X-114)/2, MAX_SCREEN_Y/2 - 3 - 32, (MAX_SCREEN_X-12)/2, 32, Black);
	LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 - 3 - 31, (MAX_SCREEN_X-16)/2, 30, White);
	GUI_Text(GUIText_CenterCX((uint8_t *) player_text[2]), MAX_SCREEN_Y/2 - 3 - 24, (uint8_t *) player_text[2], Black, White);
	
	LCD_DrawRectBorder((MAX_SCREEN_X-114)/2, MAX_SCREEN_Y/2 + 3, (MAX_SCREEN_X-12)/2, 32, Black);
	LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 + 3 + 1, (MAX_SCREEN_X-16)/2, 30, White);
	GUI_Text(GUIText_CenterCX((uint8_t *) player_text[3]), MAX_SCREEN_Y/2 + 3 + 8, (uint8_t *) player_text[3], Black, White);
}
