#include <stdio.h>
#include <time.h>
#include <stdlib.h>


void
randomize(void)
{
        srand((int)time(NULL));
}

double
randnormalize (void)
{
        return rand()/(double)RAND_MAX; /*Devuelve un numero entre 0 y 1(si quisiera sin incluir sumarle 1 a RAND)*/
}

int
randint (int izq, int der)
{
        return (int)(randnormalize()*(der-izq)+izq); /*Devuelve un numero entero entre izq y der*/
}

double
randreal(double izq, double der)                        /*Devuelve un real entre izq y der*/
{
        return (randnormalize()*(der-izq)+izq);
}
