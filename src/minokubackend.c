#include "minokubackend.h"

static void freeBoard(char ** Board, int rows);
static void CopyBoard(tBoard * board_from, tBoard * board_to);

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

		board[boardpos/auxcolumns][boardpos%auxcolumns] = HIDDEN_MINE;

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

int Query(tBoard * hiddenboard, tArray * pquery, int element, char isrow)
{
   //ToDo: Free array after use
  //ToDo: Modularize??
  //ToDo: Think what is going to return this

	int i, j;
	int boarddim = isrow ? hiddenboard->columns : hiddenboard->rows;

	int * aux;
	int * results = NULL;
	char ** board = hiddenboard->board;
	
	char state = NOT_FOUND_MINE;

	for (i = 0, j = 0; i < boarddim; i++)
	{
		switch (state)
		{
			case NOT_FOUND_MINE:
				if ((isrow ? board[element][i] : board[i][element]) == HIDDEN_MINE)
				{
					state = FOUND_MINE;

					if (j % BLOCK == 0)
					{
						aux = realloc(results, (j + BLOCK) * sizeof(*results));

						if (aux == NULL)
						{
							if (j != 0)
								free(results);
							return MALLOC_ERR;
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

	if (results != NULL)
		results = realloc(results, (j + (state == FOUND_MINE)) * sizeof(*results));
	pquery->dim = j + (state == FOUND_MINE);
	pquery->results = results;
	
	return (results != NULL);
}

/*
** 	DoFlagUnflag - Receives game(boards), flag pos and tasks, 
**  puts flags/unflags pos, checks wether the pos
**  is empty or mined and increases/decreases left mines.
**	Returns TRUE if visualboard is modified.
*/

int DoFlagUnflag(tGame * game, tPos * pos, char task, tCommand * command)
{
	int i = pos->i;
	int j = pos->j;

	// Not possible to flag/unflag sweeped pos
	if (game->visualboard.board[i][j] == VISUAL_EMPTY)
		return FALSE;
	else if (task == DO_FLAG && game->visualboard.board[i][j] == VISUAL_FLAGGED)
		return FALSE;
	else if(task == DO_UNFLAG && game->visualboard.board[i][j] == VISUAL_UNFLAGGED)
		return FALSE;
	if(command->flag.is_range)
		SaveLastState(game, &command->undo);
	game->visualboard.board[i][j] = (task == DO_FLAG? VISUAL_FLAGGED:VISUAL_UNFLAGGED);

	if (game->hiddenboard.board[i][j] == HIDDEN_MINE)
		(task == DO_FLAG? game->mines_left-- : game->mines_left++);
	
	return TRUE;
}

int
Sweep(tGame * game, tPos * pos, tCommand * command)
{
	int i = pos->i;
	int j = pos->j;
	
	if (game->hiddenboard.board[i][j] == HIDDEN_MINE)
		return SWEEP_MINE;
	
	SaveLastState(game, &command->undo);
	
	game->visualboard.board[i][j] = VISUAL_EMPTY;
	game->sweeps_left--;
	
	return TRUE;
}

int
LegalPos(tBoard * structboard, tPos * pos)
{
	int i = pos->i;
	int j = pos->j;
	
	if (i < 0 || j < 0 || i > structboard->rows || j > structboard->columns)
		return FALSE;

	return TRUE;
}

int ExecCommand(tGame *game, tCommand *command)
{
	//ToDo: tidy. front.

	int i = command->command_ref; 
	int res;

	switch (i)
	{
		case COMMAND_SWEEP:
			printf("En ejecutar: %d %d\n", command->sweep.i, command->sweep.j);
			res = Sweep(game, &command->sweep, command);
			break;
		
		case COMMAND_FLAG:
			if (!(command->flag.is_range))
				res=DoFlagUnflag(game, &(command->flag.first_pos), DO_FLAG, command);
			else
				res=FlagRange(game, &(command->flag), DO_FLAG, command);
			break;
		
		case COMMAND_UNFLAG:
			if (!(command->flag.is_range))
				res = DoFlagUnflag(game, &(command->flag.first_pos), DO_UNFLAG, command);
			else
				res=FlagRange(game, &(command->flag), DO_UNFLAG, command);
			break;
		
		case COMMAND_QUERY:
			res = Query(&(game->hiddenboard), &(command->query.results), command->query.index, command->query.is_row);
			if (res)
				PrintQuery(&command->query);
			else
				printf("0\n");
			//Free vec
			break;

		case COMMAND_SAVE:
			//res=WriteSaveFile(game, command->save_filename);
			break;
		
		case COMMAND_QUIT:
			/*exit*/
			break;
		
		case COMMAND_UNDO:
			/*undo*/
			break;


	}
	if (i == COMMAND_SWEEP || i == COMMAND_FLAG || i == COMMAND_UNFLAG)
		game->moves--;
	return res;
}

int FlagRange(tGame *game, tFlag *flag, char task, tCommand * command)
{
	//ToDo: Tidy 
	int k;
	int res=FALSE;
	char isrow = flag->is_row;
	tPos auxpos = flag->first_pos;
	tPos finalpos = flag->last_pos;
	SaveLastState(game, &command->undo); //Cambiar (Recorrer primero y ver si cambias algo, despues guardar, despues cambiar)
	if (isrow)
	{
		for(k = auxpos.j; auxpos.j<=finalpos.j; auxpos.j = ++k)
			res = DoFlagUnflag(game, &auxpos, task) || res;
	}
	else
	{
		for(k = auxpos.i; auxpos.i<=finalpos.i; auxpos.i = ++k)
			res = DoFlagUnflag(game, &auxpos, task) || res;
	}
	
	return res;
}

int WriteSaveFile(tGame *game, char *name)
{
	int i;
	FILE * savefile;
	int data[6];
	char ** hiddenboard = game->hiddenboard.board;
	char ** visualboard = game->visualboard.board;
	data[SAVEFILE_LEVEL] = game->level;
	data[SAVEFILE_ROWS] = game->hiddenboard.rows;
	data[SAVEFILE_COLUMNS] = game->hiddenboard.columns;
	data[SAVEFILE_UNDOS] = game->undos;
	data[SAVEFILE_MOVES] = game->gametype? game->moves : UNLIMITED_MOVES;
	data[SAVEFILE_ISCAMPAIGN] = game->gametype == GAMETYPE_CAMPAIGN ? 1 : 0;

	if ((savefile = fopen(name, "wb")) == NULL)
		return FALSE;

	if ( fwrite(data, sizeof(*data), sizeof(data)/sizeof(*data), savefile) !=  sizeof(data)/sizeof(*data) )
	{
		fclose(savefile);
		return FALSE;
	}
	// ToDo: Use aux function or macro

	for (i = 0; i < data[SAVEFILE_ROWS]; i++)
		if (fwrite(hiddenboard[i], sizeof(char), data[SAVEFILE_COLUMNS], savefile) != data[SAVEFILE_COLUMNS])
		{
			fclose(savefile);
			return FALSE;
		}

	for (i = 0; i < data[SAVEFILE_ROWS]; i++)
		if (fwrite(visualboard[i], sizeof(char), data[SAVEFILE_COLUMNS], savefile) != data[SAVEFILE_COLUMNS])
		{
			fclose(savefile);
			return FALSE;
		}
	
	if (data[SAVEFILE_ISCAMPAIGN])
		if( fputs(game->campaign_name, savefile) == EOF)
		{
			fclose(savefile);
			return FALSE;
		}
	
	fclose(savefile);
	return TRUE;

}

int LoadFile(tGame *game, char *name)
{
	//ToDo: Modularize
	//ToDo: Finish
	FILE * loadfile;
	int auxrows, auxcols;
	int num;
	int mines=0;
	char elem;
	char ** board;
	int i;
	
	if ((loadfile = fopen(name, "rb")) == NULL)
		return FALSE;

	if (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < EASY || num > NIGHTMARE) )
	{
		fclose(loadfile);
		return FALSE;
	}

	game->level = num;

	if (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < 5))
	{

		fclose(loadfile);
		return FALSE;
	}

	game->hiddenboard.rows = game->visualboard.rows = auxrows = num;


	if (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < 5))
	{

		fclose(loadfile);
		return FALSE;
	}
	
	game->hiddenboard.columns = game->visualboard.columns = auxcols = num;


	if (fread(&num, sizeof(num), 1, loadfile) > get_undos(game->level))
	{

		fclose(loadfile);
		return FALSE;
	}
	game->undos=num;


	if (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < 0))
	{

		fclose(loadfile);
		return FALSE;
	}

	game->moves = num;
	game->gametype = game->moves? GAMETYPE_INDIVIDUAL_LIMIT : GAMETYPE_INDIVIDUAL_NOLIMIT;


	if (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < 0 || num > 1 || (num == 1 && !game->moves)))
	{

		fclose(loadfile);
		return FALSE;
	}
	
	game->gametype = num? GAMETYPE_CAMPAIGN : game->gametype;



	if (CreateBoard(&game->hiddenboard)==FALSE)
	{

		fclose(loadfile);
		return MALLOC_ERR;
	}
	board = game->hiddenboard.board;

	for (i = 0; i < auxcols * auxrows && mines <= getmines(auxcols * auxrows, game->level) ; i++)
	{
		elem = fgetc(loadfile);
		if (elem != HIDDEN_MINE && elem != HIDDEN_EMPTY)
		{
	
			freeBoard(game->hiddenboard.board, auxrows);
			fclose(loadfile);
			return FALSE;
		}
		if (elem == HIDDEN_MINE)
			mines++;
		board[i/auxcols][i%auxcols] = elem;
	}

	if (mines != getmines(auxcols * auxrows, game->level))
	{

		freeBoard(game->hiddenboard.board, auxrows);
		fclose(loadfile);
		return FALSE;
	}
	game->mines=mines;

	if (CreateBoard(&game->visualboard)==FALSE)
	{

		fclose(loadfile);
		return MALLOC_ERR;
	}
	board = game->visualboard.board;

	for (i = 0; i < auxcols * auxrows; i++)
	{
		elem = fgetc(loadfile);
		if ((elem != VISUAL_UNFLAGGED && elem != VISUAL_EMPTY && elem != VISUAL_UNFLAGGED) || (elem == VISUAL_EMPTY && game->hiddenboard.board[i/auxcols][i%auxcols] != HIDDEN_MINE))
		{

			freeBoard(game->hiddenboard.board, auxrows);
			freeBoard(game->visualboard.board, auxrows);
			fclose(loadfile);
			return FALSE;
		}
		board[i/auxcols][i%auxcols] = elem;
	}


	fclose(loadfile);
	return TRUE;

}


void SaveLastState(tGame * game, tUndo * undo )
{
	CopyBoard(&game->visualboard, &undo->lastboard);
	undo->mines_left = game->mines_left;
	undo->sweeps_left = game->sweeps_left;
	//undo->flags_left = game->flags_left; 
}

static void CopyBoard(tBoard * board_from, tBoard * board_to)
{
	int i,j;
	int dimi, dimj;
	char ** from, to;

	dimi = board_from->rows;
	dimj = board_from->columns;

	from = board_from->board;
	to = board_to->board;
	

	for (i = 0; i < dimi; i++)
		for (j = 0; j < dimj; j++)
			to[i][j] = from[i][j];


}
