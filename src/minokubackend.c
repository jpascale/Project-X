#include "minokubackend.h"


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
	int i, auxrows, auxcolumns;
	char ** auxboard;

	auxrows = structboard->rows;
	auxcolumns = structboard->columns;
	auxboard = malloc(auxrows * sizeof(char *));

	if (auxboard == NULL)
	{	
		free(auxboard);
		return FALSE;
	}
	for(i = 0; i < auxrows; i++)
	{
		auxboard[i] = malloc(auxcolumns * sizeof(char));
		if(auxboard[i] == NULL)
		{
			freeBoard(auxboard, i);
			return FALSE;
		}	
	}
	structboard->board = auxboard;
	return TRUE;	

}

void freeBoard(char ** board, int rows)
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

/*
**	CreateHiddenVisualBoard - Creates both hidden
**	and visual board. Returns FALSE when there´s 
** 	no memory left.
*/
int CreateHiddenVisualBoard(tGame * game)
{
	if (!CreateHiddenBoard(&game->hiddenboard, game->mines) || !CreateVisualBoard(&game->visualboard))
 		return FALSE;

 	return TRUE;
}

int Query(tBoard * hiddenboard, tCommand * structcommand)
{
	tArray * pquery = &structcommand->query.results;
	int element = structcommand->query.index;
	char isrow = structcommand->query.is_row;

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
	pquery->array = results;
	
	return (results != NULL);
}

/*
** 	DoFlagUnflag - Receives game(boards), flag pos and tasks, 
**  puts flags/unflags pos, checks wether the pos
**  is empty or mined and increases/decreases left mines.
**	Returns TRUE if visualboard is modified.
*/

int DoFlagUnflag(tGame * game, tCommand * command, char task)
{
	int i = command->flag.first_pos.i;
	int j = command->flag.first_pos.j;
	
	SaveLastState(game, &command->undo);
	
	if ( (task == DO_FLAG) && (game->visualboard.board[i][j] != VISUAL_UNFLAGGED))
		return FALSE;
	else if( (task == DO_UNFLAG) && (game->visualboard.board[i][j] != VISUAL_FLAGGED))	
		return FALSE;

	game->visualboard.board[i][j] = (task == DO_FLAG? VISUAL_FLAGGED:VISUAL_UNFLAGGED);

	if (game->hiddenboard.board[i][j] == HIDDEN_MINE)
		(task == DO_FLAG? game->mines_left-- : game->mines_left++);
	
	(task == DO_FLAG)? game->flags_left-- : game->flags_left++;
		
	return TRUE;
}

int Sweep(tGame * game, tCommand * command)
{
	int i = command->sweep.i;
	int j = command->sweep.j;
	
	SaveLastState(game, &command->undo);

	if (game->hiddenboard.board[i][j] == HIDDEN_MINE)
	{
		game->visualboard.board[i][j] = HIDDEN_MINE;
		return SWEEP_MINE;
	}

	game->visualboard.board[i][j] = VISUAL_EMPTY;
	game->sweeps_left--;
	
	return TRUE;
}

int LegalPos(tBoard * structboard, tPos * pos)
{
	int i = pos->i;
	int j = pos->j;
	
	if ( (i < 0) || (j < 0) || (i >= structboard->rows) || (j >= structboard->columns))
		return FALSE;

	return TRUE;
}

int FlagRange(tGame *game, tCommand * command, char task)
{
	int k;
	int flag_count = 0;
	char isrow = command->flag.is_row;
	tPos auxpos = command->flag.first_pos;
	tPos finalpos = command->flag.last_pos;
	
	if (isrow)
	{
		for(k = auxpos.j; k<=finalpos.j; k++)
		{
			command->flag.first_pos.j = k;
			if (DoFlagUnflag(game, command, task))
				flag_count++;
		}
	}
	else
	{
		for(k = auxpos.i; k<=finalpos.i; k++)
		{
			command->flag.first_pos.i = k;
			if (DoFlagUnflag(game, command, task))
				flag_count++;
		}
	}
	
	return flag_count;
}

int WriteSaveFile(tGame *game, char *name)
{
	int i;
	FILE * savefile;
	int data[6];
	char ** hiddenboard = game->hiddenboard.board;
	char ** visualboard = game->visualboard.board;
	data[SAVEFILE_LEVEL] = game->campaign_level;
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
	FILE * loadfile;
	int auxrows=0, auxcols=0;
	int num;
	int mines=0;
	char elem;
	char ** board;
	int i;
	int campaign_len;
	int sweeps_left;
	int flags_left;
	int mines_left;
	char campaign_name[MAX_FILENAME_LEN];
	int error = FALSE;
	int hidden_created = FALSE;
	int visual_created = FALSE;
	
	if ((loadfile = fopen(name, "rb")) == NULL)
	{
		error = TRUE;
	}

	else if (fread(&num, sizeof(num), 1, loadfile) != 1)
	{
		error = TRUE; 
	}
	else if (!error)
	{
		game->campaign_level = num;
	}

	if (!error && (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < MIN_ROWS)))
	{
		error = TRUE;
	}
	else if (!error)
	{
		game->hiddenboard.rows = game->visualboard.rows = auxrows = num;
	}


	if (!error && (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < MIN_COLUMNS)))
	{
		error = TRUE;
	}
	else if (!error)
	{
		game->hiddenboard.columns = game->visualboard.columns = auxcols = num;
	}


	if (!error && (fread(&num, sizeof(num), 1, loadfile) > get_undos(game->level)))
		error = TRUE;
	else if (!error)
		game->undos = num;


	if (!error && (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < 0)))
		error = TRUE;
	else if (!error)
	{
		game->moves = num;
		game->gametype = game->moves? GAMETYPE_INDIVIDUAL_LIMIT : GAMETYPE_INDIVIDUAL_NOLIMIT;
	}


	if (!error && (fread(&num, sizeof(num), 1, loadfile) != 1 || (num < 0 || num > 1 || (num == 1 && !game->moves))))
		error = TRUE;
	else if (!error)
		game->gametype = num? GAMETYPE_CAMPAIGN : game->gametype;



	if (!error && CreateBoard(&game->hiddenboard)==FALSE)
	{

		fclose(loadfile);
		return MALLOC_ERR;
	}
	if (!error)
	{
		board = game->hiddenboard.board;
		hidden_created = TRUE;
	}

	for (i = 0; !error && i < auxcols * auxrows; i++)
	{
		elem = fgetc(loadfile);
		if (elem != HIDDEN_MINE && elem != HIDDEN_EMPTY)
			error = TRUE;
		else if (elem == HIDDEN_MINE)
			mines++;
		if (!error)
			board[i/auxcols][i%auxcols] = elem;
	}
	
	if (!error)
	{
	game->mines = mines_left = flags_left = mines;
	sweeps_left = auxcols * auxrows - mines;
	}

	if (!error && CreateBoard(&game->visualboard)==FALSE)
	{
		freeBoard(game->hiddenboard.board, auxrows);
		fclose(loadfile);
		return MALLOC_ERR;
	}
	if (!error)
	{
		board = game->visualboard.board;
		visual_created = TRUE;
	}

	for (i = 0; !error && i < auxcols * auxrows; i++)
	{
		elem = fgetc(loadfile);
		if ((elem != VISUAL_UNFLAGGED && elem != VISUAL_EMPTY && elem != VISUAL_FLAGGED) || \
            (elem == VISUAL_EMPTY && game->hiddenboard.board[i/auxcols][i%auxcols] == HIDDEN_MINE))
			error = TRUE;
		else
		{
			board[i/auxcols][i%auxcols] = elem;
			flags_left -= (elem == VISUAL_FLAGGED);
			sweeps_left -= (elem == VISUAL_EMPTY);
			mines_left -= (elem == VISUAL_FLAGGED && game->hiddenboard.board[i/auxcols][i%auxcols] == HIDDEN_MINE);
		}
	}
	if (!error)
	{
		game->flags_left = flags_left;
		game->sweeps_left = sweeps_left;
		game->mines_left = mines_left;
	}

	if (!error && game->gametype == GAMETYPE_CAMPAIGN)
	{
		if (fgets(campaign_name, MAX_FILENAME_LEN, loadfile) == NULL || (campaign_len = strlen(campaign_name)) < FORMAT_LENGTH + 1)
			error = TRUE;
		
		else if (strstr(&(campaign_name[campaign_len-FORMAT_LENGTH]), FILE_FORMAT) == NULL)
			error = TRUE;
		else
			strcpy(game->campaign_name, campaign_name);
	}

	if (!error && fgetc(loadfile) != EOF)
		error = TRUE;

	else if (!error)
		game->gamestate=GAMESTATE_DEFAULT;
	if (error)
	{
		if (hidden_created)
		{
			freeBoard(game->hiddenboard.board, auxrows);
		}
		if (visual_created)
		{
			freeBoard(game->visualboard.board, auxrows);
		}
	}
	if (loadfile != NULL)
		fclose(loadfile);
	return !error;

}


void SaveLastState(tGame * game, tUndo * undo )
{
	CopyBoard(&game->visualboard, &undo->lastboard);
	undo->mines_left = game->mines_left;
	undo->sweeps_left = game->sweeps_left;
	undo->flags_left = game->flags_left; 
	undo->can_undo = TRUE;
}

static void CopyBoard(tBoard * board_from, tBoard * board_to)
{
	int i,j;
	int dimi, dimj;
	char ** from;
	char ** to;

	dimi = board_from->rows;
	dimj = board_from->columns;

	from = board_from->board;
	to = board_to->board;
	

	for (i = 0; i < dimi; i++)
		for (j = 0; j < dimj; j++)
			to[i][j] = from[i][j];

}

int Undo(tGame * game, tUndo * undo)
{
	undo->can_undo = FALSE;
	game->mines_left = undo->mines_left;
	game->sweeps_left = undo->sweeps_left;
	game->flags_left = undo->flags_left;
	CopyBoard(&undo->lastboard, &game->visualboard);

	return TRUE;
}

void CheckGameState(tGame * game)
{

	if (!game->mines_left || !game->sweeps_left)
		game->gamestate = GAMESTATE_WIN;

	/* Campaign or limited */
	if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
	{
		if (game->moves < game->sweeps_left && game->moves < game->mines_left)
			game->gamestate = GAMESTATE_CANTWIN; 
	}

	return;
}

int LoadCampaign(tGame * game)
{
	FILE * campaign_file;
	tCampaign * aux;
	int level, rows, columns;
	int k=0;
	char line[MAX_CAMPAIGN_LINE_LENGTH];
	int error = FALSE;
	int len;
	game->campaign=NULL;
	
	if ((campaign_file = fopen(game->campaign_name, "rt")) == NULL)
		return FALSE;
	
	while (!error && fgets(line, MAX_CAMPAIGN_LINE_LENGTH, campaign_file) != NULL)
	{
		len=strlen(line);

		if (line[len-1] != '\n')
			error = TRUE;
		else
		{
			if (sscanf(line, "%d\t%dx%d", &level, &rows, &columns) != 3)
				error = TRUE;
			else
			{
				if (level < EASY || level > NIGHTMARE || \
                    rows < MIN_ROWS || columns < MIN_COLUMNS || \
                    (level == NIGHTMARE && rows * columns < 100))
					error = TRUE;
				else
				{
					if (k % BLOCK == 0)
					{
						aux = realloc (game->campaign, (k + BLOCK) * sizeof(*(game->campaign)));
						
						if (aux == NULL)
						{
							free(game->campaign);
							return MALLOC_ERR;
						}
						game->campaign = aux;
						
					}
					game->campaign[k].level = level;
					game->campaign[k].rows = rows;
					game->campaign[k].columns = columns;
					k++;
				}
			}
		}
	}
	if (!error)
	{
		game->campaign = realloc (game->campaign, k * sizeof(*(game->campaign)));
		game->levels_amount = k;
	}
	return !error;
}
