#include "minokubackend.h"

/*
**		Function prototypes (front) 
**		TODO: Put this in frontend. Backend does not need this.
*/
int Menu(void);
void setGametypeMenu(tGame * game);
void PrintBoard(tBoard * structboard);
void getLevel(tGame * game);
void getDim(tGame * game);
int setNewGame(tGame * game);
void Play(tGame * game);
int LegalCommand(tScan * scan, tCommand * command);
int InputCommand(tScan * scan);
int CreateHiddenVisualBoard(tGame * game); //ToDo: Change name
int LegalParams(tGame * game, tCommand * command, tScan * scan);
int LegalSweep(tBoard * visualboard, tCommand * command, char * params);
int LegalFlag(tGame * game, tCommand * command, char * params, char task);
int LegalQuery(tBoard * visualboard, tCommand * structcommand, char * params);
void PrintQuery (tQuery * query);
int AskUndo(tGame * game, tUndo * undo);
int ExecCommand(tGame *game, tCommand *command);
void getName(char * name);
//ToDo Remove
void PrintearTodo(tGame * game);
void getCampaignName(tGame *game);

int
main(void)
{
	//ToDo: tidy this, replace cases with constants
	int option;
	tGame game;
	char loadname[MAX_FILENAME_LEN];
	
	randomize();

	option = Menu();
	game.campaign_level = 0;
	
	switch (option)
	{
		case 1: /* New Game */
			setGametypeMenu(&game);
			if (game.gametype != GAMETYPE_CAMPAIGN)
			{
				if (setNewGame(&game))
				{
					PrintearTodo(&game);
					PrintBoard(&game.hiddenboard);
					Play(&game);
				}
				else
					printf("No hay suficiente memoria para seguir jugando.\n");
			}
			else
			{
				if (!setCampaign(&game))
					printf("Error en la campaña\n");
			}
			break;

		case 2:	/* Load */
			do
			{
				getName(loadname);

			} while (!LoadFile(&game, loadname));
			if (game.gametype == GAMETYPE_CAMPAIGN)
			{
				if (!setCampaign(&game))
				{
					printf("campaña invalida\n");
				}
			}
			else
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
			printf("No hay memoria\n");
			return FALSE;
		}
		else if (valid)
		{					
			PrintearTodo(game);
			PrintBoard(&game->visualboard);
			PrintBoard(&game->hiddenboard);
			Play(game);
		}
	}
	return TRUE;
}

int resumeCampaign(tGame * game)
{
	int i, valid;
	int campaign_rows = game->campaign[game->campaign_level].rows;
	int campaign_columns = game->campaign[game->campaign_level].columns;

	if (!LoadCampaign(game))
		return FALSE;

	if (campaign_rows != game->hiddenboard.rows || campaign_columns != game->hiddenboard.columns)
		return FALSE;

	Play(game);
	
	for (i = game->campaign_level; i < game->levels_amount; i++)
	{
		if ((valid = setNewGame(game)) == MALLOC_ERR)
		{
			printf("No hay suficiente memoria para seguir jugando.\n");
			return FALSE;
		}
		else if (valid)
		{					
			PrintearTodo(game);
			PrintBoard(&game->visualboard);
			PrintBoard(&game->hiddenboard);
			Play(game);
		}
	}
	return TRUE;
}
int
Menu(void)
{
	int option;
	
	do
	{	
		printf("1. Juego Nuevo\n"  
			   "2. Recuperar Juego Grabado\n"
			   "3. Terminar\n");
	
		option = getint("Elija una opcion: ");

		if (option > 3 || option < 1)
			printf("Ingrese una opcion valida.\n");

	} while(option > 3 || option < 1);
	
	return option;

}

void
setGametypeMenu(tGame * game)
{
	int option;

	do
	{
		printf( "1.1 Juego individual sin limite de movimientos\n"
				"1.2 Juego individual con limite de movimientos\n"
				"1.3 Juego por campaña\n");

		option = getint("Elija una opcion: \n");
		
		if (option > 3 || option < 1)
			printf("Ingrese una opcion valida.\n");
		
	} while(option > 3 || option < 1);

	game->gametype = option-1;

	return;
}

/*
**	setNewGame - Sets All the necesary info to play 
**	in game structure.  
*/
int setNewGame(tGame * game)
{

	if (game->gametype != GAMETYPE_CAMPAIGN)
	{
		getDim(game);
		getLevel(game);
	}
	else
	{
		int campaign_rows = game->campaign[game->campaign_level].rows;
		int campaign_columns = game->campaign[game->campaign_level].columns;
		int campaign_level = game->campaign[game->campaign_level].level;

		if campaign_rows > MAX_ROWS || campaign_columns > MAX_COLUMNS)
			return FALSE;
		
		game->visualboard.rows = game->hiddenboard.rows = campaign_rows;
		game->visualboard.columns = game->hiddenboard.columns = campaign_columns;
		game->level = campaign_level;
	}
	setGameMinesNumber(game);

 	game->undos = get_undos(game->level);
 	
 	//Moves
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

 	//Ready to play
 	return TRUE;
}

/*
**	getCampaignName - Gets name and checks if it ends with ".txt"
*/
void getCampaignName(tGame *game)
{
	char name[MAX_FILENAME_LEN];
	int valid;
	int len;

	do
	{
		printf("Escriba nombre de campaña\n");
		
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
				valid = TRUE;
			}
		}
	} while (!valid);
	
	strcpy(game->campaign_name, name);	
	return;
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

void getDim(tGame * game)
{
	int rowsaux, colaux;
	
	do
	{
		rowsaux = getint("Ingrese FILAS, minimo 5 y maximo 19:\n");
	} while (rowsaux < 5 || rowsaux > 19);

	do
	{
		colaux = getint("Ingrese COLUMNAS, minimo 5 y maximo 19:\n");
	} while (colaux < 5 || colaux > 19);

	game->visualboard.rows = game->hiddenboard.rows = rowsaux;
	game->visualboard.columns = game->hiddenboard.columns = colaux;

	return;
}

void getLevel(tGame * game){
	
	int level;
	int can_nightmare = ((game->visualboard.rows * game->visualboard.columns) >= 100);
	int can = FALSE;

	do
	{
		level = getint("Ingrese dificultad:\n1.Facil\n2.Medio\n3.Dificil\n4.Pesadilla\n");
	
		if (level == 4)
		{
			if (can_nightmare){
				can = TRUE;
			}
			else
				can = FALSE;
		}else
			can = TRUE;

		if (!can)
			printf("No es posible elegir pesadilla con menos de 100 casilleros.\n");

	} while(level < 1 || level > 4 || !can);

	game->level = level;

	return;
}

void PrintBoard(tBoard * structboard)
{//ToDo: Tidy this
	int i, j;
	int rows = structboard->rows;
	int columns = structboard->columns;

	char ** board = structboard->board;

	putchar('\t');
	
	for(i = 0; i < columns; i++)
		printf("%d\t", i+1);

	putchar('\n');
	
	for(i = 0; i < rows; i++)
	{
		printf("%c\t", toupperalpha(i));

		for (j = 0; j < columns; j++)
			printf("%c\t", board[i][j]);

		putchar('\n');
	}
}

void Play(tGame * game)
{	
	int legal;

	tScan scan;
	tCommand command;
	command.undo.can_undo = FALSE;
	command.undo.lastboard.rows = game->visualboard.rows; 
	command.undo.lastboard.columns = game->visualboard.columns;
	CreateBoard(&command.undo.lastboard);
	do
	{
		PrintBoard(&game->visualboard); //ToDo: Print all
		
		do
		{	
			if ((legal = InputCommand(&scan)))
			{	
				if((legal = LegalCommand(&scan, &command)))
					if (command.command_ref < 5)
						legal = LegalParams(game, &command, &scan);
				
			}
		//DEBUG
		if (!legal)
			printf("Commando invalidOO\n");
		} while (!legal);
		ExecCommand(game, &command);
		printf("Despues de Ejecutar:\n");
		PrintearTodo(game);
		CheckGameState(game);
		printf("Despues de check:\n");
		PrintearTodo(game);
	} while(game->gamestate == GAMESTATE_DEFAULT);

	if (game->gamestate == GAMESTATE_WIN)
	{
		printf ("GANO\n");
		if (game->gametype == GAMETYPE_CAMPAIGN)
			game->campaign_level++;
	}
	else
		printf("PERDIO\n");

	return;
}

/*
**		InputCommand - Scans a command and it´s params
**		and saves them in a structure. Returns FALSE if
**		no input. 
*/

int InputCommand(tScan * scan)
{	//ToDo: tidy
	int i, j, k;
	int found_space = FALSE;
	int endfor = FALSE;
	char * rinput; //Result input
	char input[MAX_COMMAND_LEN + MAX_PARAMS_LEN + 2];
	
	printf("Introducir un comando: ");

	rinput = fgets(input, MAX_COMMAND_LEN + MAX_PARAMS_LEN + 2, stdin); //+2 0 and blank
	
	if (rinput == NULL)
		return FALSE;

	// Exit if no scan
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
	scan->params[k] = '\0';
	//DEBUG
	//puts(scan->command); puts(scan->params);

	return TRUE;
}

/*
**		LegalCommand - Receives scanned command, sets 
**		command reference in structure,retuns TRUE 
**		if valid.
*/

int LegalCommand(tScan * scan, tCommand * command)
{
	//ToDo: Reduce mem access in for
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
			//DEBUG
			printf("Command Ref: %d\n",command->command_ref);
		}
	}

	if (!found)
		return FALSE;

	return TRUE;
}

int
LegalParams(tGame * game, tCommand * structcommand, tScan * scan)
{	
	switch(structcommand->command_ref)
	{
		case COMMAND_SWEEP:
			//printf("EN el switch: %s\n", scan->params);
			return LegalSweep(&game->visualboard, structcommand, scan->params);
		
		case COMMAND_FLAG:
			return LegalFlag(game, structcommand, scan->params, DO_FLAG);

		case COMMAND_UNFLAG:
			return LegalFlag(game, structcommand, scan->params, DO_UNFLAG);

		case COMMAND_QUERY:	
			return LegalQuery(&game->visualboard, structcommand, scan->params);
		
		case COMMAND_SAVE:
			return LegalSave(structcommand, scan->params);
		return TRUE;
	
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

int
LegalSweep(tBoard * visualboard, tCommand * structcommand, char * params)
{//todo: tidy
	tPos aux;
	char new_line;
	char * closepos;
	char legal = TRUE;
	char i_scan;
	int i;
	int auxnum;

	if (sscanf(params, "(%c,%d)%c", &i_scan, &aux.j, &new_line) != 3)
		return FALSE;
	if (new_line != '\n')
		return FALSE;
	
	//ToDo: Modularize
	i_scan = get_row_pos_byref(i_scan);
	aux.i = (int)i_scan;
	aux.j--;
	if (!isupper('A' + aux.i)) // If Column is not a letter return false
		legal = FALSE;

	else if (!LegalPos(visualboard, &aux)) // If Position is not on the board return false
		legal = FALSE;
	
	else if (visualboard->board[aux.i][aux.j] != VISUAL_UNFLAGGED)  // If there's a '&' or '-' on the visual board return false
		legal = FALSE;

	if (legal){
		structcommand->sweep.i = aux.i;
		structcommand->sweep.j = aux.j;
	}

	return legal;

}			

int
LegalFlag(tGame * game, tCommand * structcommand, char * params, char task) /*No valida si ya esta flaggeado*/
{	//ToDo Borraar comment
	//Tidy
	//int fposi, fposj, lposi, lposj;
	int i;
	char legal = TRUE;
	char range_count = 0;
	tPos f_aux;
	tPos l_aux;
	char f_scan;
	char l_scan;
	char new_line;

	// Checks if range is legal
	if (sscanf(params, "(%c,%d:%c,%d)%c", &f_scan, &f_aux.j, &l_scan, &l_aux.j, &new_line) == 5)
	{	
		if (new_line != '\n')
			return FALSE;
		f_scan = get_row_pos_byref(f_scan);
		f_aux.i = f_scan;
		l_scan = get_row_pos_byref(l_scan);
		l_aux.i = l_scan;
		f_aux.j--;
		l_aux.j--;

		// Syntax check
		if (!isupper('A' + f_aux.i)|| !isupper('A' + l_aux.i))
			legal = FALSE;
		
		else if (!LegalPos(&game->visualboard, &f_aux) || !LegalPos(&game->visualboard, &l_aux))
			legal = FALSE;

		// Legal move check
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
		//If there nothing to flag in the range or you have enough moves or flags left
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
	else if (sscanf(params, "(%c,%d)%c", &f_scan, &f_aux.j, &new_line) == 3)
	{
		if (new_line != '\n')
			return FALSE;	
		f_scan = get_row_pos_byref(f_scan);
		f_aux.i = f_scan;
		f_aux.j--;
		
		if (!isupper('A' + f_aux.i))
			legal = FALSE;

		else if(!LegalPos(&game->visualboard, &f_aux))
			legal = FALSE;
		
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
	//DEBUG
	printf("LEGAL(RETURN): %d\n",legal );
	return legal;
}

int
LegalQuery(tBoard * visualboard, tCommand * structcommand, char * params)
{	//ToDo: tidy
	char index_row;
	int index_column;
	char new_line;
	char legal = TRUE;

	
	if(sscanf(params, "%d%c", &index_column, &new_line) == 2)
	{	
		index_column--;
		if (new_line != '\n')
			return FALSE;
		if (index_column < 0 || index_column >= visualboard->columns)
			legal = FALSE;
		if (legal)
		{
			structcommand->query.is_row = FALSE;
			structcommand->query.index = index_column;
		}	
	}
	else if (sscanf(params, "%c%c", &index_row, &new_line) == 2 )
	{	
		index_row = get_row_pos_byref(index_row);
		if (new_line != '\n')
			return FALSE;
		if (!isupper('A' + index_row))
			legal = FALSE;
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

void PrintQuery (tQuery * query)
{	
	int i;
	int dim = query->results.dim;

	if (dim)
	{
		for (i = 0; i < dim; i++)
			printf("%d%s", query->results.results[i], (i != (dim-1))? " - ": "\n");	
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
	int valid;

	PrintBoard(&game->visualboard);

	printf("Perdiste! ¿Hacer Undo? (Ingresar undo o quit)\n");

	do{
		pinput = fgets(input, MAX_COMMAND_LEN, stdin);
		if (pinput != NULL)
		{
			wasundo = (strcmp(input,"undo\n") == 0);
			wasquit = (strcmp(input,"quit\n") == 0);
		}

		if (!wasundo && !wasquit)
			printf("Ingresar quit o undo.");
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

void PrintearTodo(tGame * game)
{
	printf("%s", KRED);
	printf("Gametype: %d\n", game->gametype);
	printf("Moves: %d\n", game->moves);
	printf("Undos: %d\n", game->undos);
	printf("Mines: %d\n", game->mines);
	printf("Mines left: %d\n", game->mines_left);
	printf("Sweeps left: %d\n", game->sweeps_left);
	printf("Flags left: %d\n", game->flags_left);
	printf("Gamestate: %d\n", game->gamestate);
	printf("%s", KNRM);
	return;
}

void Quit(tGame * game, tCommand * command)
{
	char * pinput;
	char input[5];
	char savename[MAX_FILENAME_LEN];
	int yes = FALSE;
	int no = FALSE;
	printf("?Desea guardar la partida? (Ingrese si o no)\n");
	do
	{
		pinput = fgets(input, 5, stdin);
		if (pinput != NULL)
		{
			yes = strcmp(input, "si\n") == 0;
			no = strcmp(input, "no\n") == 0;
		}
		if (!yes && !no)
			printf("Ingresar si o no\n");

	}while((!yes && !no) || (pinput == NULL));

	if(yes)
	{	//ToDo: Call Save.
		
		getName(savename);
		WriteSaveFile(game, savename);
	}
	else
		exit(0);	
}

int ExecCommand(tGame *game, tCommand * command)
{
	//ToDo: tidy. front.
	int i = command->command_ref;
	int res; //Result

	switch (i)
	{
		case COMMAND_SWEEP:
			res = Sweep(game, &command->sweep, command);
			if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
				game->moves--;
			break;
		
		case COMMAND_FLAG:
			if (!(command->flag.is_range))
			{	
				DoFlagUnflag(game, command, DO_FLAG);
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
				DoFlagUnflag(game, command, DO_UNFLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves--;
			}	
			else
			{
				FlagRange(game, command, DO_UNFLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves-= res;
			}	
			break;
		
		case COMMAND_QUERY:
			res = Query(&(game->hiddenboard), &(command->query.results), command->query.index, command->query.is_row);
			PrintQuery(&command->query);
			free(command->query.results.results);
			break;

		case COMMAND_SAVE:
			res=WriteSaveFile(game, command->save_filename);
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
				printf("No es posible usar undo\n");
			break;


	}
	//DEBUG(Codigo)
	/*if (i == COMMAND_SWEEP || i == COMMAND_FLAG || i == COMMAND_UNFLAG)
		game->moves--;*/

	if (res == SWEEP_MINE && i == COMMAND_SWEEP)
	{
		if (game->undos)
		{	
			if (game->gametype == GAMETYPE_INDIVIDUAL_NOLIMIT)
				AskUndo(game, &command->undo);
			else if (game->moves && game->undos)
				AskUndo(game, &command->undo);
			else
			{
				game->gamestate = GAMESTATE_LOSE;
			}	
		}
		//ToDo Merge
		else
			game->gamestate = GAMESTATE_LOSE;
	}	
	return res;
}
