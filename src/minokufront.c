#include "minokubackend.h"

int
main(void)
{
	//ToDo: tidy this, replace cases with constants
	int option;
	tGame game;
	char loadname[MAX_FILENAME_LEN];
	
	randomize();
	option = Menu();
	
	switch (option)
	{
		case 1: /* New Game */
			if (setNewGame(&game)){
			/* DEBUG 
			printf("hidden F C : %d %d\n", game.hiddenboard.rows, game.hiddenboard.columns);
			printf("visual F C : %d %d\n", game.visualboard.rows, game.visualboard.columns);
			printf("gametype: %d\n", game.gametype);
			printf("level: %d\n", game.level);
			printf("moves: %d\n", game.moves);
			printf("undos: %d\n", game.undos);
			printf("mines: %d\n", game.mines);
			printf("mines_left: %d\n", game.mines_left);
			printf("sweeps left: %d\n", game.sweeps_left);	
			*/
			PrintBoard(&game.visualboard);
			PrintBoard(&game.hiddenboard);

			Play(&game);
			}
			else
				printf("No hay suficiente memoria para seguir jugando.\n");
			
			break;

		case 2:	/* Load */
			do
			{
				getLoadName(loadname);

			} while (!LoadFile(&game, loadname));
			Play(&game);
			break;		
	}

	return 0;
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

	game->gametype = option;

	return;
}

/* Sets ONLY gameplay config */
int setNewGame(tGame * game)
{
	setGametypeMenu(game);
	getDim(game);
	getLevel(game);
	setGameMinesNumber(game);

 	game->undos = get_undos(game->level);
 	
 	if (game->gametype == GAMETYPE_INDIVIDUAL_NOLIMIT)
 		game->moves = UNLIMITED_MOVES;
 	else
 		game->moves = get_moves(game->undos, game->mines);
 	
 	game->mines_left = game->mines;
 	game->sweeps_left = (game->visualboard.rows * game->visualboard.columns) - game->mines;
 	game->flags_left = game->mines;
 	game->gamestate = GAMESTATE_DEFAULT;

 	if (!CreateHiddenVisualBoard(game))
 		return FALSE;

 	//Ready to play
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

	do
	{
		level = getint("Ingrese dificultad:\n1.Facil\n2.Medio\n3.Dificil\n4.Pesadilla\n");
	} while (level < 1 || level > 4);

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
	command.command_ref = COMMAND_UNDO;
	command.undo.lastboard.rows = game->visualboard.rows; 
	command.undo.lastboard.columns = game->visualboard.columns;
	CreateBoard(&command.undo.lastboard);
	//debug
	printf("gamestate: %d\n", game->gamestate);
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
		//printf("SCANNED NUMB: %d\n", scan.scanned_number);
		//DEBUG
		if (!legal)
			printf("Commando invalidOO\n");
		} while (!legal);
		//debug
		printf("gamestate: %d\n", game->gamestate);
		//DEBUG
		//printf("Antes de ejecutar (%s): %d\n", command.query.is_row? "Fila":"Columna", command.query.index);
		ExecCommand(game, &command);
		//debug
		printf("gamestate: %d\n", game->gamestate);
		CheckGameState(game);
		//debug
		printf("gamestate: %s\n", (game->gamestate==2)?"ACA":"ACA NO");
	} while(game->gamestate == GAMESTATE_DEFAULT);

	if (game->gamestate == GAMESTATE_WIN)
		printf ("GANO\n");
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
			printf("DEBUG: LEGALCOMMAND FOUND: REF: %d\n",command->command_ref);
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
			//return LegalSave(scan);
		return TRUE;
	
	}
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
	printf("Char: %c, Dentero: %d, Char: %c\n", i_scan, aux.j, new_line);
	if (new_line != '\n')
		return FALSE;
	
	//ToDo: Modularize
	i_scan = get_row_pos_byref(i_scan);
	aux.i = (int)i_scan;
	aux.j--;
	printf("I: %d, J: %d\n", aux.i, aux.j );
	if (!isupper('A' + aux.i)) // If Column is not a letter return false
		legal = FALSE;

	else if (!LegalPos(visualboard, &aux)) // If Position is not on the board return false
		legal = FALSE;
	
	else if (visualboard->board[aux.i][aux.j] != VISUAL_UNFLAGGED)  // If there's a '&' or '-' on the visual board return false
		legal = FALSE;

	if (legal){
		structcommand->sweep.i = aux.i;
		structcommand->sweep.j = aux.j;
		printf("AFTER SAVE STRUCT %d %d\n", structcommand->sweep.i, structcommand->sweep.j);
	}

	return legal;

}			

int
LegalFlag(tGame * game, tCommand * structcommand, char * params, char task) /*No valida si ya esta flaggeado*/
{	//Tidy
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
		//DEBUG
		printf("ISROW: %d\n", structcommand->flag.is_row);
		printf("AUX.i: %d, AUX.j: %d\n", f_aux.j, l_aux.j);
		printf("FilaI: %d, FilaF: %d\n", f_aux.i, l_aux.i);

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
		
		if (legal && (range_count == 0 || range_count>game->moves))
			legal = FALSE;
		
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

	for (i = 0; i < dim; i++)
		printf("%d%s", query->results.results[i], (i != (dim-1))? " - ": "");	
	putchar('\n');

	return;
}

int AskUndo(tGame * game, tUndo * undo)
{
	char fmt[6]; //ToDo: Constant
	char input[MAX_COMMAND_LEN];

	int wasundo;
	int valid;

	PrintBoard(&game->visualboard);
	sprintf(fmt, "%%%ds",MAX_COMMAND_LEN);

	printf("Perdiste! ¿Hacer Undo? (Ingresar undo o quit\n");
	
	do
	{
		scanf(fmt, input);

		valid = (strcmp(input, "quit") == 0 || (wasundo = !strcmp(input, "undo")) == 0);

		if (!valid)
			printf ("Ingresar quit o undo.\n");

	} while(!valid);

	if (wasundo)
	{
		//ToDo: Call Undo
		return TRUE;
	}
	else
	{
		game->gamestate = GAMESTATE_LOSE;
		return FALSE;
	}

}
