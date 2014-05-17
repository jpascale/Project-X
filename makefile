#!/bin/bash

gcc -o minoku.o src/minokufront.c src/minokubackend.c src/random.c src/getnum.c -lm -pedantic
