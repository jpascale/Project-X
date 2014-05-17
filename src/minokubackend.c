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

void initializeVisualBoard(tBoard * structboard)
{
	int i, j, dimrows=structboard->rows, dimcols=structboard->columns;
	for (i=0; i<dimrows; i++)
		for (j=0; j<dimcols; j++)
			structboard->board[i][j]=VISUAL_UNFLAGGED;
}

int
CreateBoard(tBoard * structboard)
{
	int i, auxrows, auxcolumns;
	char ** auxboard;
	auxboard = structboard->board;
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

void
freeBoard(char ** Board, int rows)
{
	int i;
	for(i=0;i<rows;i++)
		free(Board[i]);
	free(Board);
}
