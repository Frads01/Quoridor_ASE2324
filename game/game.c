/* Game mechanics functions */

#include "game.h"
#include "stdio.h"
#include "../render/render.h"
#include "../GLCD/GLCD.h"
#include "../GLCD/AsciiLib.h"
#include "../timer/timer.h"
#include "../CAN/CAN.h"
#include "../npc/npc.h"
#include "stdlib.h"
#include "../RIT/RIT.h"

volatile unsigned int move;
volatile int8_t timer;
volatile uint8_t last_turn;
volatile uint8_t wallIndex;
volatile uint8_t can_turn;
volatile uint8_t npc_turn;

volatile struct s_coordsw {
	uint8_t x;
	uint8_t y;
	uint8_t rotate;
} walls[16];

volatile struct s_coords { //posizioni più recenti dei giocatori dentro la griglia di gioco, (0-6, 0-6)
	uint8_t x;
	uint8_t y;
} pcoords[2];

volatile uint8_t n_walls[2]; //conteggio muri
volatile char buffer[4]; //per stampa testo a schermo
volatile char boardMat[13][13]; //matrice di gioco
volatile enum en_mod gameMode[2];
volatile uint8_t graph[V][V];
volatile uint8_t CAN;
volatile enum en_dir selectedDir;
volatile uint8_t lock;

extern unsigned int setMove(uint8_t player, uint8_t mode, uint8_t dir, uint8_t py, uint8_t px);

/*
Inizializzazione gioco lato logico (matrice rappresentante la tavola, strutture dati contenenti info)
*/
void initGame(void){
	
	int i, j;
	
	initBG();

	wallIndex = 0;
	timer = 20;
	last_turn = 0xFF;
	can_turn = 0;
	npc_turn = 0;
	selectedDir = center_e;

	//GRAPH INIT (usato per NPC)
	for(i=0; i<V; i++){
		for(j=0; j<V; j++){
			if(j==i+1 || j==i-1 || j==i+7 || j==i-7){
				graph[i][j]=1;
			}else{
				graph[i][j]=0;
			}
		}
	}
	
  for (i = 0; i < 13; i++) 
    for (j = 0; j < 13; j++) 
      boardMat[i][j] = ' ';
	
	for (i = 0; i < PL_INIT_WALLS*2; i++) {
		walls[i].x = 0;
		walls[i].y = 0;
		walls[i].rotate = 0;
	}
	
	pcoords[0].x = PL0_INIT_POS_X;
	pcoords[0].y = PL0_INIT_POS_Y;
	n_walls[0] = PL_INIT_WALLS;

	pcoords[1].x = PL1_INIT_POS_X;
	pcoords[1].y = PL1_INIT_POS_Y;
	n_walls[1] = PL_INIT_WALLS;
	
	sprintf( (char *) buffer, "%.2d", n_walls[0]);
	GUI_Text(GUIText_CenterCX((uint8_t *) buffer)-80, BOARD_Y+22, (uint8_t *) buffer, Black, White);
	sprintf( (char *) buffer, "%.2d", n_walls[1]);
	GUI_Text(GUIText_CenterCX((uint8_t *) buffer)+80, BOARD_Y+22, (uint8_t *) buffer, Black, White);

	boardMat[pcoords[0].x*2][pcoords[0].y*2] = '0';
	boardMat[pcoords[1].x*2][pcoords[1].y*2] = '1';
	
	move = setMove(0, 0, 0, PL0_INIT_POS_Y, PL0_INIT_POS_X);
	initGameScreen();
	
	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_EnableIRQ(EINT2_IRQn);
	
	if(lock==1){
		disable_RIT();
		disable_timer(0);
	}else{
		enable_timer(0);
	}

}


void moveElement(enum en_dir direction){

	uint8_t m, p, d, x, y, pn;
	int8_t skip=0;
	
	m = getMoveInfo(mode);
	p = getMoveInfo(player);
	d = getMoveInfo(dir);
	x = getMoveInfo(px);
	y = getMoveInfo(py);		
	pn = (!p & 0x1);
	
	switch(direction){
		case down_e:
			
				if(m==0 && selectedDir != down_e){
					if((pcoords[p].x==pcoords[pn].x) && (pcoords[p].y+1==pcoords[pn].y)) {
						skip++;
					}
					
					if(pcoords[p].y+skip<N_CELLS-1){
						if(boardMat[pcoords[p].x*2][(pcoords[p].y+skip)*2+1]==' '
							&& boardMat[pcoords[p].x*2][pcoords[p].y*2+1]==' '){
							if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
						
							move = setMove(p,m,d,pcoords[p].y+1+skip,pcoords[p].x);
							writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
							selectedDir = down_e;
						}
					}
				}else if (m==1){
						if(y<N_CELLS-2){
							writeWall(getWallPosition(x),getWallPosition(y), d, White);
							refreshWalls();
							move = setMove(p, m, d, y+1, x);
							writeWall(getWallPosition(getMoveInfo(px)),getWallPosition(getMoveInfo(py)), d, WALL_TMP);
						}

				}
			break;
				
				
		case up_e:
			
				if(m==0 && selectedDir != up_e){
					
					if((pcoords[p].x==pcoords[pn].x) && (pcoords[p].y-1==pcoords[pn].y)){
						skip++;
					}
					
					if(pcoords[p].y-skip>0){
						if(boardMat[pcoords[p].x*2][(pcoords[p].y-skip)*2-1]==' '
							&& boardMat[pcoords[p].x*2][pcoords[p].y*2-1]==' '){
							if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);

							move = setMove(p,m,d,pcoords[p].y-1-skip,pcoords[p].x);
							writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
							selectedDir = up_e;
						}
					}
				}else if (m==1){
					if(y>0){
						writeWall(getWallPosition(x),getWallPosition(y), d, White);
						refreshWalls();
						move = setMove(p, m, d, y-1, x);
						writeWall(getWallPosition(getMoveInfo(px)),getWallPosition(getMoveInfo(py)), d, WALL_TMP);
					}
				}
			break;
		
		
		case left_e:
			
				if(m==0 && selectedDir != left_e){
					
					if((pcoords[p].y==pcoords[pn].y) && (pcoords[p].x-1==pcoords[pn].x)){
						skip++;
					}
					
					if(pcoords[p].x-skip>0){
						if(boardMat[(pcoords[p].x-skip)*2-1][pcoords[p].y*2]==' '
							&& boardMat[pcoords[p].x*2-1][pcoords[p].y*2]==' '){
							if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
														
							move = setMove(p,m,d,pcoords[p].y,pcoords[p].x-1-skip);
							writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
							selectedDir = left_e;
						}
					}
				}else if (m==1){
					if(x>0){
						writeWall(getWallPosition(x),getWallPosition(y), d, White);
						refreshWalls();
						move = setMove(p, m, d, y, x-1);
						writeWall(getWallPosition(getMoveInfo(px)),getWallPosition(getMoveInfo(py)), d, WALL_TMP);
					}
				}
			break;
		
		
		case right_e:
			
				if(m==0 && selectedDir != right_e){
					
					if((pcoords[p].y==pcoords[pn].y) && (pcoords[p].x+1==pcoords[pn].x)) {
						skip++;
					}					
					if(pcoords[p].x+skip<N_CELLS-1){
						if(boardMat[(pcoords[p].x+skip)*2+1][pcoords[p].y*2]==' '
							&& boardMat[pcoords[p].x*2+1][pcoords[p].y*2]==' '){
							if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
							move = setMove(p,m,d,pcoords[p].y,pcoords[p].x+1+skip);
							writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
							selectedDir = right_e;
						}
					}
				}else if (m==1){
					if(x<N_CELLS-2){
						writeWall(getWallPosition(x),getWallPosition(y), d, White);
						refreshWalls();
						move = setMove(p, m, d, y, x+1);
						writeWall(getWallPosition(getMoveInfo(px)),getWallPosition(getMoveInfo(py)), d, WALL_TMP);
					}
				}
			break;
				
				/* DIAGONAL MOVES */
		
		case ur_e:
			if(m==0 && selectedDir != ur_e){
				if((pcoords[p].x == pcoords[pn].x) && (pcoords[p].y-1 == pcoords[pn].y)
					&& !(pcoords[pn].x == N_CELLS-1 || boardMat[pcoords[pn].x*2+1][pcoords[pn].y*2] == 'X')){
						if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
						move = setMove(p,m,d,pcoords[p].y-1,pcoords[p].x+1);
						writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
						selectedDir = ur_e;
					}
					
				if((pcoords[p].x+1 == pcoords[pn].x) && (pcoords[p].y == pcoords[pn].y)
					&& !(pcoords[pn].y == 0 || boardMat[pcoords[pn].x*2][pcoords[pn].y*2-1] == 'X')){
						if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
						move = setMove(p,m,d,pcoords[p].y-1,pcoords[p].x+1);
						writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
						selectedDir = ur_e;
					}
					
			}
			break;
		
		case ul_e:
			if(m==0 && selectedDir != ul_e){
				if((pcoords[p].x == pcoords[pn].x) && (pcoords[p].y-1 == pcoords[pn].y)
					&& !(pcoords[pn].x == 0 || boardMat[pcoords[pn].x*2-1][pcoords[pn].y*2] == 'X')){
						if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
						move = setMove(p,m,d,pcoords[p].y-1,pcoords[p].x-1);
						writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
						selectedDir = ul_e;
					}
					
				if((pcoords[p].x-1 == pcoords[pn].x) && (pcoords[p].y == pcoords[pn].y)
					&& !(pcoords[pn].y == 0 || boardMat[pcoords[pn].x*2][pcoords[pn].y*2-1] == 'X')){
						if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
						move = setMove(p,m,d,pcoords[p].y-1,pcoords[p].x-1);
						writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
						selectedDir = ul_e;
					}
			}
			break;
		
		case dr_e:
			if(m==0 && selectedDir != dr_e){
				if((pcoords[p].x == pcoords[pn].x) && (pcoords[p].y+1 == pcoords[pn].y)
					&& !(pcoords[pn].x == N_CELLS-1 || boardMat[pcoords[pn].x*2+1][pcoords[pn].y*2] == 'X')){
						if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
						move = setMove(p,m,d,pcoords[p].y+1,pcoords[p].x+1);
						writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
						selectedDir = dr_e;
					}
					
				if((pcoords[p].x+1 == pcoords[pn].x) && (pcoords[p].y == pcoords[pn].y)
					&& !(pcoords[pn].y == N_CELLS-1 || boardMat[pcoords[pn].x*2][pcoords[pn].y*2+1] == 'X')){
						if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
						move = setMove(p,m,d,pcoords[p].y+1,pcoords[p].x+1);
						writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
						selectedDir = dr_e;
					}
			}
			break;
		
		case dl_e:
			if(m==0 && selectedDir != dl_e){
				if((pcoords[p].x == pcoords[pn].x) && (pcoords[p].y+1 == pcoords[pn].y)
					&& !(pcoords[pn].x == 0 || boardMat[pcoords[pn].x*2-1][pcoords[pn].y*2] == 'X')){
						if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
						move = setMove(p,m,d,pcoords[p].y+1,pcoords[p].x-1);
						writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
						selectedDir = dl_e;
					}
					
				if((pcoords[p].x-1 == pcoords[pn].x) && (pcoords[p].y == pcoords[pn].y)
					&& !(pcoords[pn].y == N_CELLS-1 || boardMat[pcoords[pn].x*2][pcoords[pn].y*2+1] == 'X')){
						if(last_turn == p) writePlayer(getPlayerPosition(x), getPlayerPosition(y), PLAYER_SEL);
							
						move = setMove(p,m,d,pcoords[p].y+1,pcoords[p].x-1);
						writePlayer(getPlayerPosition(getMoveInfo(px)), getPlayerPosition(getMoveInfo(py)), PLAYER_TMP);
						selectedDir = dl_e;
					}
			}
			break;
				
		default:
			break;
	}
	

	
	if(last_turn != p && (getMoveInfo(px) != pcoords[p].x || getMoveInfo(py) != pcoords[p].y)){
		last_turn = p;
	}
}

void placePlayer(uint16_t newX, uint16_t newY, uint8_t player){
	uint8_t flag = 0;

	disable_timer(0);
		
	if(getMoveInfo(dir)!=1){
		if(pcoords[player].x != 0 && boardMat[pcoords[player].x*2-1][pcoords[player].y*2]!='X'){
			graph[pcoords[player].x+pcoords[player].y*7][pcoords[player].x-1+pcoords[player].y*7] = 1;
			graph[pcoords[player].x-1+pcoords[player].y*7][pcoords[player].x+pcoords[player].y*7] = 1;
		}
		
		if(pcoords[player].x != N_CELLS-1 && boardMat[pcoords[player].x*2+1][pcoords[player].y*2]!='X'){
			graph[pcoords[player].x+pcoords[player].y*7][pcoords[player].x+1+pcoords[player].y*7] = 1;
			graph[pcoords[player].x+1+pcoords[player].y*7][pcoords[player].x+pcoords[player].y*7] = 1;
		}
			
		if(pcoords[player].y != 0 && boardMat[pcoords[player].x*2][pcoords[player].y*2-1]!='X'){
			graph[pcoords[player].x+pcoords[player].y*7][pcoords[player].x+(pcoords[player].y-1)*7] = 1;
			graph[pcoords[player].x+(pcoords[player].y-1)*7][pcoords[player].x+pcoords[player].y*7] = 1;
		}
		
		if(pcoords[player].y != N_CELLS-1 && boardMat[pcoords[player].x*2][pcoords[player].y+1]!='X'){
			graph[pcoords[player].x+pcoords[player].y*7][pcoords[player].x+(pcoords[player].y+1)*7] = 1;
			graph[pcoords[player].x+(pcoords[player].y+1)*7][pcoords[player].x+pcoords[player].y*7] = 1;
		}
		
		if(npc_turn != 1) getPossibleMoves(Black);
		writePlayer(getPlayerPosition(pcoords[player].x), getPlayerPosition(pcoords[player].y), Black);
		writePlayer(getPlayerPosition(newX), getPlayerPosition(newY), player==0 ? PLAYER1_COL : PLAYER2_COL);
			
		boardMat[pcoords[player].x*2][pcoords[player].y*2]=' ';
		boardMat[newX*2][newY*2]= (player==0 ? '0':'1');
			
		pcoords[player].x=newX;
		pcoords[player].y=newY;
		
		if(pcoords[player].x != 0){
			graph[pcoords[player].x+pcoords[player].y*7][pcoords[player].x-1+pcoords[player].y*7] = 0;
			graph[pcoords[player].x-1+pcoords[player].y*7][pcoords[player].x+pcoords[player].y*7] = 0;
		}
		
		if(pcoords[player].x != N_CELLS-1){
			graph[pcoords[player].x+pcoords[player].y*7][pcoords[player].x+1+pcoords[player].y*7] = 0;
			graph[pcoords[player].x+1+pcoords[player].y*7][pcoords[player].x+pcoords[player].y*7] = 0;
		}
			
		if(pcoords[player].y != 0){
			graph[pcoords[player].x+pcoords[player].y*7][pcoords[player].x+(pcoords[player].y-1)*7] = 0;
			graph[pcoords[player].x+(pcoords[player].y-1)*7][pcoords[player].x+pcoords[player].y*7] = 0;
		}
		
		if(pcoords[player].y != N_CELLS-1){
			graph[pcoords[player].x+pcoords[player].y*7][pcoords[player].x+(pcoords[player].y+1)*7] = 0;
			graph[pcoords[player].x+(pcoords[player].y+1)*7][pcoords[player].x+pcoords[player].y*7] = 0;
		}
			
		if ((player==0 && newY==0)||(player==1 && newY==6)){
			
			GUIText_X_Center(BOARD_Y+44, (uint8_t *) (player==0 ? "--- P1 VICTORY ---" : "--- P2 VICTORY ---"), player==0 ? Black : Red, White);
			if(gameMode[0]==oneb_e) GUIText_X_Center(BOARD_Y+60, (uint8_t *) "HOLD INT0 TO START", Black, White);
			
			if(gameMode[0]==twob_e){
				CAN_TxMsg.data[0] = player;
				CAN_TxMsg.data[1] = 'W';
				CAN_TxMsg.len = 2;
				CAN_TxMsg.format = STANDARD_FORMAT;
				CAN_TxMsg.type = DATA_FRAME;
				CAN_TxMsg.id = CAN==1 ? 2 : 1;
				CAN_wrMsg (CAN==1 ? 1 : 2, &CAN_TxMsg);
			}
			
			flag++;
		}
	}
	
	if(flag==0){
		switchPlayer();
	}else{
		disable_RIT();
		disable_timer(0);
		reset_timer(0);
	}
}

uint8_t placeWall(uint16_t px, uint16_t py, uint8_t rotate, uint8_t player){
	short int i, pxMat, pyMat;
	
	pxMat = px*2+1; //coordinate
	pyMat = py*2+1; //centro del muro
	
	disable_timer(0);
	
	switch(rotate){
		
		case 1:
			if (!(boardMat[pxMat-1][pyMat]==' ' && boardMat[pxMat][pyMat]==' ' && boardMat[pxMat+1][pyMat]==' ')) {
				if(npc_turn != 1 && can_turn != 1){
					GUIText_X_Center(BOARD_Y+52, (uint8_t *) "INVALID WALL PLACEMENT", player==0? Black : Red, White);
					enable_timer(0);
					enable_timer(1);
				}
				return 0;
			}
			
			for(i=-1; i<=1; i++)
				boardMat[pxMat+i][pyMat]='X';

		break;
			
		case 0:

			if (!(boardMat[pxMat][pyMat-1]==' ' && boardMat[pxMat][pyMat]==' ' && boardMat[pxMat][pyMat+1]==' ')) {
				if(npc_turn != 1 && can_turn != 1){
					GUIText_X_Center(BOARD_Y+52, (uint8_t *) "INVALID WALL PLACEMENT", player==0? Black : Red, White);
					enable_timer(0);
					enable_timer(1);
				}
				return 0;
			}
			for(i=-1; i<=1; i++)
				boardMat[pxMat][pyMat+i]='X';
			
		break;
			
	}
	
	if(checkFreePath()==0){
		if(npc_turn != 1 && can_turn != 1){
			GUIText_X_Center(BOARD_Y+52, (uint8_t *) "INVALID WALL PLACEMENT", player==0? Black : Red, White);
			enable_timer(0);
			enable_timer(1);
		}
		
		if(rotate==0){
			for(i=-1; i<=1; i++)
				boardMat[pxMat][pyMat+i]=' ';
		}else{
			for(i=-1; i<=1; i++)
				boardMat[pxMat+i][pyMat]=' ';
		}
		
		return 0;
	}
	
	walls[wallIndex].x = px;
	walls[wallIndex].y = py;
	walls[wallIndex].rotate = rotate;
	wallIndex++;
	
	writeWall(getWallPosition(px),getWallPosition(py), rotate, WALL_COL);
	n_walls[player]--;
	
	if(rotate==0){ //mettendo un muro, rimuovo un arco dal grafo
		
//									|						 |									 |						|
//									V						 V									 V						V
		graph[(((px*2+1)-1)+((py*2+1)-1)*7)/2][(((px*2+1)+1)+((py*2+1)-1)*7)/2]=0;
		graph[(((px*2+1)+1)+((py*2+1)-1)*7)/2][(((px*2+1)-1)+((py*2+1)-1)*7)/2]=0;
		
		graph[(((px*2+1)-1)+((py*2+1)+1)*7)/2][(((px*2+1)+1)+((py*2+1)+1)*7)/2]=0;
		graph[(((px*2+1)+1)+((py*2+1)+1)*7)/2][(((px*2+1)-1)+((py*2+1)+1)*7)/2]=0;
		
	}else{
		
		graph[(((px*2+1)-1)+((py*2+1)+1)*7)/2][(((px*2+1)-1)+((py*2+1)-1)*7)/2]=0;
		graph[(((px*2+1)-1)+((py*2+1)-1)*7)/2][(((px*2+1)-1)+((py*2+1)+1)*7)/2]=0;
		
		graph[(((px*2+1)+1)+((py*2+1)+1)*7)/2][(((px*2+1)+1)+((py*2+1)-1)*7)/2]=0;
		graph[(((px*2+1)+1)+((py*2+1)-1)*7)/2][(((px*2+1)+1)+((py*2+1)+1)*7)/2]=0;
		
	}
	
	sprintf( (char *) buffer, "%.2d", n_walls[0]);
	GUI_Text(GUIText_CenterCX((uint8_t *) buffer)-80, BOARD_Y+22, (uint8_t *) buffer, Black, White);
	sprintf( (char *) buffer, "%.2d", n_walls[1]);
	GUI_Text(GUIText_CenterCX((uint8_t *) buffer)+80, BOARD_Y+22, (uint8_t *) buffer, Black, White);
	
	switchPlayer();
	
	return 1;
}

uint8_t checkFreePath(void){
	uint8_t i, j, k, found=0;
	uint8_t visited[N_CELLS][N_CELLS];
	
	for(k=0; k<2; k++){
		
		for(i=0; i<N_CELLS; i++)
			for(j=0; j<N_CELLS; j++)
				visited[i][j] = 0;
		
  DFS(visited, pcoords[k].x, pcoords[k].y);
		
		for(i=0; i<N_CELLS; i++){
		//Player 0: parte da X=6, deve arrivare a X=0*6=0
		//Player 1: parte da X=0, deve arrivare a X=1*6=6
			if(visited[i][k*6]==1){
				found++;
				break;
			}
			
		}

	}
	if(found!=2) return 0;
	return 1;
}

void DFS (uint8_t visited[N_CELLS][N_CELLS], uint8_t px, uint8_t py){
	
	uint8_t i;
	int8_t h[4]={1,0,-1,0};
	int8_t v[4]={0,1,0,-1};
	
	visited[px][py] = 1;
	
		for(i=0; i<4; i++){
			
			if((px+h[i] >= 0 && px+h[i] <= N_CELLS-1) && (py+v[i] >= 0 && py+v[i] <= N_CELLS-1)){
				if(boardMat[px*2+h[i]][py*2+v[i]]!='X'){
					if (visited[px+h[i]][py+v[i]]==1) continue;
					DFS(visited, px+h[i], py+v[i]);
				}
				
			}
			
		}
}

uint8_t getMoveInfo(enum en_info info){
	switch(info){
		case player:
			return ((move & 0xFF000000) >> 24);
		case mode:
			return ((move & 0x00F00000) >> 20);
		case dir:
			return ((move & 0x000F0000) >> 16);
		case py:
			return ((move & 0x0000FF00) >> 8);
		case px:
			return (move & 0x000000FF);
		default:
			return 0xFF;
	}
}

void sendMove(void){
	
	CAN_TxMsg.data[0] = getMoveInfo(player);
	CAN_TxMsg.data[1] = (getMoveInfo(mode) << 4) | getMoveInfo(dir);
	CAN_TxMsg.data[2] = getMoveInfo(py);
	CAN_TxMsg.data[3] = getMoveInfo(px);
	CAN_TxMsg.len = 4;
	CAN_TxMsg.format = STANDARD_FORMAT;
	CAN_TxMsg.type = DATA_FRAME;
	CAN_TxMsg.id = CAN==1 ? 2 : 1;
	CAN_wrMsg (CAN==1 ? 1 : 2, &CAN_TxMsg);               /* transmit message */
	disable_RIT();
	disable_timer(0);
	
}

//DA SISTEMARE, MANCA SOLO QUESTO
void switchPlayer(void){
	uint8_t p, pn;
	
	p = getMoveInfo(player);
	pn = (!p & 0x1);
	
	if(npc_turn == 1){
		npc_turn = 0;
		return;
	}
	
	if(gameMode[0]==twob_e && can_turn == 0){
		can_turn=1;
		sendMove(); //CAN
		return;
	}
	
	if(gameMode[0]==twob_e && gameMode[1]==npc_e && npc_turn == 1){
		//npc_turn = 0;
		NPC_func();
		sendMove();
	}
	
	if(gameMode[0]==oneb_e && gameMode[1]==npc_e && npc_turn==0){
		last_turn = p;
		move = setMove(pn, 0, 0, pcoords[pn].y, pcoords[pn].x);
		
		npc_turn = 1;
		NPC_func();
	
		p = getMoveInfo(player);
		pn = (!p & 0x1);
	}
	
	can_turn = 0;
	last_turn = p;
	move = setMove(pn, 0, 0, pcoords[pn].y, pcoords[pn].x);
	getPossibleMoves(PLAYER_SEL);
	timer = 20;
	selectedDir = center_e;
	reset_timer(0);
	enable_timer(0);
}
