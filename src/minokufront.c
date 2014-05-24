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
			setNewGame(&game);
			Play(&game);
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
		printf("1.1 Juego individual sin limite de movimientos\n"
				"1.2 Juego individual con limite de movimientos\n"
				"1.3 Juego por campaña\n");

		option = getint("Elija una opcion: ");
		
		if (option > 3 || option < 1)
			printf("Ingrese una opcion valida.\n");
		
	} while(option > 3 || option < 1);

	game->gametype = option;

	return;
}

/* Sets ONLY gameplay config */
void setNewGame(tGame * game)
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
 	
 	//ToDo: Prepare boards
 	//Ready to play

 	return;
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

int InputCommand(tCommand * structcommand)
{	
	int i;
	char found = FALSE;
	int scanned_params_number;
	// Get commands

	char * comands[] = {"s", "flag", "unflag", "query", "save", "quit", "undo"};

	char scannedcommand[MAX_COMMAND_LEN];
	char params[MAX_PARAMS_LEN];

	// Scanf formatting
	char fmt[13]; //ToDo: Constant
	sprintf(fmt, "%%%ds %%%ds", MAX_COMMAND_LEN, MAX_PARAMS_LEN);

	printf("Introducir un comando:\n");

	scanned_params_number = scanf(fmt, scannedcommand, params);

	if (!scanned_params_number)
		return FALSE;

	for (i = 0; i < COMMANDS_NUMBER && !found; i++) //TODO: Clarify for with comment
	{
		if (!strcmp(scannedcommand, commands[i]))
		{
			structcommand->command = i;
			found = TRUE;
		}
	}

	if (!found)
		return FALSE;

	if (structcommand->command != COMMAND_QUIT && structcommand->command != COMMAND_UNDO)
	{
		if (scanned_params_number != 2)
			return FALSE;

		strcpy(structcommand->params, params);
	}

	return TRUE;
}
