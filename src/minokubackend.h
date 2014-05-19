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

#define FALSE 0
#define TRUE 1

// Levels
#define EASY 		1
#define MEDIUM		2
#define HARD 		3
#define NIGHTMARE	4

// Chars for hidden board
#define HIDDEN_MINE 	'#'
#define HIDDEN_EMPTY 	'-'

// Chars for visual display
#define VISUAL_UNFLAGGED 	'0'
#define VISUAL_FLAGGED 		'&'
#define VISUAL_EMPTY 		'-'
#define VISUAL_MINE 		'#'

#define GAMETYPE_INDIVIDUAL_NOLIMIT 0
#define GAMETYPE_INDIVIDUAL_LIMIT 1
#define GAMETYPE_CAMPAIGN 2

#define UNLIMITED_MOVES 0

// Level mines percentage
#define PERCENT_EASY 0.2
#define PERCENT_MEDIUM 0.5
#define PERCENT_HARD 0.7
#define PERCENT_NIGHTMARE 0.9

// map undos quantity
#define get_undos(level) ((level==NIGHTMARE)?1: \
						  (level==HARD)?3: \
						  (level==MEDIUM)?5:10)

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
void PlayMenu(tGame * game);
void getLevelandDim(tGame * game);
void printBoard(tBoard * structboard);

/*
**		Function prototypes (back)
*/
void setMines(tGame * game);
void freeBoard(char ** Board, int rows);
int CreateBoard(tBoard * structboard);
int InitHiddenBoard(tBoard * structboard, int mines);
int InitBoardMines(tBoard * structboard, int mines);
void InitBoard(tBoard * structboard, char initchar);
