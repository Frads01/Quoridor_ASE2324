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


extern uint8_t can_turn;
extern int8_t timer;
extern unsigned int move;
extern uint8_t last_turn;
extern unsigned int setMove(uint8_t player, uint8_t mode, uint8_t dir, uint8_t py, uint8_t px);
extern char buffer[2];
extern enum en_mod gameMode[2];
extern uint8_t CAN;
extern uint8_t isMenu;

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
	
  /* check CAN controller 1 */
	icr = 0;
  icr = (LPC_CAN1->ICR | icr) & 0xFF;               /* clear interrupts */
	
  if (icr & (1 << 0)) {                          		/* CAN Controller #1 meassage is received */
		CAN_rdMsg (1, &CAN_RxMsg);	                		/* Read the message */
    LPC_CAN1->CMR = (1 << 2);                    		/* Release receive buffer */
		
		//data[0]==1 -> handshake asked
		//data[0]==2 -> handshake replied
		
		if(gameMode[0]==menu_e){
			gameMode[0]=twob_e;
			if(CAN_RxMsg.data[0]==1){
				CAN_TxMsg.data[0] = 2;
				CAN_TxMsg.len = 1;
				CAN_TxMsg.format = STANDARD_FORMAT;
				CAN_TxMsg.type = DATA_FRAME;
				CAN = 1;
				CAN_TxMsg.id = CAN==1 ? 2 : 1;
				CAN_wrMsg (CAN, &CAN_TxMsg);               /* transmit message */
			}
			isMenu = 2;
			playerChoice();
		}else if(gameMode[0]==twob_e && gameMode[1]==menu_e){
			isMenu = 0;
			gameMode[1]=npc_e;
			initGame();
		}else{
			move = setMove(CAN_RxMsg.data[0],
										(CAN_RxMsg.data[1] & 0xF0) >> 4,
										(CAN_RxMsg.data[1] & 0xF),
										CAN_RxMsg.data[2],
										CAN_RxMsg.data[3]);
			
			if(getMoveInfo(mode) == 0) {
				placePlayer(getMoveInfo(px), getMoveInfo(py), getMoveInfo(player));
			}else if(getMoveInfo(mode) == 1) {
				placeWall(getMoveInfo(px), getMoveInfo(py), getMoveInfo(dir), getMoveInfo(player));
			}
			
			can_turn = 1;
			switchPlayer();
		}
		

		
  }
	if (icr & (1 << 1)) {                         /* CAN Controller #1 meassage is transmitted */
		// do nothing
	}
		
	/* check CAN controller 2 */
	icr = 0;
	icr = (LPC_CAN2->ICR | icr) & 0xFF;             /* clear interrupts */

	
	if (icr & (1 << 0)) {                          	/* CAN Controller #2 meassage is received */
		CAN_rdMsg (2, &CAN_RxMsg);	                		/* Read the message */
    LPC_CAN2->CMR = (1 << 2);                    		/* Release receive buffer */
		
		if(gameMode[0]==menu_e){
			gameMode[0]=twob_e;
			if(CAN_RxMsg.data[0]==1){
				CAN_TxMsg.data[0] = 2;
				CAN_TxMsg.len = 1;
				CAN_TxMsg.format = STANDARD_FORMAT;
				CAN_TxMsg.type = DATA_FRAME;
				CAN = 2;
				CAN_TxMsg.id = CAN==1 ? 2 : 1;
				CAN_wrMsg (CAN, &CAN_TxMsg);               /* transmit message */
			}
			isMenu = 2;
			playerChoice();
		}else if(gameMode[0]==twob_e && gameMode[1]==menu_e){
			isMenu = 0;
			gameMode[1]=npc_e;
			initGame();
		}else{
			move = setMove(CAN_RxMsg.data[0],
										(CAN_RxMsg.data[1] & 0xF0) >> 4,
										(CAN_RxMsg.data[1] & 0xF),
										CAN_RxMsg.data[2],
										CAN_RxMsg.data[3]);
		
			if(getMoveInfo(mode) == 0) {
				placePlayer(getMoveInfo(px), getMoveInfo(py), getMoveInfo(player));
			}else if(getMoveInfo(mode) == 1) {
				placeWall(getMoveInfo(px), getMoveInfo(py), getMoveInfo(dir), getMoveInfo(player));
			}
			can_turn = 1;
			switchPlayer();
		}
		
	}
	if (icr & (1 << 1)) {                         /* CAN Controller #2 meassage is transmitted */
		// do nothing
	}
}
