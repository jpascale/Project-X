all:
	gcc -pedantic -Wall -lm src/minokufront.c src/minokubackend.c src/random.c src/getnum2.c -o minoku
