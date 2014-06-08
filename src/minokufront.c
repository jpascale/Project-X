#include "minokubackend.h"

#define MEM_ERR "No hay suficiente memoria para seguir jugando."
#define FILE_ERR "Archivo invalido o inexistente."
#define CFILE_ERR "Archivo de campaña invalido o inexistente."
#define OPT_ERR "Ingrese una opcion valida."
#define NIGHTMARE_ERR "No es posible elegir pesadilla con menos de 100 casilleros."
#define COMMAND_ERR "Comando invalido."
#define ASKUNDO_ERR "Ingresar quit o undo."
#define UNDO_ERR "No es posible usar undo."
#define NOMOVES_ERR "No quedan suficientes movimientos para ganar la partida."

/*
**		Function prototypes (front) 
*/

/*
**	Menu - Initial menu: Play, Load, Quit.
*/
int Menu(void);

/*
**	setGametypeMenu - Choose gametype: No moves limit, Limited moves, Campaign.   
*/
void setGametypeMenu(tGame * game);

/*
**	PrintBoard - Prints the gameboard.
*/
void PrintBoard(tBoard * structboard);

/*
**	getLevel - Asks for level taking into account nightmare restriction.
*/
void getLevel(tGame * game);

/*
**	getDim - Asks for board dim.
*/
void getDim(tGame * game);

/*
**	setNewGame - Sets All the necesary info to play in game structure.  
*/ 
int setNewGame(tGame * game);

/*
**	Play - Receives gameplay configured structure and starts game.
*/
void Play(tGame * game);

/*
**	InputCommand - Scans a command and it´s params and saves
**	them in a structure. Returns FALSE if no input.
*/
int InputCommand(tScan * scan);

/*
**	LegalCommand - Receives scanned command, sets command reference
**	in structure,retuns TRUE if valid.
*/
int LegalCommand(tScan * scan, tCommand * command);

/*
**	LegalParams - Detects command reference and returns command validation.
*/
int LegalParams(tGame * game, tCommand * command, tScan * scan);

/*
**	LegalSweep - Validates Sweep command.
*/
int LegalSweep(tBoard * visualboard, tCommand * command, char * params);

/*
**	LegalFlag - Validates Flag/Unflag command. Saves in a struct if it's
**  a single flag or a range flag.
*/
int LegalFlag(tGame * game, tCommand * command, char * params, char task);

/*
**	LegalQuery - Validates Query command.
*/
int LegalQuery(tBoard * visualboard, tCommand * structcommand, char * params);

/*
**	LegalSave - Validates Save command. 
*/
int LegalSave(tCommand * structcommand, char * params);

/*
**	PrintQuery - Prints query resluts.
*/
void PrintQuery (tQuery * query);

/*
**	AskUndo - If you sweep a mine, AskUndo asks if you want to undo the move
**	or quit.
*/
int AskUndo(tGame * game, tUndo * undo);

/*
**	ExecCommand - Recieves a valid command and executes it.
*/
int ExecCommand(tGame *game, tCommand *command);

/*
**	getName - Gets a filename.
*/
void getName(char * name);

/*
**	PrintAll - Prints moves left, undos left, flags left, after a valid comand.
**  If the previous command was a query calls PrintQuery.   
*/
void PrintAll(tGame * game, tCommand * command);

/*
**	getCampaignName - Gets filename and checks if it ends with ".txt"
*/
void getCampaignName(tGame *game);

/*
**	setCampaign - Starts a new campaign and Plays all levels. 
*/
int setCampaign(tGame * game);

/*
**	resumeCampaign - Loads a campaign file and Plays all levels. 
*/
int resumeCampaign(tGame * game);

/*
**	CheckLegalPos - Validates a position with (row,column) format, Eg: (A,5)
**	Calls TranslateCoord, ValidRow and LegalPos.
*/
int CheckLegalPos(tBoard * structboard, tPos * pos);

/*
**	TranslateCoords - Changes rows from letter to number and decreases 
**  columns so they match backend columns. (For example column 1 for the 
**  user is column 0 for the backend)
*/
void TranslateCoords(tPos * pos);

/*
**	ValidRow - Checks if row is a letter.
*/
int ValidRow(tPos * pos);

/*
**	Quit - Asks if you want to save the game, then exits game.
*/
void Quit(tGame * game, tCommand * command);

/*
**	PrintResult - After finishing game prints Win or Lose. If you are still 
**  playing but you dont have enough moves to win prints Lose.
*/
void PrintResult(tGame * game);


int
main(void)
{
	int option;
	tGame game;
	char loadname[MAX_FILENAME_LEN];
	int valid;
	
	randomize();

	option = Menu();
	
	
	switch (option)
	{
		case 1: /* New Game */
			
			setGametypeMenu(&game);
			if (game.gametype != GAMETYPE_CAMPAIGN)
			{
				game.campaign_level = 0;
				if (setNewGame(&game))
					Play(&game);
				else
					printf("%s%s%s\n", KERR, MEM_ERR, KDEF);
			}
			else
			{
				game.campaign_level = 1;
				do
				{	
					if (!(valid = setCampaign(&game)))
						printf("%s%s%s\n", KERR, FILE_ERR, KDEF);
				}
				while (!valid);
			}
			break;

		case 2:	/* Load */
			do
			{
				getName(loadname);
				if (!(valid = LoadFile(&game, loadname)))
					printf("%s%s%s\n", KERR, FILE_ERR, KDEF);
				else
					if (game.gametype == GAMETYPE_CAMPAIGN)
					{
						if (!(valid = resumeCampaign(&game)))
							printf("%s%s%s\n", KERR, CFILE_ERR, KDEF);
					}

			} while (!valid);

			Play(&game);
			break;		
	}

	return 0;
}

int setCampaign(tGame * game)
{
	int i, valid;
	getCampaignName(game);
	

	if (!LoadCampaign(game))
		return FALSE;

	for (i = 0; i < game->levels_amount; i++)
	{
		if ((valid = setNewGame(game)) == MALLOC_ERR)
		{
			printf("%s%s%s\n", KASK, MEM_ERR,KDEF);
			return FALSE;
		}
		else if (valid)
			Play(game);
	}
	return TRUE;
}

int resumeCampaign(tGame * game)
{
	int i, valid;
	int campaign_rows;
	int campaign_columns;

	if (!LoadCampaign(game))
		return FALSE;
	campaign_rows = game->campaign[game->campaign_level-1].rows;
	campaign_columns = game->campaign[game->campaign_level-1].columns;
	
	if (campaign_rows != game->hiddenboard.rows || campaign_columns != game->hiddenboard.columns)
		return FALSE;

	Play(game);
	
	for (i = game->campaign_level; i < game->levels_amount; i++)
	{
		if ((valid = setNewGame(game)) == MALLOC_ERR)
		{
			printf("%s%s%s\n", KERR, MEM_ERR, KDEF);
			return FALSE;
		}
		else if (valid)				
			Play(game);
	}
	return TRUE;
}

int Menu(void)
{
	int option;
	
	do
	{	
		printf("%s1. Juego Nuevo\n"  
			   "2. Recuperar Juego Grabado\n"
			   "3. Terminar%s\n", KASK, KDEF);
	
		option = getint("%sElija una opcion: %s", KASK, KDEF);

		if (option > 3 || option < 1)
			printf("%s%s%s\n", KERR, OPT_ERR, KASK);

	} while(option > 3 || option < 1);
	
	return option;

}

void setGametypeMenu(tGame * game)
{
	int option;

	do
	{
		printf( "%s1. Juego individual sin limite de movimientos\n"
				"2. Juego individual con limite de movimientos\n"
				"3. Juego por campaña%s\n", KASK, KDEF);

		option = getint("%sElija una opcion: %s\n", KASK, KDEF);
		
		if (option > 3 || option < 1)
			printf("%s%s%s\n", KERR, OPT_ERR, KDEF);
		
	} while(option > 3 || option < 1);

	game->gametype = option-1;

	return;
}


int setNewGame(tGame * game)
{

	if (game->gametype != GAMETYPE_CAMPAIGN)
	{
		getDim(game);
		getLevel(game);
	}
	else
	{
		int campaign_rows = game->campaign[game->campaign_level-1].rows;
		int campaign_columns = game->campaign[game->campaign_level-1].columns;
		int campaign_level = game->campaign[game->campaign_level-1].level;

		if (campaign_rows > MAX_ROWS || campaign_columns > MAX_COLUMNS)
		{	
			printf("El nivel actual posee un tablero muy grande, se pasara al proximo nivel");
			game->campaign_level++;
			return FALSE;
		}
		
		game->visualboard.rows = game->hiddenboard.rows = campaign_rows;
		game->visualboard.columns = game->hiddenboard.columns = campaign_columns;
		game->level = campaign_level;
	}
	setGameMinesNumber(game);
 	game->undos = get_undos(game->level);
 	
 	/* Moves */
 	if (game->gametype == GAMETYPE_INDIVIDUAL_NOLIMIT)
 		game->moves = UNLIMITED_MOVES;
 	else
 		game->moves = get_moves(game->undos, game->mines);
 	
 	game->mines_left = game->mines;
 	game->sweeps_left = (game->visualboard.rows * game->visualboard.columns) - game->mines;
 	game->flags_left = game->mines;
 	game->gamestate = GAMESTATE_DEFAULT; 

 	if (!CreateHiddenVisualBoard(game))
 		return MALLOC_ERR;

 	/* Ready to play */
 	return TRUE;
}


void getCampaignName(tGame *game)
{
	char name[MAX_FILENAME_LEN];
	int valid;
	int len;

	do
	{
		printf("%sEscriba nombre de campaña, el archivo debe terminar en .txt%s\n", KASK, KDEF);
		
		if (gets(name) == NULL)
			valid = FALSE;
		else
		{	
			len = strlen(name);
			if (len < FORMAT_LENGTH + 1)
				valid = FALSE;
			else
			{
				if (strstr(&(name[len-FORMAT_LENGTH]), FILE_FORMAT) == NULL)
					valid = FALSE;
				else
					valid = TRUE;
			}
		}
	} while (!valid);
	
	strcpy(game->campaign_name, name);	
	return;
}


void getDim(tGame * game)
{
	int rowsaux, colaux;
	
	do
	{
		rowsaux = getint("%sIngrese FILAS, minimo 5 y maximo 19:%s\n", KASK, KDEF);
	} while (rowsaux < 5 || rowsaux > 19);

	do
	{
		colaux = getint("%sIngrese COLUMNAS, minimo 5 y maximo 19:%s\n", KASK, KDEF);
	} while (colaux < 5 || colaux > 19);

	game->visualboard.rows = game->hiddenboard.rows = rowsaux;
	game->visualboard.columns = game->hiddenboard.columns = colaux;

	return;
}


void getLevel(tGame * game)
{	
	int level;
	int can_nightmare = ((game->visualboard.rows * game->visualboard.columns) >= 100);
	int can = FALSE;

	do
	{
		level = getint(	"%sIngrese dificultad:\n"
						"%s1.Facil\n"
						"%s2.Medio\n"
						"%s3.Dificil\n"
						"%s4.Pesadilla%s\n", KREF, KSWP, KFLG, KREF, KMIN, KDEF);
	
		if (level == NIGHTMARE)
			can_nightmare? (can = TRUE) : (can = FALSE);
		else
			can = TRUE;

		if (!can)
			printf("%s%s%s\n", KERR, NIGHTMARE_ERR, KDEF);

	} while(level < EASY || level > NIGHTMARE || !can);

	game->level = level;

	return;
}


void PrintBoard(tBoard * structboard)
{
	int i, j;
	int rows = structboard->rows;
	int columns = structboard->columns;

	char ** board = structboard->board;

	putchar('\t');
	
	printf("%s", KREF);	
	for(i = 0; i < columns; i++)
		printf("%d\t", i+1);

	printf("%s\n", KDEF);
	
	for(i = 0; i < rows; i++)
	{
		printf("%s%c%s\t", KSWP, toupperalpha(i), KDEF);

		for (j = 0; j < columns; j++)
			printf("%s%c%s\t", COLORBOARD, board[i][j], KDEF);

		putchar('\n');
	}
}


void Play(tGame * game)
{	
	int legal;

	tScan scan;
	tCommand command;
	command.undo.can_undo = FALSE;
	command.undo.undo_error = FALSE;
	command.undo.lastboard.rows = game->visualboard.rows; 
	command.undo.lastboard.columns = game->visualboard.columns;
	CreateBoard(&command.undo.lastboard);;

	do
	{
		PrintAll(game, &command);
		
		do
		{	
			if ((legal = InputCommand(&scan)))
			{	
				if((legal = LegalCommand(&scan, &command)))
					if (command.command_ref < 5) /*All commands but quit or undo*/
						legal = LegalParams(game, &command, &scan);
			}

		if (!legal)
			printf("%s%s%s\n", KERR, COMMAND_ERR, KDEF);

		} while (!legal);

		ExecCommand(game, &command);
		CheckGameState(game);
	} while(game->gamestate == GAMESTATE_DEFAULT);

	PrintResult(game);

	freeBoard(game->hiddenboard.board, game->hiddenboard.rows);
	freeBoard(game->visualboard.board, game->hiddenboard.rows);
	return;
}

int InputCommand(tScan * scan)
{
	int i, j, k;
	int found_space = FALSE;
	int endfor = FALSE;
	char * rinput; /*Result input*/

	/* Command, params and 2 for space and '\n' */
	char input[MAX_COMMAND_LEN + MAX_PARAMS_LEN + 2]; 
	
	printf("%sIntroducir un comando: %s", KASK, KDEF);

	rinput = fgets(input, MAX_COMMAND_LEN + MAX_PARAMS_LEN + 2, stdin);
	
	if (rinput == NULL)
		return FALSE;

	/* Exit if no scan */
	if (input[0] == '\n')
		return FALSE;

	for (i = 0, j = 0, k = 0; input[i] && !endfor; i++)
	{
		if (input[i] == ' ')
		{	
			if (!found_space && input[i+1] != '\n')
				found_space = TRUE;
			else
				endfor = TRUE;
		}

		else if (!found_space)
			scan->command[j++] = input[i];
		else
			scan->params[k++] = input[i];
	}

	scan->command[j] = '\0';
	scan->params[k]	 = '\0';

	return TRUE;
}

int LegalCommand(tScan * scan, tCommand * command)
{
	/* Hardcoded commands respecting the COMMAND_ defines order */
	static char * commandlist[] = {"s", "flag", "unflag", "query", "save", "quit\n", "undo\n"};
	
	char found = FALSE;
	int commandindex;

	for (commandindex = 0; commandindex < COMMANDS_NUMBER && !found; commandindex++)
	{
		if (!strcmp(scan->command, commandlist[commandindex]))
		{
			/* Assigns the command reference respecting the 
			   COMMAND_ defines order */
			command->command_ref = commandindex;
			found = TRUE;
		}
	}

	return found;
}


int LegalParams(tGame * game, tCommand * structcommand, tScan * scan)
{	
	switch(structcommand->command_ref)
	{
		case COMMAND_SWEEP:
			return LegalSweep(&game->visualboard, structcommand, scan->params);
		
		case COMMAND_FLAG:
			return LegalFlag(game, structcommand, scan->params, DO_FLAG);

		case COMMAND_UNFLAG:
			return LegalFlag(game, structcommand, scan->params, DO_UNFLAG);

		case COMMAND_QUERY:	
			return LegalQuery(&game->visualboard, structcommand, scan->params);
		
		case COMMAND_SAVE:
			return LegalSave(structcommand, scan->params);
	}
	return TRUE;
}

int LegalSave(tCommand * structcommand, char * params)
{
	params[strlen(params)-1] = '\0';
	
	if (! *params)
		return FALSE;
	strcpy(structcommand->save_filename, params);
	return TRUE;
}

int LegalSweep(tBoard * visualboard, tCommand * structcommand, char * params)
{
	tPos aux;
	char new_line;
	char legal = TRUE;
	char i_scan;

	if (sscanf(params, "(%c,%d)%c", &i_scan, &aux.j, &new_line) != 3)
		return FALSE;

	if (new_line != '\n')
		return FALSE;
	/*	Used &i_scan because we cannot use int pointer,
	**	we use char pointer and cast it instead
	*/
	aux.i = (int)i_scan;

	if (!CheckLegalPos(visualboard, &aux))
		legal = FALSE;
	/* If there's an '&' or '-' on the visual board return false*/
	else if (visualboard->board[aux.i][aux.j] != VISUAL_UNFLAGGED)  
		legal = FALSE;

	if (legal){
		structcommand->sweep.i = aux.i;
		structcommand->sweep.j = aux.j;
	}

	return legal;

}

int
LegalFlag(tGame * game, tCommand * structcommand, char * params, char task) /*No valida si ya esta flaggeado*/
{
	int i;
	char legal = TRUE;
	char range_count = 0;
	tPos f_aux;
	tPos l_aux;
	char f_scan;
	char l_scan;
	char new_line;

	/* Range flag*/
	if (sscanf(params, "(%c,%d:%c,%d)%c", &f_scan, &f_aux.j, &l_scan, &l_aux.j, &new_line) == 5)
	{	
		if (new_line != '\n')
			return FALSE;

		f_aux.i = (int)f_scan;
		l_aux.i = (int)l_scan;
		
		if (!CheckLegalPos(&game->visualboard, &f_aux) || !CheckLegalPos(&game->visualboard, &l_aux))
			legal = FALSE;
		/*Check if it's a row or a column and that the first position comes
		before the last position*/
		else if (f_aux.i == l_aux.i)
		{	
			if(f_aux.j > l_aux.j)
				legal = FALSE;
			else
				structcommand->flag.is_row = TRUE;
		}	
		else if (f_aux.j == l_aux.j)
		{	
			if(f_aux.i > f_aux.i)
				legal = FALSE;
			else
				structcommand->flag.is_row = FALSE;
		}	
		else
			legal = FALSE;
		/*Check how many elements you are flagging*/
		if(legal && structcommand->flag.is_row)
		{
			for(i=f_aux.j; i<=l_aux.j; i++)
			{	
				if ((task == DO_FLAG) && (game->visualboard.board[f_aux.i][i] == VISUAL_UNFLAGGED))
					range_count++;
				else if( (task == DO_UNFLAG) && (game->visualboard.board[f_aux.i][i] == VISUAL_FLAGGED))
					range_count++;
			}	
		}
		else if (legal)
		{
			for(i=f_aux.i; i<=l_aux.i; i++)
			{	
				if ((task == DO_FLAG) && (game->visualboard.board[i][f_aux.j] == VISUAL_UNFLAGGED))
					range_count++;
				else if( (task == DO_UNFLAG) && (game->visualboard.board[i][f_aux.j] == VISUAL_FLAGGED))
					range_count++;
			}		
		}	
		/*Check if there's nothing to flag in the range
		or if you don't have enough moves or flags left*/
		if (legal)
		{
			if (range_count == 0)
				legal = FALSE;
			else if ( (task == DO_FLAG) && (range_count>game->flags_left) )
				legal = FALSE;
			else if( (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT && range_count>game->moves))
				legal = FALSE;	
		}
		if (legal)
		{	
			structcommand->flag.is_range = TRUE;
			structcommand->flag.first_pos.i = f_aux.i;
			structcommand->flag.first_pos.j = f_aux.j;
			structcommand->flag.last_pos.i 	= l_aux.i;
			structcommand->flag.last_pos.j 	= l_aux.j;
		}

	}
	/*Single Flag*/
	else if (sscanf(params, "(%c,%d)%c", &f_scan, &f_aux.j, &new_line) == 3)
	{
		if (new_line != '\n')
			return FALSE;	
		
		f_aux.i = (int)f_scan;

		if (!CheckLegalPos(&game->visualboard, &f_aux))
			legal = FALSE;

		/*Check the element you are flagging is valid and you have enough flags left*/
		else if ( (task == DO_FLAG) && (game->visualboard.board[f_aux.i][f_aux.j] != VISUAL_UNFLAGGED))
			legal = FALSE;
		else if( (task == DO_UNFLAG) && (game->visualboard.board[f_aux.i][f_aux.j] != VISUAL_FLAGGED))
			legal = FALSE;
		else if( task == DO_FLAG && game->flags_left == 0)
			legal = FALSE;
		
		if (legal)
		{	
			structcommand->flag.is_range 	= FALSE;
			structcommand->flag.first_pos.i = f_aux.i;
			structcommand->flag.first_pos.j = f_aux.j;
		}			
	}
	else
		legal = FALSE; 	
	
	return legal;
}

int
LegalQuery(tBoard * visualboard, tCommand * structcommand, char * params)
{
	char index_row;
	int index_column;
	char new_line;
	char legal = TRUE;

	/*Query Column*/
	if(sscanf(params, "%d%c", &index_column, &new_line) == 2)
	{	
		/*Translate the column to backend*/
		index_column--;

		if (new_line != '\n')
			return FALSE;
		/*Check the column is inside the board*/
		if (index_column < 0 || index_column >= visualboard->columns)
			legal = FALSE;
		if (legal)
		{
			structcommand->query.is_row = FALSE;
			structcommand->query.index = index_column;
		}	
	}
	/*Query Row*/
	else if (sscanf(params, "%c%c", &index_row, &new_line) == 2 )
	{	
		/*Translate the row to backend*/
		index_row = get_row_pos_byref(index_row);
		
		if (new_line != '\n')
			return FALSE;
		
		/*Check the row was a letter*/
		if (!isupper('A' + index_row))
			legal = FALSE;
		
		/*Check the row is inside the board*/
		else if (index_row < 0 || index_row >= visualboard->rows)
			legal = FALSE;
		
		if(legal)
		{	
			structcommand->query.is_row = TRUE;
			structcommand->query.index = index_row;
		}
	}
	else
		legal = FALSE;
	
	return legal;

}

void PrintQuery(tQuery * query)
{	
	int i;
	int dim = query->results.dim;

	printf("Query: ");

	if (dim)
	{
		for (i = 0; i < dim; i++)
			printf("%d%s", query->results.array[i], (i != (dim-1))? " - ": "\n");	
	}else
		printf("0\n");
		
	return;
}

int AskUndo(tGame * game, tUndo * undo)
{
	char input[MAX_COMMAND_LEN];
	char * pinput;

	int wasundo = FALSE;
	int wasquit = FALSE;

	PrintBoard(&game->visualboard);

	printf("%sPerdiste! ¿Hacer Undo? (Ingresar undo o quit)%s\n", KASK, KDEF);

	do{
		pinput = fgets(input, MAX_COMMAND_LEN, stdin);
		if (pinput != NULL)
		{
			wasundo = (strcmp(input,"undo\n") == 0);
			wasquit = (strcmp(input,"quit\n") == 0);
		}

		if (!wasundo && !wasquit)
			printf("%s%s%s\n", KERR, ASKUNDO_ERR, KDEF);
	}
	while ( (!wasundo && !wasquit) || (pinput == NULL));


	if (wasundo)
	{
		Undo(game, undo);
		game->undos--;
		if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
			game->moves--;
		return TRUE;
	}
	else
	{
		game->gamestate = GAMESTATE_LOSE;
		return FALSE;
	}

}

void PrintAll(tGame * game, tCommand * command)
{
	DELSHELL();

	PrintBoard(&game->visualboard);
	
	printf("%s", KMSG);
	if (game->gametype)
		printf("Movimientos restantes: %d\n", game->moves);
	
	printf("Undos restantes: %d\n", game->undos);
	printf("Flags restantes: %d\n", game->flags_left);
	
	if (command->command_ref == COMMAND_QUERY)
	{
		PrintQuery(&command->query);
		free(command->query.results.array);
	}
	else if (command->command_ref == COMMAND_UNDO && \
			(command->undo.undo_error == TRUE))
	{
		printf("%s%s\n", KERR, UNDO_ERR);
		command->undo.undo_error = FALSE;
	}
	printf("%s", KDEF);
	return;
}

void Quit(tGame * game, tCommand * command)
{
	char * pinput;
	char input[5];
	char savename[MAX_FILENAME_LEN];
	int yes = FALSE;
	int no = FALSE;

	printf("%sDesea guardar la partida? (Ingrese si o no)%s\n", KASK, KDEF);
	do
	{
		pinput = fgets(input, 5, stdin);
		if (pinput != NULL)
		{
			yes = strcmp(input, "si\n") == 0;
			no = strcmp(input, "no\n") == 0;
		}
		if (!yes && !no)
			printf("%sIngresar si o no.%s\n", KASK, KDEF);

	}while((!yes && !no) || (pinput == NULL));

	if (yes)
	{
		getName(savename);
		WriteSaveFile(game, savename);
	}
	else
		exit(0);	
}

int ExecCommand(tGame *game, tCommand * command)
{
	int i = command->command_ref;
	int res;

	switch (i)
	{
		case COMMAND_SWEEP:
			res = Sweep(game, command);
			if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
				game->moves--;
			break;
		
		case COMMAND_FLAG:
			if (!(command->flag.is_range))
			{	
				res = DoFlagUnflag(game, command, DO_FLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves--;
			}	
			else
			{
				res = FlagRange(game, command, DO_FLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves-= res;
			}	
			break;
		
		case COMMAND_UNFLAG:
			if (!(command->flag.is_range))
			{
				res = DoFlagUnflag(game, command, DO_UNFLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves--;
			}	
			else
			{
				res = FlagRange(game, command, DO_UNFLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves-= res;
			}	
			break;
		
		case COMMAND_QUERY:
			res = Query(&game->hiddenboard, command);
			break;

		case COMMAND_SAVE:
			res = WriteSaveFile(game, command->save_filename);
			break;
		
		case COMMAND_QUIT:
			Quit(game, command);
			exit(0);
			break;
		
		case COMMAND_UNDO:
			if (command->undo.can_undo && game->undos)
			{
				Undo(game, &command->undo);
				game->undos--;
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves--;
			}
			else
				command->undo.undo_error = TRUE;
			break;


	}

	if (res == SWEEP_MINE && i == COMMAND_SWEEP)
	{
		if (game->undos)
		{	
			if (game->gametype == GAMETYPE_INDIVIDUAL_NOLIMIT)
				AskUndo(game, &command->undo);
			else if (game->moves)
				AskUndo(game, &command->undo);
			else
				game->gamestate = GAMESTATE_LOSE;
		}
		else
			game->gamestate = GAMESTATE_LOSE;
	}	
	return res;
}

void getName(char * name)
{	
	int res = 0;
	char fmt[6];
	sprintf(fmt, "%%%ds", MAX_FILENAME_LEN);
	do
	{
		printf("%sIntroducir nombre de archivo%s\n", KASK, KDEF);
		res = scanf(fmt, name);
	
	} while(!res);

	DELBFF();

	return;

}

void TranslateCoords(tPos * pos)
{
	pos->i = get_row_pos_byref(pos->i);
	pos->j--;
}

int ValidRow(tPos * pos)
{
	return isupper('A' + pos->i);
}

int CheckLegalPos(tBoard * structboard, tPos * pos)
{
	int res;
	TranslateCoords(pos);
	res = ValidRow(pos);
	res = res && LegalPos(structboard, pos);
	return res;
}

void PrintResult(tGame * game)
{
	switch (game->gamestate)
	{
		case GAMESTATE_WIN:
			PrintBoard(&game->visualboard);
			printf ("%sGanaste!%s\n", KEXC, KDEF);
			if (game->gametype == GAMETYPE_CAMPAIGN)
				game->campaign_level++;
			break;

		case GAMESTATE_CANTWIN:
			printf("%s%s%s\n", KEXC, NOMOVES_ERR, KDEF);
			game->gamestate = GAMESTATE_LOSE;
		case GAMESTATE_LOSE:
			PrintBoard(&game->hiddenboard);
			printf("%sPerdiste!%s\n", KEXC, KDEF);
			break;	
	}
}
