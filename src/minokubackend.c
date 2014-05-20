#include "minokubackend.h"

void setMines(tGame * game)
{
	//20 50 70 90
	int dim = game->hiddenboard.rows * game->hiddenboard.columns;
	int mines;

	switch (game->level)
	{
		case EASY:
			mines = dim * PERCENT_EASY;
			break;

		case MEDIUM:
			mines = dim * PERCENT_MEDIUM;
			break;
		
		case HARD:
			mines = dim * PERCENT_HARD;
			break;
		
		case NIGHTMARE:
			mines = dim * PERCENT_NIGHTMARE;
			break;
	}

	game->mines = mines;

	return;
}

int CreateBoard(tBoard * structboard)
{
	int i, auxrows, auxcolumns;
	char ** auxboard;
	auxrows = structboard->rows;
	auxcolumns = structboard->columns;
	auxboard = malloc(auxrows*sizeof(char*));
	if (auxboard == NULL)
	{	
		free(auxboard);
		return FALSE;
	}
	for(i=0; i<auxrows; i++)
	{
		auxboard[i] = malloc(auxcolumns*sizeof(char));
		if(auxboard[i]==NULL)
		{
			freeBoard(auxboard, i);
			return FALSE;
		}	
	}
	structboard->board = auxboard;
	return TRUE;	

}

void freeBoard(char ** Board, int rows)
{
	int i;
	for(i=0;i<rows;i++)
		free(Board[i]);
	free(Board);
}

int InitBoardMines(tBoard * structboard, int mines)
{
	int i, k;
	int auxrows = structboard->rows;
	int auxcolumns = structboard->columns;
	char ** board = structboard->board;

	int * randvec;
	int dimrandvec;

	int randpos;
	int boardpos;

	dimrandvec = auxrows * auxcolumns;

	randvec = malloc(dimrandvec * sizeof(*randvec));
	if (randvec == NULL)
		return FALSE;

	/* Board positions taken randomly to put mines */
	for (k = 0; k < dimrandvec; k++)
		randvec[k] = k;

	for (i = 0; i < mines; i++)
	{
		randpos = randint(0, dimrandvec-1);
		boardpos = randvec[randpos];

		board[boardpos/auxrows][boardpos%auxrows] = HIDDEN_MINE;

		randvec[randpos] = randvec[--dimrandvec];
	}

	return TRUE;
}

int CreateHiddenBoard(tBoard * structboard, int mines)
{
	if (CreateBoard(structboard) == FALSE)
		return FALSE;
	InitBoard(structboard, HIDDEN_EMPTY);
	if (InitBoardMines(structboard, mines) == FALSE)
		return FALSE;
	return TRUE;
}

void InitBoard(tBoard * structboard, char initchar)
{
	int i,j;
	int dimr = structboard->rows;
	int dimc = structboard->columns;
	char ** board = structboard->board;

	for (i = 0; i < dimr ; i++)
		for (j = 0; j < dimc; j++)
			board[i][j] = initchar;

	return;
}

int CreateVisualBoard(tBoard * structboard)
{
	if (CreateBoard(structboard) == FALSE)
		return FALSE;
	InitBoard(structboard, VISUAL_UNFLAGGED);
	return TRUE;

}
