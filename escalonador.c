
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>

#define MAX 255
#define MAX_STRING_LENGTH 255

/* 	Struct de um processo: ID pra controle no vetor de processos/ids/threads
	e o resto dos atributos passados no enunciado do programa */

struct process {

	int id;
	float time_arrive, execution_time, deadline;
	char* name;
};

typedef struct process process;

/* 	Variaveis globais. Não sei se todas necessariamente precisam ser globais.
	Quando terminar a gente olha	*/ 

int total_processes = 0, cont_teste = 0;
process processes [MAX];
pthread_t threads[MAX];
long thread_ids[MAX];

sem_t mutex;

/* 	Printa os processos, só pra ver se foi lido corretamente do arquivo	*/

void list_processes(){
	int i;
	for (i = 0; i < total_processes; i++)
		printf("|ID: %d | |t0: %f | |name: %s | |dt: %f | |dl:%f |\n", processes[i].id, processes[i].time_arrive, processes[i].name, processes[i].execution_time, processes[i].deadline);
}

/* 	Função que executa o que realmente deve ser feito no EP (O que terá que ser feito nas threads).
	Depois muda o nome e pah */
void *teste(void * thread_id){
	int bla;
	sem_wait(&mutex);
	/* Código real */
	sem_post(&mutex);
	pthread_exit(NULL);
}

/* Cria as threads e coloca elas no vetor threads[MAX], em ordem de leitura do arquivo */

void create_threads(){
	long i;
	for (i = 0; i < total_processes; i++){
		printf("Criando thread %ld.\n", i);
		thread_ids[i] = i;
		assert (0 == pthread_create(&threads[i], NULL, teste, (void *) thread_ids[i]));
	}
}

/*	Recebe o arquivo com os valores de t0, name, dt e deadline, e cria um vetor
	processes[MAX] com os processos lidos. Os ids dos processos correspondem a ordem
	deles no vetor */

void read_file(char ** argv){
	FILE *fp;
	char name[255];
	fp = fopen(argv[1], "r");
	while(!feof(fp)){
		if (fscanf(fp, "%f %s %f %f\n", 
		&processes[total_processes].time_arrive, 
		name, 
		&processes[total_processes].execution_time, 
		&processes[total_processes].deadline) != 4){

			fclose(fp);
			exit(EXIT_FAILURE);
		}
		processes[total_processes].name = malloc(MAX_STRING_LENGTH * sizeof (char));
		strcpy(processes[total_processes].name, name);
		processes[total_processes].id = total_processes;
		total_processes++;
	}
	fclose(fp);
}

int main(int argc, char** argv){
	int i;
	sem_init(&mutex, 0, 1);
	read_file(argv);
	list_processes();
	create_threads();
	for (i = 0; i < total_processes; i++)
		assert(0 == pthread_join(threads[i], NULL));
	printf("Valor final: %d\n", cont_teste);
	return 0;

}