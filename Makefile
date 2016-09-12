all: ep1sh ep1esc

ep1sh: ep1sh.c
	gcc -Wall -ansi -pedantic ep1sh.c -o ep1sh -std=gnu99 -lreadline

ep1esc: escalonador.c
	gcc -Wall -ansi -pedantic escalonador.c -o ep1esc -std=gnu99 -lpthread