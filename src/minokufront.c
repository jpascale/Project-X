#include "minokubackend.h"

int
main(void)
{
	int option;
	tGame game;
	randomize();
	option = Menu();
	switch (option)
	{
		case 1: /*Juego Nuevo*/
			PlayMenu(&game);
			switch (game.gametype)
			{
				case 1:
					getLevelandDim (&game);
					break;
				case 2:	/*Juego individual con limite de movimientos*/
					break;
				case 3:	/*Juego por campania*/
					break;
			}
			break;
		case 2:	/*Cargar partida*/
			break;		
	}

	return 0;
}

int
Menu(void)
{
	int option;
	do
	{	printf("1. Juego Nuevo\n"  
				"2. Recuperar Juego Grabado\n"
				"3. Terminar\n");
	
		option = getint("Elija una opcion: ");
		if (option>3 || option<1)
			printf("Ingrese una opcion valida.\n");
	}while(option>3 || option<1 );
	
	return option;

}

void
PlayMenu(tGame * game)
{
	int option;
	do
	{
		printf("1.1 Juego individual sin limite de movimientos\n"
				"1.2 Juego individual con limite de movimientos\n"
				"1.3 Juego por campaña\n");
		option = getint("Elija una opcion: ");
		if (option>3 || option <1)
			printf("Ingrese una option valida.\n");
	}while(option>3 || option<1);
	game->gametype = option;
}

/*returns level, modify rows and columns*/
void getLevelandDim (tGame * game)
{
	int rowsaux, colaux, level;
	do
	{
		rowsaux = getint("Ingrese FILAS, minimo 5 y maximo 19:\n");
	} while (rowsaux < 5 || rowsaux > 19);

	do
	{
		colaux = getint("Ingrese COLUMNAS, minimo 5 y maximo 19:\n");
	} while (colaux < 5 || colaux > 19);
	
	do
	{
		level = getint("Ingrese dificultad:\n1.Facil\n2.Medio\n3.Dificil\n4.Pesadilla\n");
	} while (level < 1 || level > 4);
	
	game->visualboard.rows = game->hiddenboard.rows = rowsaux;
	game->visualboard.columns = game->hiddenboard.columns = colaux;
	game->level = level;

}

void PrintBoard(tBoard * structboard)
{
	int i, j, rows = structboard->rows, columns = structboard->columns;
	for(i = 0; i < rows; i++)
	{
		for (j = 0; j < columns; j++)
			putchar(structboard->board[i][j]);
		putchar('\n');
	}
}
