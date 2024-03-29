/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../game/game.h"
#include "../render/render.h"
#include "../timer/timer.h"
#include "../GLCD/GLCD.h"
#include "../menu/menu.h"
#include "../CAN/CAN.h"

volatile uint8_t zero=0;
volatile uint8_t one=0;
volatile uint8_t two=0;

volatile int8_t oneb = -1;

extern unsigned int move;
extern uint8_t n_walls[2]; //conteggio muri
extern struct s_coords { //posizioni pi� recenti dei giocatori dentro la griglia di gioco
	uint8_t x;
	uint8_t y;
} pcoords[2];
extern char boardMat[13][13];
extern int8_t timer;
extern uint8_t last_turn;
extern enum en_mod gameMode[2];
extern uint8_t isMenu;
extern uint8_t CAN;
extern uint8_t lock;
extern uint8_t can_turn;

extern unsigned int setMove(uint8_t player, uint8_t mode, uint8_t dir, uint8_t py, uint8_t px);

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void RIT_IRQHandler (void)
{			
	static uint8_t select=0;	//25
	static uint8_t down=0;		//26
	static uint8_t left=0;		//27
	static uint8_t right=0;		//28
	static uint8_t up=0;			//29
	
	if(getMoveInfo(mode)==0){
			if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0 && (LPC_GPIO1->FIOPIN & (1<<27)) == 0){ //Bottom left
		down++;
		left++;
		
		if(down==1 && left==1){
			moveElement(dl_e);
			return;
		}
		
	} else {
		down=0;
		left=0;
	}

	if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0 && (LPC_GPIO1->FIOPIN & (1<<28)) == 0){ //Bottom right
		down++;
		right++;
		
		if (down>=1 && right>=1){
			moveElement(dr_e);
			return;
		}
		
	} else {
		down=0;
		right=0;
	}
	
	if ((LPC_GPIO1->FIOPIN & (1<<29)) == 0 && (LPC_GPIO1->FIOPIN & (1<<27)) == 0){ //Top left
		up++;
		left++;
		
		if (up==1 && left==1) {
			moveElement(ul_e);
			return;
		}
		
	} else {
		up=0;
		left=0;
	}
	
	if ((LPC_GPIO1->FIOPIN & (1<<29)) == 0 && (LPC_GPIO1->FIOPIN & (1<<28)) == 0){ //Top right
		up++;
		right++;
		
		if(up==1 && right==1) {
			moveElement(ur_e);
			return;
		}
		
	} else {
		up=0;
		right=0;
	}
	}

	
	if((LPC_GPIO1->FIOPIN & (1<<25)) == 0){	/* Joytick SELECT pressed */
		select++;
		switch(select){
			case 1:
				
			if(isMenu==0){
					if(getMoveInfo(mode)==0){
						if(getMoveInfo(px)==pcoords[getMoveInfo(player)].x && getMoveInfo(py)==pcoords[getMoveInfo(player)].y)
							break;
						
						placePlayer(getMoveInfo(px), getMoveInfo(py), getMoveInfo(player));
					}else{
						placeWall(getMoveInfo(px), getMoveInfo(py), getMoveInfo(dir), getMoveInfo(player));
					}
					
				}
				
				break;
			default:
				break;
		}
	}else select=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	/* Joytick DOWN pressed */
		down++;
		switch(down){
			case 1:
				if(isMenu==1){
					if(oneb!=0){
						oneb=0;
						LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 - 3 - 31, (MAX_SCREEN_X-16)/2, 30, White);
						GUI_Text(GUIText_CenterCX((uint8_t *) boards_text[1]), MAX_SCREEN_Y/2 - 3 - 24, (uint8_t *) boards_text[1], Black, White);
						LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 + 3 + 1, (MAX_SCREEN_X-16)/2, 30, Yellow);
						GUI_Text(GUIText_CenterCX((uint8_t *) boards_text[2]), MAX_SCREEN_Y/2 + 3 + 8, (uint8_t *) boards_text[2], Black, Yellow);
					}
				}else if (isMenu==2){
					
					if(gameMode[1] != npc_e){
						if(gameMode[1] != menu_e){
							LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 - 3 - 31, (MAX_SCREEN_X-16)/2, 30, White);
							GUI_Text(GUIText_CenterCX((uint8_t *) player_text[2]), MAX_SCREEN_Y/2 - 3 - 24, (uint8_t *) player_text[2], Black, White);
						}
						gameMode[1] = npc_e;
						LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 + 3 + 1, (MAX_SCREEN_X-16)/2, 30, Yellow);
						GUI_Text(GUIText_CenterCX((uint8_t *) player_text[3]), MAX_SCREEN_Y/2 + 3 + 8, (uint8_t *) player_text[3], Black, Yellow);

					}

				}else if (isMenu == 0){
					moveElement(down_e);
				}
				break;
			default:
				break;
		}
	}else down=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	/* Joytick LEFT pressed */
		left++;
		switch(left){
			case 1:
				moveElement(left_e);
				break;
			default:
				break;
		}
	}else	left=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){	/* Joytick RIGHT pressed */
		right++;
		switch(right){
			case 1:
				moveElement(right_e);				
				break;
			default:
				break;
		}
	}else right=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){	/* Joytick UP pressed */
		up++;
		switch(up){
			case 1:
				if(isMenu==1){
					if(oneb!=1){
						oneb=1;
						LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 + 3 + 1, (MAX_SCREEN_X-16)/2, 30, White);	
						GUI_Text(GUIText_CenterCX((uint8_t *) boards_text[2]), MAX_SCREEN_Y/2 + 3 + 8, (uint8_t *) boards_text[2], Black, White);
						LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 - 3 - 31, (MAX_SCREEN_X-16)/2, 30, Yellow);
						GUI_Text(GUIText_CenterCX((uint8_t *) boards_text[1]), MAX_SCREEN_Y/2 - 3 - 24, (uint8_t *) boards_text[1], Black, Yellow);
					}
				}else if (isMenu==2){
					if(gameMode[1] != human_e){
						if(gameMode[0] != menu_e){
							LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 + 3 + 1, (MAX_SCREEN_X-16)/2, 30, White);
							GUI_Text(GUIText_CenterCX((uint8_t *) player_text[3]), MAX_SCREEN_Y/2 + 3 + 8, (uint8_t *) player_text[3], Black, White);
						}
						gameMode[1] = human_e;
						LCD_DrawRectFilled((MAX_SCREEN_X-112)/2, MAX_SCREEN_Y/2 - 3 - 31, (MAX_SCREEN_X-16)/2, 30, Yellow);
						GUI_Text(GUIText_CenterCX((uint8_t *) player_text[2]), MAX_SCREEN_Y/2 - 3 - 24, (uint8_t *) player_text[2], Black, Yellow);
					}

					
				}else if (isMenu == 0){
					moveElement(up_e);
				}
				break;
			default:
				break;
		}
	}else	up=0;
	
	if(zero>=1){
		if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){
			switch(zero){
				case 1:
					if(oneb == 0){
						if(isMenu == 1){
							
							CAN = 1;
							CAN_TxMsg.data[0] = 0xFE;
							CAN_TxMsg.len = 1;
							CAN_TxMsg.format = STANDARD_FORMAT;
							CAN_TxMsg.type = DATA_FRAME;
							CAN_TxMsg.id = (CAN == 1)? 2:1;
							CAN_wrMsg (CAN, &CAN_TxMsg);
							GUIText_X_Center(210, (uint8_t *) "- CAN1 not working -", White, Blue);
							GUIText_X_Center(226, (uint8_t *) "Trying CAN2...", White, Blue);
							enable_timer(2);
							disable_RIT();
							
						}else if(isMenu == 2 && gameMode[0] == twob_e){
							//segnalo modalit� scelta
							if(gameMode[1] == human_e){
								
								CAN_TxMsg.data[0] = (uint8_t) human_e;
								CAN_TxMsg.len = 1;
								CAN_TxMsg.format = STANDARD_FORMAT;
								CAN_TxMsg.type = DATA_FRAME;
								CAN_TxMsg.id = (CAN == 1)? 2:1;
								CAN_wrMsg (CAN, &CAN_TxMsg);
								
							}else if (gameMode[1] == npc_e){
								
								CAN_TxMsg.data[0] = (uint8_t) npc_e;
								CAN_TxMsg.len = 1;
								CAN_TxMsg.format = STANDARD_FORMAT;
								CAN_TxMsg.type = DATA_FRAME;
								CAN_TxMsg.id = (CAN == 1)? 2:1;
								CAN_wrMsg (CAN, &CAN_TxMsg);;
								
							}
							isMenu=0;
							initGame();
						}
					}else if (isMenu == 1){
						
						gameMode[0]=oneb_e;
						isMenu = 2;
						playerChoice();
						
					}else if (isMenu == 2){
						isMenu=0;
						initGame();
						NVIC_EnableIRQ(EINT1_IRQn);
						NVIC_EnableIRQ(EINT2_IRQn);
					}

					break;
					
				case 20:
					if(gameMode[0]==oneb_e){
						initGame();
						NVIC_EnableIRQ(EINT1_IRQn);
						NVIC_EnableIRQ(EINT2_IRQn);
					}

					break;
				default:
					break;
			}
			zero++;
		} else {
			zero=0;			
			NVIC_EnableIRQ(EINT0_IRQn);
			LPC_PINCON->PINSEL4 |= (1 << 20);
		}
	}
	
	if(one>=1){
		if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){
			switch(one){
				case 2:
					if(getMoveInfo(mode)==0){
						if(n_walls[getMoveInfo(player)]>0){
							getPossibleMoves(Black);
							
							move = setMove(getMoveInfo(player), 1, 0, WALL_INIT_POS_Y, WALL_INIT_POS_X);
							
							writeWall(getWallPosition(WALL_INIT_POS_X),getWallPosition(WALL_INIT_POS_Y), getMoveInfo(dir), WALL_TMP);
						}else{
							GUIText_X_Center(BOARD_Y+52, (uint8_t *) "NO WALLS AVAILABLE", player==0? Black : Red, White);
							enable_timer(1);
						}
					}else{
						last_turn = 0xFF;
						writeWall(getWallPosition(getMoveInfo(px)),getWallPosition(getMoveInfo(py)), getMoveInfo(dir), White);
						
						move = setMove(getMoveInfo(player), 0, 0, pcoords[getMoveInfo(player)].y, pcoords[getMoveInfo(player)].x);
						
						getPossibleMoves(PLAYER_SEL);
					}
					break;
				default:
					break;
			}
			one++;
		} else {
			one=0;			
			NVIC_EnableIRQ(EINT1_IRQn);
			LPC_PINCON->PINSEL4 |= (1 << 22);
		}
	}
	
	if(two>=1){
		if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){
			switch(two){
				case 2:
					if (getMoveInfo(mode)!=0) {
						
						writeWall(getWallPosition(getMoveInfo(px)), getWallPosition(getMoveInfo(py)), getMoveInfo(dir), White);
						refreshWalls();
						
						move = setMove(getMoveInfo(player), getMoveInfo(mode), !getMoveInfo(dir) & 0x1, getMoveInfo(py), getMoveInfo(px));
						
						writeWall(getWallPosition(getMoveInfo(px)), getWallPosition(getMoveInfo(py)), getMoveInfo(dir), WALL_TMP);
					}
					break;
				default:
					break;
			}
			two++;
		} else {
			two=0;			
			NVIC_EnableIRQ(EINT2_IRQn);
			LPC_PINCON->PINSEL4 |= (1 << 24);
		}
	}
	

  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
