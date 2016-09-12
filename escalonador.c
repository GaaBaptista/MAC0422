/*	Ainda tem que reorganizar a bagaça toda. Tem umas variaveis com nome nada a ver
	que depois eu arrumo. Vai pensando nas paradas todas aí e pah	*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <sched.h>
#include <float.h>
#include <semaphore.h>
#include <time.h>

#define MAX 10000
#define MAX_STRING_LENGTH 255

/* 	Struct de um processo: ID pra controle no vetor de processos/ids/threads
	e o resto dos atributos passados no enunciado do programa */

struct process {

	long id;
	double arrival_time, execution_time, deadline, quantum, remaining_time;
	char* name;
	int run_flag;
};
typedef struct process process;





/* 	Variaveis globais. Não sei se todas necessariamente precisam ser globais.
	Quando terminar a gente olha	*/ 

int total_processes = 0, wf = 1, line = 1;
long context_change = 0;
process processes [MAX];

pthread_t threads[MAX];
pthread_mutex_t mutex;

double global_cronometer = 0;
struct timespec start, end, initial_time;
FILE *file;

int sched_getcpu(); //	Frescura do compilador

int compare_processes_qsort(const void * a, const void * b){
	return ((*(process *)a).arrival_time - (*(process *)b).arrival_time);
}

/* 	Printa os processos, só pra ver se foi lido corretamente do arquivo	*/

void list_processes(){
	int i;
	for (i = 0; i < total_processes; i++)
		printf("|ID: %ld | |t0: %lf | |name: %s | |dt: %lf | |dl:%lf |\n", processes[i].id, processes[i].arrival_time, processes[i].name, processes[i].execution_time, processes[i].deadline);
}

static double TimeSpecToSeconds(struct timespec* ts)
{
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

/*	Recebe o arquivo com os valores de t0, name, dt e deadline, e cria um vetor
	processes[MAX] com os processos lidos. Os ids dos processos correspondem a ordem
	deles no vetor */

void read_file(char ** argv){
	FILE *fp;
	char name[255];
	fp = fopen(argv[2], "r");
	if (fp == NULL) {
      printf ("\nArquivo %s não encontrado.\n", argv[1]);
      exit (EXIT_FAILURE);
   	}
	while(!feof(fp)){
		if (fscanf(fp, "%lf %s %lf %lf\n", 
		&processes[total_processes].arrival_time, 
		name, 
		&processes[total_processes].execution_time, 
		&processes[total_processes].deadline) != 4){

			fclose(fp);
			exit(EXIT_FAILURE);
		}
		processes[total_processes].name = malloc(MAX_STRING_LENGTH * sizeof (char));
		strcpy(processes[total_processes].name, name);
		processes[total_processes].id = total_processes;
		processes[total_processes].quantum = 1;
		processes[total_processes].remaining_time = processes[total_processes].execution_time;
		processes[total_processes].run_flag = 0;
		total_processes++;
	}
	fclose(fp);
}

/* 	Função que executa o que realmente deve ser feito no EP (O que terá que ser feito nas threads).
	Depois muda o nome e pah */

void *run_thread_FCFS(void * thread_id){
	long id = (long) thread_id;
	struct timespec st_time, curr_time;
	clock_gettime(CLOCK_MONOTONIC, &st_time);
	int CPU;
	CPU = sched_getcpu();
	if (wf) fprintf(stderr, "Processo %s usa a CPU %d\n", processes[id].name, CPU);
	pthread_mutex_lock(&mutex);
	while (processes[id].remaining_time > 0.0){	//	Tempo do processo / deadline
		clock_gettime(CLOCK_MONOTONIC, &curr_time);
		if (processes[id].run_flag == 1){
			processes[id].remaining_time -= (TimeSpecToSeconds(&curr_time) - TimeSpecToSeconds(&st_time));
		}
		st_time = curr_time;
	}
	pthread_mutex_unlock(&mutex);
	clock_gettime(CLOCK_MONOTONIC, &curr_time);
	double tf = (TimeSpecToSeconds(&curr_time) - TimeSpecToSeconds(&initial_time));
	fprintf(file, "%s %lf %lf\n",processes[id].name, tf, tf - processes[id].arrival_time);
	if (wf) {
		fprintf(stderr, "Processo %s liberou a CPU %d\n", processes[id].name, CPU);
		fprintf(stderr, "Linha da saída: %d\n", line++);
	}
	/*printf("Current CPU: %d\n", sched_getcpu());*/	// Deixa isso aqui. Tem que ter no relatorio, e não posso esquecer desse comando
	return NULL;
}

void * FCFS (){
	int i = 0;
	double tick_time;
	qsort(processes, total_processes, sizeof(process), compare_processes_qsort);	//	qsort de acordo com a ordem de chegada. Talvez pros
	while(i < total_processes){														//	outros tenha que fazer outra função de comparação
		clock_gettime(CLOCK_MONOTONIC, &end);
		tick_time = (TimeSpecToSeconds(&end) - TimeSpecToSeconds(&initial_time));
		for ( ; tick_time > processes[i].arrival_time && i < total_processes; ){		//	Espera até que um processo esteja disponível. Faz isso	
			if (wf)
				fprintf(stderr, "Processo %s chegou no sistema. Linha %d\n", processes[i].name, i);
			processes[i].run_flag = 1;
			pthread_create(&threads[i], NULL, run_thread_FCFS, (void *) (long) i);
			pthread_join(threads[i], NULL);
			i++;
		}
	}
	return NULL;
}

void  sort_queue (int n){
	int current = n, previous = n - 1;
	process temp;
	pthread_mutex_lock(&mutex);
	for (int i = 0; i <= n; i++)
		processes[i].run_flag = 0;
	while (n && previous >= 0 && processes[previous].remaining_time > processes[current].remaining_time){
		temp = processes[previous];
		processes[previous] = processes[current];
		processes[current] = temp;
		previous--;
		current--;
	}
	if (current == 0) context_change++;
	for (int i = 0; i <= n; i++)
		if (processes[i].remaining_time > 0){
			processes[i].run_flag = 1;
			break;
		}
	pthread_mutex_unlock(&mutex);
}

void * run_thread_SRTN(void * thread_id){
	long id = (long) thread_id;
	struct timespec st_time, curr_time;
	clock_gettime(CLOCK_MONOTONIC, &st_time);
	int CPU;
	CPU = sched_getcpu();
	while (processes[id].remaining_time > 0.0){	//	Tempo do processo / deadline
		pthread_mutex_lock(&mutex);
		clock_gettime(CLOCK_MONOTONIC, &curr_time);
		if (processes[id].run_flag == 1)
			processes[id].remaining_time -= (TimeSpecToSeconds(&curr_time) - TimeSpecToSeconds(&st_time));
		st_time = curr_time;
		pthread_mutex_unlock(&mutex);
	}
	clock_gettime(CLOCK_MONOTONIC, &curr_time);
	double tf = (TimeSpecToSeconds(&curr_time) - TimeSpecToSeconds(&initial_time));
	fprintf(file, "%s %lf %lf\n",processes[id].name, tf, tf - processes[id].arrival_time);
	if (wf) {
		fprintf(stderr, "Processo %s liberou a CPU %d\n", processes[id].name, CPU);
		fprintf(stderr, "Linha da saída: %d\n", line++);
	}
	/*printf("Current CPU: %d\n", sched_getcpu());*/	// Deixa isso aqui. Tem que ter no relatorio, e não posso esquecer desse comando
	
	return NULL;
}

void * SRTN (){
	int i = 0;
	double tick_time;
	qsort(processes, total_processes, sizeof(process), compare_processes_qsort);	//	qsort de acordo com a ordem de chegada. Talvez pros
	while(i < total_processes){														//	outros tenha que fazer outra função de comparação
		clock_gettime(CLOCK_MONOTONIC, &end);
		tick_time = (TimeSpecToSeconds(&end) - TimeSpecToSeconds(&initial_time));
		for ( ; tick_time > processes[i].arrival_time && i < total_processes; ){		//	Espera até que um processo esteja disponível. Faz isso	
			if (wf)
				fprintf(stderr, "Processo %s chegou no sistema. Linha %d\n", processes[i].name, i);
			sort_queue(i);
			pthread_create(&threads[i], NULL, run_thread_SRTN, (void *) (long) i);
			i++;
		}
	}
	for (int j = 0; j < total_processes; j++)
		processes[j].run_flag = 0;
	for (int j = 0; j < total_processes; j++)
		if (processes[j].remaining_time < 0);
		else{
			processes[j].run_flag = 1;
			pthread_join(threads[j], NULL);
			processes[i].run_flag = 0;
		}		
	return NULL;
}

int main(int argc, char** argv){
	int scheduler;
	file = fopen(argv[3], "w");
	if (argc == 5)
		wf = 1;
	else
		wf = 0;
	scheduler = atoi(argv[1]);
	clock_gettime(CLOCK_MONOTONIC, &initial_time);
	pthread_mutex_init(&mutex, NULL);
	read_file(argv);
	if (scheduler == 1)
		FCFS();
	else if (scheduler == 2)
		SRTN();
	else
		printf("Não fizemos =(\n");
	if (wf) fprintf(stderr, "Mudanças de contexto: %ld\n", context_change);
	return 0;

}

