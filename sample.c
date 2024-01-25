
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
#include "CAN/CAN.h"
#include "menu/menu.h"

#define SIMULATOR 1

#ifdef SIMULATOR

extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

extern int8_t timer;
extern enum en_mod gameMode[2];
extern uint8_t CAN;
	
int main(void){	
 	SystemInit();  												/* System Initialization (i.e., PLL)  */
  LCD_Initialization();
	BUTTON_init();
	joystick_init();
	CAN_Init();
	init_RIT(0x004C4B40);									/* RIT Initialization 50 msec       */
	init_timer(0, 0x017D7840);						/* 1s, per il timer di 20 secondi */
	init_timer(1, 0x047868C0);						/* 3s, per la comparsa a schermo di testi */
	
	enable_RIT();
	
	generateMenu();
	
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
