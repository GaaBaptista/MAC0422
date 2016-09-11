/*	Ainda tem que reorganizar a bagaça toda. Tem umas variaveis com nome nada a ver
	que depois eu arrumo. Vai pensando nas paradas todas aí e pah	*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <sched.h>
#include <float.h>
#include <semaphore.h>

#define MAX 255
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

int total_processes = 0;
long queue_size = 0;
int queue_flags [MAX];

process processes [MAX];
process processes_queue[MAX];

clock_t initial_time;

pthread_t threads[MAX];
pthread_mutex_t mutex, sem_queue;

double global_cronometer = 0;


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

/*double global_clock(){
	return ((clock() - initial_time) / CLOCKS_PER_SEC);
}*/

/*	Recebe o arquivo com os valores de t0, name, dt e deadline, e cria um vetor
	processes[MAX] com os processos lidos. Os ids dos processos correspondem a ordem
	deles no vetor */

void read_file(char ** argv){
	FILE *fp;
	char name[255];
	fp = fopen(argv[1], "r");
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

/*void *run_thread_FCFS(void * thread_id){
	long id = (long) thread_id;
	clock_t tick;

	pthread_mutex_lock(&mutex);
	double cronometer_a = DBL_MIN, tick_time;	//	Cronometros para cronometrar o tempo total que o programa esta rodando.
	while ((cronometer_a < processes[id].execution_time) && (global_clock() < processes[id].deadline)){	//	Tempo do processo / deadline
		tick = clock();
		tick_time = (double)(clock() - tick) / ((double)(CLOCKS_PER_SEC)) ;
		cronometer_a += tick_time;
		processes_queue[id].remaining_time -= tick_time;
	}*/
	/*printf("Current CPU: %d\n", sched_getcpu());*/	// Deixa isso aqui. Tem que ter no relatorio, e não posso esquecer desse comando
	/*if ((global_clock()) < processes[id].deadline)
		printf("%s rodou um tempo total de %lf segundos.\n", processes[id].name, cronometer_a);
	else
		printf("%s parou no deadline de %lf segundos.\n", processes[id].name, processes[id].deadline);
	pthread_mutex_unlock(&mutex);
	return NULL;
}*/

/*void * FCFS (){
	int i = 0;
	double start_time;
	qsort(processes, total_processes, sizeof(process), compare_processes_qsort);	//	qsort de acordo com a ordem de chegada. Talvez pros
	while(i < total_processes){														//	outros tenha que fazer outra função de comparação
		start_time = (difftime(clock(), initial_time) / CLOCKS_PER_SEC);
		if ((start_time - processes[i].arrival_time) > (1 / CLOCKS_PER_SEC)){		//	Espera até que um processo esteja disponível. Faz isso
			assert (0 == pthread_create(&threads[i], NULL, run_thread_FCFS, (void *) (long) i));
			i++;																	//	ate que todos os processos estejam em threads
		}
	}
	return NULL;
}*/

void  sort_queue (int n){
	long i = (long) n - 1;
	long j = (long) n;
	process temp;
	pthread_mutex_lock(&mutex);
	printf("ESPERANDO....\n");
	while (i >= 0 && processes_queue[i].remaining_time > processes_queue[j].remaining_time && processes_queue[i].remaining_time > 0){
		temp = processes_queue[i];
		processes_queue[i] = processes_queue[j];
		processes_queue[j] = temp;
		i--;
		j--;
	}
	printf("SAI\n");
	pthread_mutex_unlock(&mutex);
}

void * run_thread_SRTN(void * thread_id){
	long id = (long) thread_id;
	clock_t tick;
	double tick_time, processed_time = 0;
	printf("%s comecou a rodar...\n", processes[id].name);
	pthread_mutex_lock(&mutex);
	while (processes[id].remaining_time > 0.0){	//	Tempo do processo / deadline
		if (processes[id].run_flag){
			tick = clock();
			tick_time = ((clock() - tick) / (double)(CLOCKS_PER_SEC)) ;
			processes_queue[id].remaining_time -= tick_time;
			processes[id].remaining_time -= tick_time;
			processed_time += tick_time;
			global_cronometer += tick_time;
		}
	}
	processes[id].run_flag = 0;
	/*printf("Current CPU: %d\n", sched_getcpu());*/	// Deixa isso aqui. Tem que ter no relatorio, e não posso esquecer desse comando
	printf("%s parou de rodar depois de %lf segundos.\n", processes[id].name, processed_time);
	pthread_mutex_unlock(&mutex);
	return NULL;
}

void * SRTN (){
	int i = 0;
	clock_t tick;
	double cronometer = DBL_MIN, tick_time;
	pthread_t thread_queue;
	qsort(processes, total_processes, sizeof(process), compare_processes_qsort);	//	qsort de acordo com a ordem de chegada. Talvez pros
	while(i < total_processes){														//	outros tenha que fazer outra função de comparação
		tick = clock();
		tick_time = ((clock() - tick) / (double)(CLOCKS_PER_SEC));
		global_cronometer += tick_time;
		for ( ; (global_cronometer > processes[i].arrival_time) && i < total_processes; ){		//	Espera até que um processo esteja disponível. Faz isso	
			processes_queue[i] = processes[i];
			assert (0 == pthread_create(&threads[i], NULL, run_thread_SRTN, (void *) (long) i));
			i++;
			queue_size++;																	//	ate que todos os processos estejam em threads
		}
	}
	return NULL;
}

int main(int argc, char** argv){
	int i;
	initial_time = clock();	
	pthread_mutex_init(&mutex, NULL);
	read_file(argv);
	SRTN();

	for (i = 0; i < total_processes; i++)
		assert(0 == pthread_join(threads[i], NULL));	//	Comando pra esperar as threads finalizarem
	for (i = 0; i < total_processes; i++)
		printf("ID: %d 	PROCESSO:	%ld 	remaining_time: %lf\n", i, processes_queue[i].id, processes_queue[i].remaining_time);
	printf("GLOBAL_CRONOMETER: %lf\n", global_cronometer);
	pthread_exit(NULL);
	return 0;

}

/* 	PRINTS:
	for (i = 0; i < total_processes; i++)
		printf("ID: %d 	PROCESSO:	%ld 	remaining_time: %lf\n", i, processes[i].id, processes[i].remaining_time);
	for (i = 0; i < total_processes; i++)
		printf("ID: %d 	PROCESSO:	%ld 	remaining_time: %lf\n", i, processes[i].id, processes[i].remaining_time);
	for (int j = 0; j < i; j++)
		printf("ID: %d 	PROCESSO:	%ld 	remaining_time: %lf\n", j, processes_queue[j].id, processes_queue[j].remaining_time);
	printf("%s sendo inserido na fila\n", processes[i].name);
		printf("%s terminou de rodar...\n", processes[id].name);
					printf("HOLDING THREAD %ld...\n", id);
						printf("GLOBAL TIME: %lf\n", global_time);

*/

