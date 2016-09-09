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

#define MAX 255
#define MAX_STRING_LENGTH 255

/* 	Struct de um processo: ID pra controle no vetor de processos/ids/threads
	e o resto dos atributos passados no enunciado do programa */

struct process {

	long id;
	float arrival_time, execution_time, deadline;
	char* name;
};

typedef struct process process;

/* 	Variaveis globais. Não sei se todas necessariamente precisam ser globais.
	Quando terminar a gente olha	*/ 

int total_processes = 0;
process processes [MAX];
pthread_t threads[MAX];
long thread_ids[MAX];
clock_t initial_time;
double global_time = 0;

pthread_mutex_t mutex;

int sched_getcpu(); //	Frescura do compilador

int compare_processes_qsort(const void * a, const void * b){
	return ((*(process *)a).arrival_time - (*(process *)b).arrival_time);
}

/* 	Printa os processos, só pra ver se foi lido corretamente do arquivo	*/

void list_processes(){
	int i;
	for (i = 0; i < total_processes; i++)
		printf("|ID: %ld | |t0: %f | |name: %s | |dt: %f | |dl:%f |\n", processes[i].id, processes[i].arrival_time, processes[i].name, processes[i].execution_time, processes[i].deadline);
}

/* 	Função que executa o que realmente deve ser feito no EP (O que terá que ser feito nas threads).
	Depois muda o nome e pah */

void *execute_process(void * thread_id){
	long id = (long) thread_id;
	clock_t start, tick;

	pthread_mutex_lock(&mutex);
	double cronometer_a = (- DBL_MIN), cronometer_b;	//	Cronometros para cronometrar o tempo total que o programa esta rodando.
	start = tick = clock();								//	Começa de - DBL_MIN devido a chegada do processo poder ser 0
	while ((cronometer_a < processes[id].execution_time) && (global_time < processes[id].deadline)){	//	Tempo do processo / deadline
		cronometer_b = difftime(tick, start) / CLOCKS_PER_SEC;
		tick = clock();
		cronometer_a = difftime(tick, start) / CLOCKS_PER_SEC;
		global_time = global_time + (cronometer_a - cronometer_b);	//	Computa quanto tempo passou em um "tick" do clock
	}
	printf("Current CPU: %d\n", sched_getcpu());	// Deixa isso aqui. Tem que ter no relatorio, e não posso esquecer desse comando
	if ((global_time) < processes[id].deadline)
		printf("%s rodou um tempo total de %lf segundos.\n", processes[id].name, cronometer_a);
	else
		printf("%s parou no deadline de %lf segundos.\n", processes[id].name, processes[id].deadline);
	pthread_mutex_unlock(&mutex);

	return NULL;
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
		total_processes++;
	}
	fclose(fp);
}

void * FCFS (){
	int i = 0;
	double start_time;
	qsort(processes, total_processes, sizeof(process), compare_processes_qsort);	//	qsort de acordo com a ordem de chegada. Talvez pros
	while(i < total_processes){														//	outros tenha que fazer outra função de comparação
		start_time = (difftime(clock(), initial_time) / CLOCKS_PER_SEC);
		if ((start_time - processes[i].arrival_time) > (1 / CLOCKS_PER_SEC)){		//	Espera até que um processo esteja disponível. Faz isso
			assert (0 == pthread_create(&threads[processes[i].id], NULL, execute_process, (void *) (long) i));
			i++;																	//	ate que todos os processos estejam em threads
		}
	}
	return NULL;
}

int main(int argc, char** argv){
	int i;
	initial_time = clock();
	pthread_mutex_init(&mutex, NULL);
	read_file(argv);
	FCFS();
	for (i = 0; i < total_processes; i++)
		assert(0 == pthread_join(threads[i], NULL));	//	Comando pra esperar as threads finalizarem
	pthread_exit(NULL);
	pthread_mutex_destroy(&mutex);
	return 0;

}