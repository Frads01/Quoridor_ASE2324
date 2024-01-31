/* NPC Mechanics */

#include "npc.h"
#include "stdlib.h"
#include "time.h"
#include "../game/game.h"
#include "../render/render.h"
#include "../GLCD/GLCD.h"
#include "../GLCD/AsciiLib.h"
#include "../timer/timer.h"
#include "../CAN/CAN.h"

extern unsigned int move;
extern int8_t timer;
extern uint8_t last_turn;
extern uint8_t wallIndex;
extern uint8_t can_turn;

extern struct s_coordsw {
	uint8_t x;
	uint8_t y;
	uint8_t rotate;
} walls[16];

extern struct s_coords { //posizioni più recenti dei giocatori dentro la griglia di gioco, (0-6, 0-6)
	uint8_t x;
	uint8_t y;
} pcoords[2];

extern uint8_t n_walls[2]; //conteggio muri
extern char buffer[4]; //per stampa testo a schermo
extern char boardMat[13][13]; //matrice di gioco
extern enum en_mod gameMode[2];
extern uint8_t graph[V][V];

extern unsigned int setMove(uint8_t player, uint8_t mode, uint8_t dir, uint8_t py, uint8_t px);

static uint8_t mindistance, bestMove, start;
static uint8_t minDist, distance[V], pred[V];

static uint8_t moveCount, lastMove=0xFF;

void NPC_func(void){
	
	uint8_t m;
	
	srand(LPC_RIT->RICOUNTER);
	m = (rand() % 2);
	
	//l'npc può muovere la pedina/mettere un muro massimo 2 volte di fila
	if(moveCount == 3) {
		m = (m == 0 ? 1 : 0);
		moveCount = 1;
		lastMove = m;
	} else if(lastMove != m) {
		moveCount = 1;
		lastMove = m;
	} else moveCount++;
	
	if (m==0) generateMove();
	if (m==1 && n_walls[getMoveInfo(player)]>0) generateWall();

}

void generateWall(void){
	uint8_t cx, cy, crot;
	
	do {
		cx = rand() % 6;
		cy = rand() % 6;
		crot = rand() % 2;
		// piazza il muro solo sopra il giocatore 0
	} while (placeWall(cx, cy, crot, getMoveInfo(player))==0);

	move = setMove(getMoveInfo(player), 1, crot, cy, cx);
	
	return;
}

void generateMove(void){

	uint8_t p = getMoveInfo(player);
	uint8_t m = getMoveInfo(mode);
	uint8_t d = getMoveInfo(dir);
	uint8_t x = getMoveInfo(px);
	uint8_t y = getMoveInfo(py);
	
	uint8_t i, side, skip, doSearch;
	enum en_dir choice;
	
	minDist = 0xFF;
	
	for(choice = up_e; choice <= dl_e; choice++){
		skip = 0;
		doSearch = 0;
		
		switch(choice){
			
			case up_e:
				if((pcoords[p].x==pcoords[(!p & 0x1)].x) && (pcoords[p].y-1==pcoords[(!p & 0x1)].y)) {
					skip++;
				}
				
				if(pcoords[p].y-skip>0){
					if(boardMat[pcoords[p].x*2][(pcoords[p].y-skip)*2-1]==' '
							&& boardMat[pcoords[p].x*2][pcoords[p].y*2-1]==' '){
						start = (pcoords[p].x)+(pcoords[p].y-skip-1)*7;
						doSearch++;
					}
				}
				break;
				
			case down_e:
				if((pcoords[p].x==pcoords[(!p & 0x1)].x) && (pcoords[p].y+1==pcoords[(!p & 0x1)].y)) {
					skip++;
				}
				
				if(pcoords[p].y+skip<N_CELLS-1){
					if(boardMat[pcoords[p].x*2][(pcoords[p].y+skip)*2+1]==' '
							&& boardMat[pcoords[p].x*2][pcoords[p].y*2+1]==' '){
						start = (pcoords[p].x)+(pcoords[p].y+skip+1)*7;
						doSearch++;
					}
				}
				break;
			
			case left_e:
				if((pcoords[p].y==pcoords[(!p & 0x1)].y) && (pcoords[p].x-1==pcoords[(!p & 0x1)].x)){
					skip++;
				}
				
				if(pcoords[p].x-skip>0){
					if(boardMat[(pcoords[p].x-skip)*2-1][pcoords[p].y*2]==' '
							&& boardMat[pcoords[p].x*2-1][pcoords[p].y*2]==' '){
						start = (pcoords[p].x-1-skip)+(pcoords[p].y)*7;	
						doSearch++;
					}
				}
				break;
			
			case right_e:
				if((pcoords[p].y==pcoords[(!p & 0x1)].y) && (pcoords[p].x+1==pcoords[(!p & 0x1)].x)){
					skip++;
				}
				
				if(pcoords[p].x+skip<N_CELLS-1){
					if(boardMat[(pcoords[p].x+skip)*2+1][pcoords[p].y*2]==' '
							&& boardMat[pcoords[p].x*2+1][pcoords[p].y*2]==' '){
						start = (pcoords[p].x+1+skip)+(pcoords[p].y)*7;
						doSearch++;							
					}
				}
				break;
									
			// DIAGONAL MOVES
			
			case ur_e: //UP RIGHT
				if((pcoords[p].x == pcoords[(!p & 0x1)].x) && (pcoords[p].y-1 == pcoords[(!p & 0x1)].y)
						&& !(pcoords[(!p & 0x1)].x == N_CELLS-1 || boardMat[pcoords[(!p & 0x1)].x*2+1][pcoords[(!p & 0x1)].y*2] == 'X')
						&& (pcoords[(!p & 0x1)].y == 0 || boardMat[pcoords[(!p & 0x1)].x*2][pcoords[(!p & 0x1)].y*2-1] == 'X')){
					start = (pcoords[p].x+1)+(pcoords[p].y-1)*7;
					doSearch++;		
				}
					
				if((pcoords[p].x+1 == pcoords[(!p & 0x1)].x) && (pcoords[p].y == pcoords[(!p & 0x1)].y)
					&& !(pcoords[(!p & 0x1)].y == 0 || boardMat[pcoords[(!p & 0x1)].x*2][pcoords[(!p & 0x1)].y*2-1] == 'X')
					&& (pcoords[(!p & 0x1)].x == N_CELLS-1 || boardMat[pcoords[(!p & 0x1)].x*2+1][pcoords[(!p & 0x1)].y*2] == 'X')){
					start = (pcoords[p].x+1)+(pcoords[p].y-1)*7;
					doSearch++;		
				}
				break;
			
			case ul_e: //UP LEFT
				if((pcoords[p].x == pcoords[(!p & 0x1)].x) && (pcoords[p].y-1 == pcoords[(!p & 0x1)].y)
					&& !(pcoords[(!p & 0x1)].x == 0 || boardMat[pcoords[(!p & 0x1)].x*2-1][pcoords[(!p & 0x1)].y*2] == 'X')
					&& (pcoords[(!p & 0x1)].y == 0 || boardMat[pcoords[(!p & 0x1)].x*2][pcoords[(!p & 0x1)].y*2-1] == 'X')){
					start = (pcoords[p].x-1)+(pcoords[p].y-1)*7;
					doSearch++;		
				}
					
				if((pcoords[p].x-1 == pcoords[(!p & 0x1)].x) && (pcoords[p].y == pcoords[(!p & 0x1)].y)
					&& !(pcoords[(!p & 0x1)].y == 0 || boardMat[pcoords[(!p & 0x1)].x*2][pcoords[(!p & 0x1)].y*2-1] == 'X')
					&& (pcoords[(!p & 0x1)].x == 0 || boardMat[pcoords[(!p & 0x1)].x*2-1][pcoords[(!p & 0x1)].y*2] == 'X')){
					start = (pcoords[p].x-1)+(pcoords[p].y-1)*7;
					doSearch++;		
				}
				break;
			
			case dr_e: //DOWN RIGHT
				if((pcoords[p].x == pcoords[(!p & 0x1)].x) && (pcoords[p].y+1 == pcoords[(!p & 0x1)].y)
					&& !(pcoords[(!p & 0x1)].x == N_CELLS-1 || boardMat[pcoords[(!p & 0x1)].x*2+1][pcoords[(!p & 0x1)].y*2] == 'X')
					&& (pcoords[(!p & 0x1)].y == N_CELLS-1 || boardMat[pcoords[(!p & 0x1)].x*2][pcoords[(!p & 0x1)].y*2+1] == 'X')){
					start = (pcoords[p].x+1)+(pcoords[p].y+1)*7;
					doSearch++;		
				}
					
				if((pcoords[p].x+1 == pcoords[(!p & 0x1)].x) && (pcoords[p].y == pcoords[(!p & 0x1)].y)
					&& !(pcoords[(!p & 0x1)].y == N_CELLS-1 || boardMat[pcoords[(!p & 0x1)].x*2][pcoords[(!p & 0x1)].y*2+1] == 'X')
					&& (pcoords[(!p & 0x1)].x == N_CELLS-1 || boardMat[pcoords[(!p & 0x1)].x*2+1][pcoords[(!p & 0x1)].y*2] == 'X')){
					start = (pcoords[p].x+1)+(pcoords[p].y+1)*7;
					doSearch++;		
				}
				break;
			
			case dl_e: //DOWN LEFT
				if((pcoords[p].x == pcoords[(!p & 0x1)].x) && (pcoords[p].y+1 == pcoords[(!p & 0x1)].y)
					&& !(pcoords[(!p & 0x1)].x == 0 || boardMat[pcoords[(!p & 0x1)].x*2-1][pcoords[(!p & 0x1)].y*2] == 'X')
					&& (pcoords[(!p & 0x1)].y == N_CELLS-1 || boardMat[pcoords[(!p & 0x1)].x*2][pcoords[(!p & 0x1)].y*2+1] == 'X')){
					start = (pcoords[p].x-1)+(pcoords[p].y+1)*7;
					doSearch++;		
				}
					
				if((pcoords[p].x-1 == pcoords[(!p & 0x1)].x) && (pcoords[p].y == pcoords[(!p & 0x1)].y)
					&& !(pcoords[(!p & 0x1)].y == N_CELLS-1 || boardMat[pcoords[(!p & 0x1)].x*2][pcoords[(!p & 0x1)].y*2+1] == 'X')
					&& (pcoords[(!p & 0x1)].x == 0 || boardMat[pcoords[(!p & 0x1)].x*2-1][pcoords[(!p & 0x1)].y*2] == 'X')){
					start = (pcoords[p].x-1)+(pcoords[p].y+1)*7;
					doSearch++;		
				}
				break;
			
			
			default:
				break;
		}
		
		if(doSearch == 0) continue;
		
		dijkstra(); //CERCO CAMMINO MINIMO
	
		side = p==0 ? 0 : (N_CELLS)*(N_CELLS-1);
	
		for(i=0; i<N_CELLS; i++){
			if(minDist > distance[side+i]) {
				minDist = distance[side+i];
				bestMove = start;
			}
		}
	}

	x = bestMove % 7;
	y = (bestMove - bestMove % 7) / 7;
	
	placePlayer(x, y, getMoveInfo(player));
	move = setMove(p, m, d, y, x);
	
	return;
}

void dijkstra(void){
	uint8_t cost[V][V], visited[V], count, nextnode, i, j;
	
  for (i = 0; i < V; i++)
    for (j = 0; j < V; j++)
      if (graph[i][j] == 0)
        cost[i][j] = 0xFF;
      else
        cost[i][j] = graph[i][j];

  for (i = 0; i < V; i++) {
    distance[i] = cost[start][i];
    pred[i] = start;
    visited[i] = 0;
  }

  distance[start] = 0;
  visited[start] = 1;
  count = 1;

  while (count < V - 1) {
    mindistance = 0xFF;

    for (i = 0; i < V; i++)
      if (distance[i] < mindistance && !visited[i]) {
        mindistance = distance[i];
        nextnode = i;
      }

    visited[nextnode] = 1;
    for (i = 0; i < V; i++)
      if (!visited[i])
        if (mindistance + cost[nextnode][i] < distance[i]) {
          distance[i] = mindistance + cost[nextnode][i];
          pred[i] = nextnode;
        }
    count++;
  }
	
	return;
}
