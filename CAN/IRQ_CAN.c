/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for for LPC17xx with MCB1700
 * Note(s): see also http://www.port.de/engl/canprod/sv_req_form.html
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <lpc17xx.h>                  /* LPC17xx definitions */
#include "CAN.h"                      /* LPC17xx CAN adaption layer */
#include "../GLCD/GLCD.h"
#include "../game/game.h"
#include "../render/render.h"
#include "../timer/timer.h"
#include "../menu/menu.h"
#include "../RIT/RIT.h"


extern uint8_t can_turn;
extern uint8_t npc_turn;
extern int8_t timer;
extern unsigned int move;
extern uint8_t last_turn;
extern unsigned int setMove(uint8_t player, uint8_t mode, uint8_t dir, uint8_t py, uint8_t px);
extern char buffer[4];
extern enum en_mod gameMode[2];
extern uint8_t CAN;
extern uint8_t isMenu;
extern uint8_t lock;

extern struct s_coords {
	uint8_t x;
	uint8_t y;
} pcoords[2];

extern uint8_t icr ; 										//icr and result must be global in order to work with both real and simulated landtiger.
extern uint32_t result;
extern CAN_msg       CAN_TxMsg;    /* CAN message for sending */
extern CAN_msg       CAN_RxMsg;    /* CAN message for receiving */                                

/*----------------------------------------------------------------------------
  CAN interrupt handler
 *----------------------------------------------------------------------------*/
void CAN_IRQHandler (void)  {
	
	uint8_t p;
  /* check CAN controller 1 */
	icr = 0;
  icr = (LPC_CAN1->ICR | icr) & 0xFF;               /* clear interrupts */
	
	if (icr & (1 << 0)) {                          	/* CAN Controller #1 meassage is received */
		CAN_rdMsg (1, &CAN_RxMsg);	                		/* Read the message */
    LPC_CAN1->CMR = (1 << 2);                    		/* Release receive buffer */
		
		if(gameMode[0]==twob_e && isMenu == 2 && gameMode[1] == menu_e) {

			if(CAN_RxMsg.data[0] == (uint8_t) human_e){
				gameMode[1] = human_e;
			}else if(CAN_RxMsg.data[0] == (uint8_t) npc_e){
				gameMode[1] = npc_e;
			}
			
			isMenu = 0;
			initGame();
		}
		else if(gameMode[0]==twob_e && isMenu == 0 && gameMode[1]!=menu_e && CAN_RxMsg.data[0]!=0xFF) {
			move = setMove(CAN_RxMsg.data[0],
										(CAN_RxMsg.data[1] & 0xF0) >> 4,
										(CAN_RxMsg.data[1] & 0xF),
										CAN_RxMsg.data[2],
										CAN_RxMsg.data[3]);
			
			npc_turn=1;
			can_turn=1;
			
			if(getMoveInfo(mode) == 0) {
				placePlayer(getMoveInfo(px), getMoveInfo(py), getMoveInfo(player));
			}else if(getMoveInfo(mode) == 1) {
				placeWall(getMoveInfo(px), getMoveInfo(py), getMoveInfo(dir), getMoveInfo(player));
			}
			
			enable_RIT();

		}
		else if (isMenu == 1){
			gameMode[0]=twob_e;
			isMenu = 2;
			if(CAN_RxMsg.data[0] == 0xFE){
				CAN = 1;
				CAN_TxMsg.data[0] = 0xFF;
				CAN_TxMsg.len = 1;
				CAN_TxMsg.format = STANDARD_FORMAT;
				CAN_TxMsg.type = DATA_FRAME;
				CAN_TxMsg.id = (CAN == 1) ? 2:1;
				CAN_wrMsg (CAN, &CAN_TxMsg);               /* transmit message */

				playerChoice();
			} else {
				disable_timer(2);
				playerChoice();
				enable_RIT();
			}
		} 
		else if (CAN_RxMsg.data[1] == 'W') {
			p = CAN_RxMsg.data[0];
			GUIText_X_Center(BOARD_Y+44, (uint8_t *) (p==0 ? "--- P1 VICTORY ---" : "--- P2 VICTORY ---"), p==0 ? Black : Red, White);
			disable_RIT();
			reset_timer(0);
		}
		
	}
	if (icr & (1 << 1)) {                         /* CAN Controller #1 meassage is transmitted */
		if(isMenu == 2){
			lock++; //usata per bloccare il "secondo" giocatore ad inizio partita
		}else{
			//nothing
		}	
	}
		
	/* check CAN controller 2 */
	icr = 0;
	icr = (LPC_CAN2->ICR | icr) & 0xFF;             /* clear interrupts */

	
	if (icr & (1 << 0)) {                          	/* CAN Controller #2 meassage is received */
		CAN_rdMsg (2, &CAN_RxMsg);	                		/* Read the message */
    LPC_CAN2->CMR = (1 << 2);                    		/* Release receive buffer */
		
		if(gameMode[0]==twob_e && isMenu == 2 && gameMode[1] == menu_e){

			if(CAN_RxMsg.data[0] == (uint8_t) human_e){
				gameMode[1] = human_e;
			}else if(CAN_RxMsg.data[0] == (uint8_t) npc_e){
				gameMode[1] = npc_e;
			}
			
			isMenu = 0;
			initGame();
		}
		else if(gameMode[0]==twob_e && isMenu == 0 && gameMode[1]!=menu_e && CAN_RxMsg.data[0]!=0xFF){
			move = setMove(CAN_RxMsg.data[0],
										(CAN_RxMsg.data[1] & 0xF0) >> 4,
										(CAN_RxMsg.data[1] & 0xF),
										CAN_RxMsg.data[2],
										CAN_RxMsg.data[3]);
		
			npc_turn=1;
			can_turn=1;
			
			if(getMoveInfo(mode) == 0) {
				placePlayer(getMoveInfo(px), getMoveInfo(py), getMoveInfo(player));
			}else if(getMoveInfo(mode) == 1) {
				placeWall(getMoveInfo(px), getMoveInfo(py), getMoveInfo(dir), getMoveInfo(player));
			}
			
			enable_RIT();
		}
		else if (isMenu == 1){
			gameMode[0]=twob_e;
			isMenu = 2;
			if(CAN_RxMsg.data[0] == 0xFE){
				CAN = 2;
				CAN_TxMsg.data[0] = 0xFF;
				CAN_TxMsg.len = 1;
				CAN_TxMsg.format = STANDARD_FORMAT;
				CAN_TxMsg.type = DATA_FRAME;
				CAN_TxMsg.id = 2;
				CAN_TxMsg.id = (CAN == 1) ? 2:1;
				CAN_wrMsg (CAN, &CAN_TxMsg);				/* transmit message */

				playerChoice();
			}else {
				disable_timer(2);
				playerChoice();
				enable_RIT();
			}
		}
		else if (CAN_RxMsg.data[1] == 'W') {
			p = CAN_RxMsg.data[0];
			GUIText_X_Center(BOARD_Y+44, (uint8_t *) (p==0 ? "--- P1 VICTORY ---" : "--- P2 VICTORY ---"), p==0 ? Black : Red, White);
			disable_RIT();
			reset_timer(0);
		}
		
	}
	if (icr & (1 << 1)) {                         /* CAN Controller #2 meassage is transmitted */
		if(isMenu == 2){
			lock++; //usata per bloccare il "secondo" giocatore ad inizio partita
		}else{
			//nothing
		}	
	}
}
