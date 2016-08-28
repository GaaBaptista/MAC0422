/*	Gabriel Baptista		8941300(?)
	Hélio Assakura			8941064 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define size_directory 100
#define path_size 100
#define MAX_ARGUMENTS 10

char * arguments [MAX_ARGUMENTS];
char * command;

/* Mostra o prompt, com o diretorio local */

void print_prompt(){
	char * current_directory = (char *) malloc (size_directory); 
	if (getcwd (current_directory, size_directory) == current_directory)
		printf ("(%s): ", current_directory);
}

/* Recebe a linha de comando e separa o comando e os argumentos em arguments[MAX_ARGUMENTS] */

void read_command (){
	char * line;
	char * token;
	int i = 0;
	line = readline("");
	if (line && *line)
    	add_history (line);
	command = token = strtok (line, " ");
	while (token != NULL){
		arguments[i] = (char *) malloc (strlen(token) * sizeof (char));
		strcpy (arguments[i++], token);
		token = strtok (NULL, " ");
	}
}

/* 	Execução dos binarios (ls, date e ./ <args>), retornando o pid do
	filho (não necessário, tem que tirar depois) */
				
int binary_commands(){
	pid_t child_pid;
	if ((child_pid = fork()) > 0){
		waitpid(-1, NULL, 0);
	}
	else if (child_pid == 0)
		execve(command, arguments, 0);
	else{
		printf("\n ERRO");
      	exit(1);
  	}
	return child_pid;
}

/* Execução do chmod por system call, ta separado do id. Tem que juntar depois */

void sys_commands_chmod(){
	const char * path;
	mode_t number;
	struct stat fileStat;
	path = arguments[2];
	number = strtol(arguments[1], NULL, 8);
	if (chmod(path, number) == 0)
		printf("Permissão alterada");
	else
		printf("fracasso");
}

/* Execução do id -u */

void sys_commands_id(){
	if (strcmp(arguments[1], "-u") == 0)
		printf( "%d" , getuid());
	else
		exit(1);
}

int main(int argc, char *argv[]){
	int i;
	while (1){
		print_prompt();
		read_command();
		/*i = binary_commands();
		printf ("pid_id: %d\n", i);
		sys_commands_chmod();*/
		sys_commands_id();
	}
	return 0;
}