/*	Gabriel Baptista		8941300
	Hélio Assakura			8941064 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <sched.h>
#include <float.h>
#include <time.h>

#define team_size 4
#define track_size 20

struct cyclist{	
    pthread_t thread_cyclist;
    long pos,
    	speed,
    	cyclist_ID,
		team_ID,
		completed_laps,
		start_pos;
};
typedef struct cyclist cyclist;

/* variáveis globais */

cyclist cyclists [2 * team_size];
pthread_barrier_t barrier;
long track [track_size][2];
pthread_mutex_t mutex;

/* Contagem de tempo (nao necessario) */
void nsleep(long us)
{
    struct timespec wait;
    
    wait.tv_sec = us / (1000 * 1000);
    wait.tv_nsec = (us % (1000 * 1000)) * 1000;
    nanosleep(&wait, NULL);
}

/* Cria os ciclistas. As equipes estao identificadas por 0 e 1. */

void create_cyclists(long id){
	cyclists[id].speed = 60;
	cyclists[id].cyclist_ID = id;
	cyclists[id].completed_laps = 0;
	if (id < team_size){
		cyclists[id].team_ID = 0;
		cyclists[id].pos = 0;
		cyclists[id].start_pos = 0;
	}
	else{
		cyclists[id].team_ID = 1;
		cyclists[id].pos = track_size / 2;
		cyclists[id].start_pos = track_size / 2;
	}
}

/* Funcao usada para identificar a situacao das threads/ciclistas */
void print_cyclist(long id){
	printf( "pos: %ld | cyclist_ID: %ld | team_ID: %ld | completed_laps: %ld\n", cyclists[id].pos,
		cyclists[id].cyclist_ID, cyclists[id].team_ID, cyclists[id].completed_laps);
}

/* Estabelece a posicao inicial de cada ciclista, enfileirando-os */
long start_position(long id){
	int i = 0;
	while (track[cyclists[id].pos + i][0] != -1) i++;
	track[cyclists[id].pos + i][0] = id;
	return (cyclists[id].pos + i);
}

/* funcao das threads. Primeiramente, posiciona os ciclistas nas suas posicoes iniciais. Depois, vai simulando
	a corrida. Ainda falta fazer a segunda parte */

void * run_const(void * argument){
	long thread_id, start;
	thread_id = (long) argument;

	/* Coloca os ciclistas em suas posicoes iniciais */
	pthread_mutex_lock(&mutex);
	start = start_position(thread_id);
	cyclists[thread_id].pos = start;
	for (int i = 0; i < track_size; i++)
		if (track[i][0] == -1) printf("|_|");
		else printf("|%ld|", track[i][0]);
	printf("\n");
	pthread_mutex_unlock(&mutex);
	pthread_barrier_wait(&barrier);

	/* Todos os ciclistas estao a postos, prontos para comecar a corrida */

	while (cyclists[thread_id].completed_laps < 4){
		
		/* Atualizacao da posicao do ciclista depois de 60ms. Usamos mod para facilitar a atualizacao
			da posicao de cada um. Ainda precisamos arrumar isso, colocar a condicao de quebra, e o modo
			debug. Falta coisa pra coroi */

		pthread_mutex_lock(&mutex);
		track[cyclists[thread_id].pos % track_size][0] = 0;
		cyclists[thread_id].pos += 1;
		track[cyclists[thread_id].pos % track_size][0] = thread_id;
		for (int i = 0; i < track_size; i++)
			if (track[i][0] == -1) printf("|_|");
			else printf("|%ld|", track[i][0]);
		printf("\n");
		printf("Thread %ld na barreira\n", thread_id);
		pthread_mutex_unlock(&mutex);
		pthread_barrier_wait(&barrier);

		/* Todos andaram e tem suas posicoes. Printamos pra saber como esta a fila */

		print_cyclist(thread_id);
		nsleep(6000000);

		/*	verificamos se foi completada uma volta */
		if ((cyclists[thread_id].pos % track_size) == cyclists[thread_id].start_pos)
			cyclists[thread_id].completed_laps += 1;
	}
	printf("Ciclista %ld acabou\n", thread_id);
	return NULL;
}

int main(int argc, char** argv){
	int id;
	pthread_barrier_init(&barrier, NULL, team_size * 2);
	pthread_mutex_init(&mutex, NULL);
	if (strcmp(argv[2], "u"))printf("OPCAO UNIFORME\n");
	/* inicializacao da pista */
	for (id = 0; id < track_size; id++)
		track[id][0] = -1;
	/* Criacao das threads, e join delas */
	for (id = 0; id < (2 * team_size); id++){
		create_cyclists(id);
		pthread_create(&cyclists[id].thread_cyclist, NULL, run_const, (void *) (long) id);
	}
	for (id = 0; id < (2 * team_size); id++){
		pthread_join(cyclists[id].thread_cyclist, NULL);
	}
	return 0;
}

