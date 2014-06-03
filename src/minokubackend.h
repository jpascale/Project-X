/*_____minokubackend.h_____*/

#ifndef _MINOKUBACK_H
	#define _MINOKUBACK_H

/*
**		Includes
*/
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "getnum.h"
#include "random.h"

/*
**		Macros
*/

/* Clears shell */
#define DELSHELL() printf("\033[2J\033[1;1H")

#define FALSE 	0
#define TRUE 	1

/* COLOR CONSTANTS */
#define KDEF  "\x1B[0m"		/* default */

/*	Color constants for Board */
#define KMIN  "\x1B[31m" 	/* MINE - RED */ 
#define KSWP  "\x1B[32m"	/* SWEEP - GREEN */
#define KFLG  "\x1B[34m"	/* FLAG - BLUE */
#define KREF  "\x1B[36m"	/* REFERENCE - CYAN */

#define KMSG  "\x1B[34m"	/* MESSAGE - BLUE */
#define KASK  "\x1B[31m"	/* ASK - RED */
#define KEXC  "\x1B[32m"	/* EXCLAME - GREEN */
#define KERR  "\x1B[31m"	/* ERROR - RED */

/* Levels */
#define EASY 		1
#define MEDIUM		2
#define HARD 		3
#define NIGHTMARE	4


/* Campaign */
#define INDIVIDUAL_GAME 0

/* Query States */
#define NOT_FOUND_MINE 	0
#define FOUND_MINE 		1

/* Sweep */
#define SWEEP_MINE -1

/* Chars for hidden board */
#define HIDDEN_MINE 	'#'
#define HIDDEN_EMPTY 	'-'

/* Chars for visual display */
#define VISUAL_UNFLAGGED 	'0'
#define VISUAL_FLAGGED 		'&'
#define VISUAL_EMPTY 		'-'

#define GAMETYPE_INDIVIDUAL_NOLIMIT 0
#define GAMETYPE_INDIVIDUAL_LIMIT 	1
#define GAMETYPE_CAMPAIGN 			2

#define UNLIMITED_MOVES 0

/* Minimum and maximum dim */
#define MIN_ROWS 5
#define MIN_COLUMNS 5
#define MAX_ROWS 19
#define MAX_COLUMNS 19

/* Level mines percentage */
#define PERCENT_EASY 		0.2
#define PERCENT_MEDIUM 		0.5
#define PERCENT_HARD 		0.7
#define PERCENT_NIGHTMARE 	0.9

/* Command constants */
#define MAX_COMMAND_LEN 	8
#define MAX_PARAMS_LEN 		24
#define COMMANDS_NUMBER 	7
#define MAX_FILENAME_LEN 	24

#define COMMAND_SWEEP	0
#define COMMAND_FLAG	1
#define COMMAND_UNFLAG 	2
#define COMMAND_QUERY 	3
#define COMMAND_SAVE	4
#define COMMAND_QUIT 	5
#define COMMAND_UNDO 	6

/* Flag/Unflag tasks */
#define DO_FLAG		0
#define DO_UNFLAG 	1

/* Malloc constants */
#define BLOCK 5
#define MALLOC_ERR -1

/* Savefile constants */
#define SAVEFILE_LEVEL 0
#define SAVEFILE_ROWS 1
#define SAVEFILE_COLUMNS 2
#define SAVEFILE_UNDOS 3
#define SAVEFILE_MOVES 4
#define SAVEFILE_ISCAMPAIGN 5

#define GAMESTATE_DEFAULT 	0
#define GAMESTATE_WIN 		1
#define GAMESTATE_LOSE 		2
#define GAMESTATE_CANTWIN 	3

/* Campaign Format */
#define FILE_FORMAT ".txt"
#define FORMAT_LENGTH 4

/* map undos quantity */
#define get_undos(level) (((level)==NIGHTMARE)?1: \
						  ((level)==HARD)?3: \
						  ((level)==MEDIUM)?5:10)

/* Gets player moves */
#define get_moves(mines, undos) ((mines) + (undos))

/* Maps letter reference to board row number */
#define get_row_pos_byref(row) ( (row) - 'A')

/* Number to upper letter */
#define toupperalpha(x) ((x)+'A')

/* Maximum and minimum */
#define min(x, y) ((x)<(y)?(x):(y))
#define max(x, y) ((x)>(y)?(x):(y))

/* Delete buffer */
#define DELBFF() while(getchar() != '\n')


#define CLEAR_SCREEN() printf("\e[1;1H\e[2J")

#define COLORBOARD	((board[i][j] == VISUAL_UNFLAGGED)? KMIN: \
					((board[i][j] == VISUAL_FLAGGED)? KFLG: \
					((board[i][j] == VISUAL_EMPTY)? KSWP:KMIN)))

/*Campaign Format */
#define FILE_FORMAT ".txt"
#define FORMAT_LENGTH 4
#define MAX_CAMPAIGN_LINE_LENGTH 9


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
	int level;
	int rows;
	int columns;

} tCampaign;


typedef struct 
{
	tBoard visualboard;
	tBoard hiddenboard;
	int gametype;
	int level;
	int moves;
	int undos;
	int mines;
	int mines_left; 	/* number of mines not flagged */
	int sweeps_left; 	/* Number of not sweeped positions */
	int flags_left;		/* Resting flags */
	char campaign_name[MAX_FILENAME_LEN];
	int campaign_level;
	int gamestate;		/* DEFAULT, WIN or LOSE */
	int levels_amount;	/* Number of campaign levels */
	tCampaign * campaign;

} tGame;

typedef struct
{
	int i;
	int j;

} tPos;

typedef struct
{
	tBoard lastboard;
	int mines_left; 
	int sweeps_left;
	int flags_left;
	char can_undo;
	char undo_error;
} tUndo;

typedef struct
{
	int * array;
	int dim;

} tArray;

typedef struct
{
	char command[MAX_COMMAND_LEN];
	char params[MAX_PARAMS_LEN];

} tScan;

typedef struct 
{
	tPos first_pos;
	tPos last_pos;
	char is_range;
	char is_row;

} tFlag;

typedef struct 
{
	tArray results;
	int index;
	char is_row;

} tQuery;

typedef struct
{
	int command_ref;
	tPos sweep;
	tFlag flag;
	tQuery query;
	tUndo undo;
	char save_filename[MAX_FILENAME_LEN];

} tCommand;

/*
**		Function prototypes (back)
*/

/*Sets how many mines the board will have based on the difficulty*/
void setGameMinesNumber(tGame * game);

/*Reserves dynamic memory for a board, if there's no memory, it returns an error*/
int CreateBoard(tBoard * structboard);

/*Sets the mines on random positions on the board*/
int InitBoardMines(tBoard * structboard, int mines);

/*Sets all spaces on a board to a specific character*/
void InitBoard(tBoard * structboard, char initchar);

/*Creates the visual board and initializes it with '0'*/
int CreateVisualBoard(tBoard * structboard);

/*Creates the hidden board and puts the mines in random positions of it*/
int CreateHiddenBoard(tBoard * structboard, int mines);

/*
**	CreateHiddenVisualBoard - Creates both hidden
**	and visual board. Returns FALSE when there´s 
** 	no memory left.
*/
int CreateHiddenVisualBoard(tGame * game);

/*Returns an array with mines distribution on a certain column or row*/
int Query(tBoard * structboard, tCommand * structcommand);

/*Flags or unflags a certain space, based on task parameter*/
int DoFlagUnflag(tGame * game, tCommand * command, char task);

/*Sweeps a space, if there's a mine returns SWEEP_MINE*/
int Sweep(tGame * game, tCommand * command);

/*Checks if a certain position exists on a certain board, returns TRUE or FALSE*/
int LegalPos(tBoard * structboard, tPos * position);

/*Flags or unflags all spaces on a given range, returns amount of modified spaces*/
int FlagRange(tGame *game, tCommand * command, char task);

/* Writes on a given binary file current game's info in the following order:
** Campaign_level
** Rows
** Columns
** Undos
** Moves Left (0 if unlimited)
** Campaign? 1:0
** Hidden Board
** Visual Board
** Campaign filename (if Campaign == 1)
*/
int WriteSaveFile(tGame *game, char *name);

/*Saves the current board before modifying it*/
void SaveLastState(tGame * game, tUndo * undo);

/*Checks if the game is finished, returns if won or lost*/
void CheckGameState(tGame * game);

/* Loads a campaign text file finished in '.txt' with the following format:
** DIFFICULTY ROWSxCOLUMNS
** DIFFICULTY ROWSxCOLUMNS
** ...
** DIFFICULTY ROWSxCOLUMNS[EOF]
*/
int LoadCampaign(tGame * game);

/* Checks if a binary savefile is valid, then loads it */
int LoadFile(tGame *game, char *name);

/*Undoes last move*/
int Undo(tGame * game, tUndo * undo);

/*Frees the memory reserved for a board*/
void freeBoard(char ** Board, int rows);

#endif
