#include "button.h"
#include "lpc17xx.h"
#include "../RIT/RIT.h"
#include "../timer/timer.h"
#include "../render/render.h"
#include "../game/game.h"
#include "../menu/menu.h"

extern uint8_t zero;
extern uint8_t one;
extern uint8_t two;

extern uint8_t isMenu;
extern enum en_mod gameMode[2];

void EINT0_IRQHandler (void)	  	/* INT0														 */
{		

	if(isMenu == 1 && gameMode[0] != menu_e){
		handshake();
		isMenu++;
		playerChoice();
	}else if(isMenu == 2 && gameMode[1] != menu_e){
		isMenu=0;
		initGame();
	}else if(isMenu == 0){
		enable_RIT();
		NVIC_DisableIRQ(EINT0_IRQn);
		LPC_PINCON->PINSEL4    &= ~(1 << 20);     /* GPIO pin selection */
		zero=1;
	}
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	NVIC_DisableIRQ(EINT1_IRQn);
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     
	one=1;
	LPC_SC->EXTINT &= (1 << 1);     
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	NVIC_DisableIRQ(EINT2_IRQn);
	LPC_PINCON->PINSEL4    &= ~(1 << 24);     
	two=1;
  LPC_SC->EXTINT &= (1 << 2);     
}


