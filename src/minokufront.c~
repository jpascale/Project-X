#include "minokubackend.h"

int
main(void)
{
	randomize();

	int option;
	option = Menu();
	switch (option)
	{
		case 1: /*Juego Nuevo*/
			option = Menu2();
			switch (option)
			{
				case 1:  /*Juego individual sin limite de movimientos*/
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

int
Menu2(void)
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
	return option;
}

