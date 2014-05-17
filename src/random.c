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
        return rand()/((double)RAND_MAX+1); /*Devuelve un numero entre 0 y 1(sin incluir)*/
}

int
randint (int izq, int der)
{
        return (int)(randnormalize()*(der-izq+1)+izq); /*Devuelve un numero entero entre izq y der*/
}

double
randreal(double izq, double der)                        /*Devuelve un real entre izq y der(sin incluir)*/
{
        return (randnormalize()*(der-izq)+izq);
}
