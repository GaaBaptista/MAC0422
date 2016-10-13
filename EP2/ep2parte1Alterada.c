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
#include <sys/time.h>
#include <sys/resource.h>

#define team_size 4
#define track_size 10
#define total_laps 16

struct cyclist{
    pthread_t thread_cyclist;
    long pos,
    	speed,
    	speed_last_lap,
    	cyclist_ID,
		team_ID,
		completed_laps,
		start_pos,
		curr_pos,
		cyclist_forward;
};
typedef struct cyclist cyclist;

struct team{
	long team_cyclists[team_size];
	long remaining,
		leader;
};
typedef struct team team;

/* variáveis globais */
pthread_t dummies[2 * team_size];
cyclist cyclists [2 * team_size];
pthread_barrier_t barrier, barrier_msg;
long track [track_size][2];
team team_0, team_1;
pthread_mutex_t mutex, pmutex;
int quebrados = 0, done = 0;

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
		team_0.remaining++;

	}
	else{
		cyclists[id].team_ID = 1;
		cyclists[id].pos = track_size / 2;
		team_1.remaining++;
	}
}

/* Funcao usada para identificar a situacao das threads/ciclistas */
void print_cyclist(long id){
	printf( "pos: %ld | cyclist_ID: %ld | cyclist_forward: %ld | team_ID: %ld | completed_laps: %ld | curr_pos:%ld\n", cyclists[id].pos,
		cyclists[id].cyclist_ID, cyclists[id].cyclist_forward, cyclists[id].team_ID, cyclists[id].completed_laps, cyclists[id].curr_pos);
}

/* Estabelece a posicao inicial de cada ciclista, enfileirando-os */
long start_position(long id){
	int i = 0;
	while (track[cyclists[id].pos + i][0] != -1) i++;
	track[cyclists[id].pos + i][0] = id;
	cyclists[id].start_pos = cyclists[id].pos + i;
	cyclists[id].curr_pos = team_size - i;
	if (cyclists[id].team_ID == 0)
		team_0.team_cyclists[team_size - i - 1] = id;
	else
		team_1.team_cyclists[team_size - i - 1] = id;
	if (i == 1){
		if (cyclists[id].team_ID == 0)
			team_0.leader = id;
		else
			team_1.leader = id;
	}
	return (cyclists[id].pos + i);
}

void print_track(){
	for (int j = 0; j < 2; j++){
		for (int i = 0; i < track_size; i++)
			if (track[i][j] == -1) printf("|_|");
			else printf("|%ld|", track[i][j]);
		printf("\n");
	}
	printf("\n");
}

int check_engine(){
	return (rand() % 10);
}

void print_saida(long id){
	if (cyclists[id].team_ID == 0){
		printf("Volta nro %ld:\n", cyclists[team_0.team_cyclists[2]].completed_laps);
		for (int i = 0; i < 3; i++){
			printf("%d: Ciclista: %ld\n",
				i + 1, team_0.team_cyclists[i]);
		}
	}
	else{
		printf("Volta nro %ld:\n", cyclists[team_1.team_cyclists[2]].completed_laps);
		for (int i = 0; i < 3; i++){
			printf("%d: Ciclista: %ld\n",
				i + 1, team_1.team_cyclists[i]);
		}
	}
	printf("O terceiro ciclista demorou %d ms pra executar essa volta\n", 60 * track_size);
}


/* funcao das threads. Primeiramente, posiciona os ciclistas nas suas posicoes iniciais. Depois, vai simulando
	a corrida. Ainda falta fazer a segunda parte */

void * dummy (){
	int s;
	while(1){
		s = pthread_barrier_wait(&barrier);
		pthread_barrier_wait(&barrier_msg);
	}
	return NULL;
}

void change_leader(long thread_id, long team){
	int i = 0, j;
	long temp;
	if (team == 0){
		while (team_0.team_cyclists[i] != thread_id) i++;
		j = i + 1;
		while (j < team_size){
			temp = team_0.team_cyclists[i];
			team_0.team_cyclists[i] = team_0.team_cyclists[j];
			team_0.team_cyclists[j] = temp;
			j++;i++;
		}
		team_0.leader = team_0.team_cyclists[2];
	}
	else{
		while (team_1.team_cyclists[i] != thread_id) i++;
		j = i + 1;
		while (j < team_size){
			temp = team_1.team_cyclists[i];
			team_1.team_cyclists[i] = team_1.team_cyclists[j];
			team_1.team_cyclists[j] = temp;
			j++;i++;
		}
		team_1.leader = team_1.team_cyclists[2];
		}
	}

int break_bycicle(long thread_id, int track_side){
	if (cyclists[thread_id].team_ID == 0 && team_0.remaining > 3){
		team_0.remaining--;
		if (track_side == 0)
			track[(cyclists[thread_id].pos) % track_size][0] = -1;
		else
			track[(cyclists[thread_id].pos) % track_size][1] = -1;
		printf("Thread %ld quebrou na volta %ld na marca de %ld metros.\n", thread_id, cyclists[thread_id].completed_laps, cyclists[thread_id].pos % track_size);
		change_leader(thread_id, 0);
		return 1;

	}
	else if (cyclists[thread_id].team_ID == 1 && team_1.remaining > 3){
		team_1.remaining--;
		if (track_side == 0)
			track[(cyclists[thread_id].pos) % track_size][0] = -1;
		else
			track[(cyclists[thread_id].pos) % track_size][1] = -1;
        printf("Thread %ld quebrou na volta %ld na marca de %ld metros.\n", thread_id, cyclists[thread_id].completed_laps, cyclists[thread_id].pos % track_size);
		change_leader(thread_id, 1);
		return 1;
	}
	return 0;
}


// preenche o campo de quem do seu time está imediatamente antes
void get_whoIsFoward(long id){
	if(cyclists[id].team_ID == 0 && cyclists[id].cyclist_ID != team_0.leader){
		for(int i = 0; i < team_size; i++){
			if(cyclists[id].curr_pos == (cyclists[i].curr_pos + 1)){
				cyclists[id].cyclist_forward = cyclists[i].cyclist_ID;
				return;
			}
		}
	}
	else if(cyclists[id].team_ID == 1 && cyclists[id].cyclist_ID != team_1.leader){
		for(int i = team_size; i < 2 * team_size; i++){
			if(cyclists[id].curr_pos == (cyclists[i].curr_pos + 1)){
				cyclists[id].cyclist_forward = cyclists[i].cyclist_ID;
				return;
			}
		}
	}	
}

void * run_const(void * argument){
	int s = 0, broke = 0;
	long thread_id, start;
	thread_id = (long) argument;
	int track_side = 0;
	/* Coloca os ciclistas em suas posicoes iniciais */
	pthread_mutex_lock(&mutex);
	start = start_position(thread_id);
	cyclists[thread_id].pos = start;
	get_whoIsFoward(thread_id);
	print_track();
	/*print_cyclist(thread_id);*/
	pthread_mutex_unlock(&mutex);
	pthread_barrier_wait(&barrier);

	/* Todos os ciclistas estao a postos, prontos para comecar a corrida */
	while ((cyclists[team_0.leader].completed_laps < total_laps || cyclists[team_1.leader].completed_laps < total_laps) && !broke){
		/* Atualizacao da posicao do ciclista depois de 60ms. Usamos mod para facilitar a atualizacao
			da posicao de cada um. Ainda precisamos arrumar isso, colocar a condicao de quebra, e o modo
			debug. Falta coisa pra coroi */
		pthread_mutex_lock(&mutex);
		get_whoIsFoward(thread_id);
		/* Apaga a ultima posicao dele, dependendo de que pista ele veio */
		if (track_side == 0)
		    track[(cyclists[thread_id].pos) % track_size][0] = -1;
		else
		    track[(cyclists[thread_id].pos) % track_size][1] = -1;
		/* Avança o ciclista pra uma das pistas vagas */

		cyclists[thread_id].pos += 1;
		get_whoIsFoward(thread_id);
		if (track[cyclists[thread_id].pos % track_size][0] == -1){
		    track[cyclists[thread_id].pos % track_size][0] = cyclists[thread_id].cyclist_ID;
			track_side = 0;
			get_whoIsFoward(thread_id);
		}
		else{
		    track[cyclists[thread_id].pos % track_size][1] = cyclists[thread_id].cyclist_ID;
			track_side = 1;
			get_whoIsFoward(thread_id);
		}

		/*	verificamos se foi completada uma volta */

		if ((cyclists[thread_id].pos % track_size) == cyclists[thread_id].start_pos){
			cyclists[thread_id].completed_laps += 1;
			get_whoIsFoward(thread_id);
			/*if (cyclists[thread_id].curr_pos == 3){
				print_saida(thread_id);
			}*/
			if (cyclists[thread_id].completed_laps % 4 == 0 && cyclists[thread_id].completed_laps != total_laps)
				if (!check_engine()){
					broke = break_bycicle(thread_id, track_side);
				}
		}
		get_whoIsFoward(thread_id);
		pthread_mutex_unlock(&mutex);

		s = pthread_barrier_wait(&barrier);

		if (s == PTHREAD_BARRIER_SERIAL_THREAD){
			print_track();
		}
		get_whoIsFoward(thread_id);
		pthread_barrier_wait(&barrier_msg);
		/* Todos andaram e tem suas posicoes. Printamos pra saber como esta a fila */
	}
	get_whoIsFoward(thread_id);
	pthread_create(&dummies[thread_id], NULL, dummy, (void *) (long) thread_id);
	return NULL;
}

int main(int argc, char** argv){
	int id;
	team_0.remaining = 0;
	team_1.remaining = 0;
	pthread_barrier_init(&barrier, NULL, team_size * 2);
	pthread_barrier_init(&barrier_msg, NULL, team_size * 2);
	pthread_mutex_init(&mutex, NULL);
	srand(time(NULL));
	if (strcmp(argv[2], "u"))printf("OPCAO UNIFORME\n");
	/* inicializacao da pista */
	for (int j = 0; j < 2; j++)
		for (int i = 0; i < track_size; i++)
			track[i][j] = -1;
	/* Criacao das threads, e join delas */
	for (id = 0; id < (2 * team_size); id++){
		create_cyclists(id);
		pthread_create(&cyclists[id].thread_cyclist, NULL, run_const, (void *) (long) id);
	}
	for (id = 0; id < (2 * team_size); id++){
		pthread_join(cyclists[id].thread_cyclist, NULL);
	}
	for (id = 0; id < (2 * team_size); id++)
		print_cyclist(id);
	sleep(1);
	return 0;




}

/*printf("TIME0:\n");
	for (int i = 0; i < team_size; i++){
		printf("team_0.team_cyclists[%d]: %ld\n", i, team_0.team_cyclists[i]);
	}
	printf("TIME1:\n");
	for (int i = 0; i < team_size; i++){
		printf("team_1.team_cyclists[%d]: %ld\n", i, team_1.team_cyclists[i]);
	}*/
