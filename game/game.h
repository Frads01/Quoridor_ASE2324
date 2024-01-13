/* Game mechanics header */

#include "LPC17xx.h"

//Posizione iniziale muro
#define WALL_INIT_POS_X 3
#define WALL_INIT_POS_Y 2

//Posizione iniziale giocatori
#define PL0_INIT_POS_X 3
#define PL0_INIT_POS_Y 6
#define PL1_INIT_POS_X 3
#define PL1_INIT_POS_Y 0

//Numero muri iniziale
#define PL_INIT_WALLS 8

//enum usati per:
// - ottenere info sulla mossa (funzione uint8_t getMoveInfo(enum en_info ino); )
enum en_info {player, mode, dir, py, px};
// - rappresentare la direzione scelta
enum en_dir {up_e, down_e, left_e, right_e};

void initGame(void);
void moveElement(enum en_dir direction);
void placePlayer(uint16_t newX, uint16_t newY, uint8_t player);
void placeWall(uint16_t newX, uint16_t newY, uint8_t rotate, uint8_t player);
uint8_t checkFreePath(void);
uint8_t getMoveInfo(enum en_info info);
void DFS(uint8_t visited[7][7], uint8_t xMat, uint8_t yMat);
