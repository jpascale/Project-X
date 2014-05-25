#include "minokubackend.h"

static void freeBoard(char ** Board, int rows);

void setGameMinesNumber(tGame * game)
{

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
	//ToDo: Tidy
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

static void freeBoard(char ** board, int rows)
{
	int i;

	for(i = 0; i < rows; i++)
		free(board[i]);
	free(board);
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

int Query(tBoard * structboard, tArray * pquery, int element, char isrow, int block)
{ //ToDo: Use realloc directly
  //ToDo: Modularize??
  //ToDo: Think what is going to return this

	int i, j;
	int boarddim = isrow ? structboard->columns : structboard->rows;

	int * aux;
	int * results = pquery-> results;
	char ** board = structboard->board;
	
	char state = NOT_FOUND_MINE;
	
	results = malloc(block * sizeof(*results));
	
	if (results == NULL)
		return FALSE;

	results[0] = 0;

	for (i = 0, j = 0; i < boarddim; i++)
	{
		switch (state)
		{
			case NOT_FOUND_MINE:
				if ((isrow && board[element][i] == HIDDEN_MINE) || (!isrow && board[i][element] == HIDDEN_MINE))
				{
					state = FOUND_MINE;

					if (j % block == 0 && j > 0)
					{
						aux = realloc(results, (j + block) * sizeof(*results));

						if (aux == NULL)
						{
							free(results);
							return FALSE;
						}

						results = aux;
					}

					results[j] = 1;
				}
				break;

			case FOUND_MINE:
				if ((isrow ? board[element][i] : board[i][element]) == HIDDEN_MINE)
					results[j]++;
				else
				{
					state = NOT_FOUND_MINE;
					j++;
				}
				break;
		}
	}
	 /* If no mines are found returns 0 */
	results = realloc(results, max((j + (state == FOUND_MINE)), 1) * sizeof(*results));
	
	pquery->dim = max(j + (state == FOUND_MINE), 1);
	
	return TRUE;
}

/*
** 	DoFlagUnflag - Receives game(boards), flag pos and tasks, 
**  puts flags/unflags pos, checks wether the pos
**  is empty or mined and increases/decreases left mines.
**	Returns TRUE if visualboard is modified.
*/

int DoFlagUnflag(tGame * game, tPos * pos, char task)
{
	int i = pos->i;
	int j = pos->j;

	// Not possible to flag/unflag
	if (game->visualboard.board[i][j] == VISUAL_EMPTY)
		return FALSE;

	game->visualboard.board[i][j] = (task == DO_FLAG? VISUAL_FLAGGED:VISUAL_UNFLAGGED);

	if (game->hiddenboard.board[i][j] == HIDDEN_MINE)
		(task == DO_FLAG? game->mines_left-- : game->mines_left++);

	return TRUE;
}

int
Sweep(tGame * game, tPos * pos)
{
	int i = pos->i;
	int j = pos->j;
	
	if (game->hiddenboard.board[i][j] == HIDDEN_MINE)
		return SWEEP_MINE;
	
	game->visualboard.board[i][j] = VISUAL_EMPTY;
	
	game->sweeps_left--;

	return TRUE;
}

int
LegalPos(tBoard * structboard, tPos * pos)
{
	int i = pos->i;
	int j = pos->j;
	
	if (i < 0 || j < 0 || i >= structboard->rows || j >= structboard->columns)
		return FALSE;

	return TRUE;
}