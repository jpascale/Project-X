#include "minokubackend.h"

int
main(void)
{
	//ToDo: tidy this, replace cases with constants
	int option;
	tGame game;
	
	randomize();
	option = Menu();
	
	switch (option)
	{
		case 1: /* New Game */
			if (setNewGame(&game))
				Play(&game);
			else
				printf("No hay suficiente memoria para seguir jugando.");
			
			break;

		case 2:	/* Load */
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
	game->visualboard.columns = game->visualboard.columns = colaux;

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
	char won = FALSE; //ToDo: put in tGame
	char end = FALSE;
	tCommand command;

	do
	{
		do
		{
			int result = InputCommand(&command);

			if (!result)
				printf("Comando no válido.\n");

		} while (!result)

	} while(!won && !end);

	return;
}

/*
**		InputCommand - Scans a command and it´s params
**		and saves them in a structure. Returns FALSE if
**		no input. 
*/

int InputCommand(tScan * scan)
{	
	int scanned_number;

	// Scanf formatting
	char fmt[13]; //ToDo: Constant
	sprintf(fmt, "%%%ds %%%ds", MAX_COMMAND_LEN, MAX_PARAMS_LEN);

	printf("Introducir un comando:\n");

	scanned_number = scanf(fmt, scan->command, scan->params);
	DELBFF();

	if (!scanned_number)
		return FALSE;

	scan->scanned_number = scanned_number;

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
	static char * commandlist[] = {"s", "flag", "unflag", "query", "save", "quit", "undo"};
	
	char found = FALSE;
	int commandindex;

	for (commandindex = 0; i < COMMANDS_NUMBER && !found; i++) //ToDo: Clarify for with comment
	{
		if (!strcmp(scan->command, commandlist[commandindex]))
		{
			/* Assigns the command reference respecting the 
			   COMMAND_ defines order */
			command->command_ref = commandindex;
			found = TRUE;
		}
	}

	if (!found)
		return FALSE;

	return TRUE;
}