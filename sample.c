
/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "timer/timer.h"
#include "game/game.h"
#include "render/render.h"
#include "RIT/RIT.h"
#include "button_EXINT/button.h"
#include "joystick/joystick.h"

#define SIMULATOR 1

#ifdef SIMULATOR

extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

extern int8_t time;
	
int main(void){
	int i, j;
	
 	SystemInit();  												/* System Initialization (i.e., PLL)  */
  LCD_Initialization();
	BUTTON_init();
	joystick_init();
	init_RIT(0x004C4B40);									/* RIT Initialization 50 msec       */
	init_timer(0, 0x017D7840);						/* 1s, per il timer di 20 secondi */
	init_timer(1, 0x047868C0);						/* 3s, per la comparsa a schermo di testi */
	enable_RIT();
	
	LCD_Clear(Blue);

	for(i=0; i<TITLE_WIDTH; i++)
		for(j=0; j<TITLE_HEIGHT; j++)
			LCD_SetPoint(0+i, 16+j, TITLE_PIXEL_DATA[i+TITLE_WIDTH*j]);
	
	GUIText_X_Center(66, (uint8_t *) "HOLD INT0 TO START", White, Blue);
	
	LCD_DrawRectFilled(0, MAX_SCREEN_Y-54, MAX_SCREEN_X, 32, Black);
	GUIText_X_Center(MAX_SCREEN_Y-54, (uint8_t *) "DI SANTO FRANCESCO", White, Black);
	GUIText_X_Center(MAX_SCREEN_Y-38, (uint8_t *) "S331336", White, Black);
	
	NVIC_DisableIRQ(EINT1_IRQn);
	NVIC_DisableIRQ(EINT2_IRQn);
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
	
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
