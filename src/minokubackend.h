//_____minokubackend.h_____//

/*
**		Includes
*/
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getnum.h"
#include "random.h"

/*
**		Macros
*/
#define DEBUG

#define EASY 1
#define MEDIUM 2
#define HARD 3
#define NIGHTMARE 4

#define GAMETYPE_INDIVIDUAL_NOLIMIT 0
#define GAMETYPE_INDIVIDUAL_LIMIT 1
#define GAMETYPE_CAMPAIGN 2

#define UNLIMITED_MOVES 0

/*
**		Structs
*/
typedef struct
{
	char ** board;
	int rows;
	int columns;

} tBoard;

typedef struct 
{
	tBoard visualboard;
	tBoard hiddenboard;
	int gametype;
	int level;
	int moves;
	int undos;
	int mines;

} tGame;

/*
**		Function prototypes (front)
*/
int Menu(void);
void Menu2(tGame * game);
void getLevelandDim(tGame * game);

/*
**		Function prototypes (back)
*/
