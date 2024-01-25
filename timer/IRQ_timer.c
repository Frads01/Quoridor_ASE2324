/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "stdio.h"
#include "lpc17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 
#include "../game/game.h"
#include "../render/render.h"

extern uint8_t n_walls[2]; //conteggio muri

extern struct s_coords {
	uint8_t x;
	uint8_t y;
} pcoords[2];

extern int8_t timer;
extern unsigned int move;
extern uint8_t last_turn;
extern unsigned int setMove(uint8_t player, uint8_t mode, uint8_t dir, uint8_t py, uint8_t px);
extern char buffer[2];
extern enum en_mod gameMode[2];

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void TIMER0_IRQHandler (void)
{
	sprintf( (char *) buffer, "%.2d", timer);
	GUIText_X_Center(BOARD_Y+22, (uint8_t *) buffer, getMoveInfo(player)==0 ? Black: Red, White);

	timer--;

	if(timer<0){
		disable_timer(0);

		if(getMoveInfo(mode)==0)
			getPossibleMoves(Black);
		else if(getMoveInfo(mode)==1)
			writeWall(getWallPosition(getMoveInfo(px)), getWallPosition(getMoveInfo(py)), getMoveInfo(dir), White);
			
		switchPlayer();
	}
	
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
	LCD_DrawRectFilled(0, BOARD_Y+42, MAX_SCREEN_X, 38, White);
	disable_timer(1);
	reset_timer(1);
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
