/* Render file */

#include "render.h"
#include "../GLCD/GLCD.h"
#include "../GLCD/AsciiLib.h"
#include "../game/game.h"

/* VARIABILI CONDIVISE */

extern int8_t timer;
extern unsigned int move;
extern uint8_t wallIndex;
extern uint8_t lock;

extern struct s_coordsw {
	uint8_t x;
	uint8_t y;
	uint8_t rotate;
} walls[16];

extern uint8_t n_walls[2]; //conteggio muri

extern struct s_coords {
	uint8_t x;
	uint8_t y;
} pcoords[2];

extern char boardMat[13][13];

/* FUNZIONI */

void initBG(void){
	LCD_Clear(White);
	writeBoard();
	writeStatsSection();
}

void initGameScreen(void){
	writePlayer(getPlayerPosition(pcoords[0].x), getPlayerPosition(pcoords[0].y), PLAYER1_COL);
	writePlayer(getPlayerPosition(pcoords[1].x), getPlayerPosition(pcoords[1].y), PLAYER2_COL);
	
	if(lock!=1){getPossibleMoves(PLAYER_SEL);}
}

void refreshWalls(void){
	uint8_t i;
	for(i=0; i<wallIndex; i++){
		writeWall(getWallPosition(walls[i].x), getWallPosition(walls[i].y), walls[i].rotate, WALL_COL);
	}
}

void writeBoard(void){
	int i, j;
	uint16_t px, py;

	py=4;

	for(i=0; i<N_CELLS; i++){
		px=4;
		for(j=0; j<N_CELLS; j++){
			LCD_DrawRectFilled(px, py, CELL_DIM, CELL_DIM, Black);
			px += CELL_DIM +6;
		}
		py += CELL_DIM +6;
	}
}

void writePlayer(uint16_t px, uint16_t py, uint16_t color){
	
	int i, j;
	
	switch(color){
		case PLAYER1_COL:
			for(i=0; i<PLAYER_SIZE; i++){
				for(j=0; j<PLAYER_SIZE; j++){
					LCD_SetPoint(px+i, py+j, P1_PIXEL_DATA[i+PLAYER_SIZE*j]);
				}
			}
			
			break;
		case PLAYER2_COL:
			for(i=0; i<PLAYER_SIZE; i++){
				for(j=0; j<PLAYER_SIZE; j++){
					LCD_SetPoint(px+i, py+j, P2_PIXEL_DATA[i+PLAYER_SIZE*j]);
				}
			}
			break;
		default:
			LCD_DrawRectFilled(px, py, PLAYER_SIZE, PLAYER_SIZE, color);
			break;
	}

	

}

uint16_t getWallPosition(int num_groove){
	if ( num_groove > 5 ) num_groove = 5;
	if ( num_groove < 0 ) num_groove = 0;
	return (CELL_DIM+5)*(num_groove+1)+num_groove;
}

void writeWall(uint16_t px, uint16_t py, uint8_t rotate, uint16_t color){
	if(rotate){
		LCD_DrawRectFilled(px-CELL_DIM, py, WALL_X, WALL_Y, color);
	}else{
		LCD_DrawRectFilled(px, py-CELL_DIM, WALL_Y, WALL_X, color);
	}

}

void writeStatsSection(void){
	int i;
	uint16_t px=2, py=2;

	GUI_Text(GUIText_CenterCX((uint8_t *)"P1 WALLS")-80, BOARD_Y+4, (uint8_t *) "P1 WALLS", Black, White);
	GUIText_X_Center(BOARD_Y+4, (uint8_t *) "TIME", Black, White);
	GUI_Text(GUIText_CenterCX((uint8_t *)"P2 WALLS")+80, BOARD_Y+4, (uint8_t *) "P2 WALLS", Black, White);

	for(i=0; i<3; i++) {
		LCD_DrawRectBorder(px, BOARD_X+py, STAT_WIND_X, STAT_WIND_Y, Black);
		px+=STAT_WIND_X+4;
	}
}

uint16_t getPlayerPosition(int num_cell){
	if ( num_cell > 7 ) num_cell = 5;
	if ( num_cell < 0 ) num_cell = 0;
	return 6+(PLAYER_SIZE+4+6)*(num_cell);
}

void getPossibleMoves(uint16_t color){
	uint8_t px, py;
	int8_t skip_lx=0, skip_rx=0, skip_dy=0, skip_uy=0;
	uint8_t turn = getMoveInfo(player);
	
	px = pcoords[turn].x;
	py = pcoords[turn].y;
	
	if(py==pcoords[(!turn & 0x1)].y){
		
		if(px+1==pcoords[(!turn & 0x1)].x)
			skip_rx++;
		if(px-1==pcoords[(!turn & 0x1)].x)
			skip_lx--;
		
	}else if(px==pcoords[(!turn & 0x1)].x){
		
		if(py+1==pcoords[(!turn & 0x1)].y) 
			skip_dy++;
		if(py-1==pcoords[(!turn & 0x1)].y)
			skip_uy--;
		
	}
		
	if(px+skip_rx != N_CELLS-1 && boardMat[px*2+1][py*2] == ' ' && boardMat[(px+skip_rx)*2+1][py*2] == ' ') {			//RIGHT
		writePlayer(getPlayerPosition(px+1+skip_rx), getPlayerPosition(py), color);
	}else if(boardMat[px*2+1][py*2] == ' ' && (px+1 == N_CELLS-1 || boardMat[(px+1)*2+1][py*2] == 'X')){
		if(py+skip_dy != N_CELLS-1 && boardMat[(px+skip_rx)*2][py*2+1] == ' ') 		//DOWN
			writePlayer(getPlayerPosition(px+skip_rx), getPlayerPosition(py+1), color);
		if(py+skip_uy != 0 && boardMat[(px+skip_rx)*2][py*2-1] == ' ') 						//UP
			writePlayer(getPlayerPosition(px+skip_rx), getPlayerPosition(py-1), color);
	}

	if(px+skip_lx != 0 && boardMat[px*2-1][py*2] == ' ' && boardMat[(px+skip_lx)*2-1][py*2] == ' '){ 							//LEFT
		writePlayer(getPlayerPosition(px-1+skip_lx), getPlayerPosition(py), color);
	}else if(boardMat[px*2-1][py*2] == ' ' && (px-1 == 0 || boardMat[(px-1)*2-1][py*2] == 'X')){
		if(py+skip_dy != N_CELLS-1 && boardMat[(px+skip_lx)*2][py*2+1] == ' ') 		//DOWN
			writePlayer(getPlayerPosition(px+skip_lx), getPlayerPosition(py+1), color);
		if(py+skip_uy != 0 && boardMat[(px+skip_lx)*2][py*2-1] == ' ') 						//UP
			writePlayer(getPlayerPosition(px+skip_lx), getPlayerPosition(py-1), color);
	}

	if(py+skip_dy != N_CELLS-1 && boardMat[px*2][py*2+1] == ' ' && boardMat[px*2][(py+skip_dy)*2+1] == ' '){ 			//DOWN
		writePlayer(getPlayerPosition(px), getPlayerPosition(py+1+skip_dy), color);
	}else if(boardMat[px*2][py*2+1] == ' ' && (py+1 == N_CELLS-1 || boardMat[px*2][(py+1)*2+1] == 'X')){
		if(px+skip_rx != N_CELLS-1 && boardMat[px*2+1][(py+skip_dy)*2] == ' ') 		//RIGHT
			writePlayer(getPlayerPosition(px+1), getPlayerPosition(py+skip_dy), color);
		if(px+skip_lx != 0 && boardMat[px*2-1][(py+skip_dy)*2] == ' ') 						//LEFT
			writePlayer(getPlayerPosition(px-1), getPlayerPosition(py+skip_dy), color);
	}

	if(py+skip_uy != 0 && boardMat[px*2][py*2-1] == ' ' && boardMat[px*2][(py+skip_uy)*2-1] == ' '){							//UP
		writePlayer(getPlayerPosition(px), getPlayerPosition(py-1+skip_uy), color);
	}else if(boardMat[px*2][py*2-1] == ' ' && (py-1 == 0 || boardMat[px*2][(py-1)*2-1] == 'X')){
		if(px+skip_rx != N_CELLS-1 && boardMat[px*2+1][(py+skip_uy)*2] == ' ') 		//RIGHT
			writePlayer(getPlayerPosition(px+1), getPlayerPosition(py+skip_uy), color);
		if(px+skip_lx != 0 && boardMat[px*2-1][(py+skip_uy)*2] == ' ') 						//LEFT
			writePlayer(getPlayerPosition(px-1), getPlayerPosition(py+skip_uy), color);
	}
}

