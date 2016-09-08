
	#include <stdio.h>
	#include <string.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <pthread.h>
	#include <assert.h>

	#define MAX 255
	#define MAX_STRING_LENGTH 255

	struct process {

		int id;
		float time_arrive, execution_time, deadline;
		char* name;
	};

	typedef struct process process;

	int total_processes = 0;
	process processes [MAX];

	void list_processes(){
		int i;
		for (i = 0; i < total_processes; i++)
			printf("|ID: %d | |t0: %f | |name: %s | |dt: %f | |dl:%f |\n", processes[i].id, processes[i].time_arrive, processes[i].name, processes[i].execution_time, processes[i].deadline);
	}
	

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
		read_file(argv);
		list_processes();
		return 0;

	}