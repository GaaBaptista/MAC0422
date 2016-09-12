/*	Gabriel Baptista		8941300
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
	filho  */

void binary_commands(){
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
}

/* Execução do chmod e do id por system call*/

void sys_commands(){
	const char * path;
	mode_t number;
	path = arguments[2];
	number = strtol(arguments[1], NULL, 8);
	if (strcmp(arguments[1], "-u") == 0)
		printf( "%d\n" , getuid());
	else{
        if (chmod(path, number) == 0)
            printf("Permissão alterada\n");
        else
            printf("Fracasso\n");
    }
}

int main(int argc, char *argv[]){
	while (1){
		print_prompt();
		read_command();
		if((strcmp(arguments[0],"/bin/ls") == 0) || (strcmp(arguments[0], "/bin/date") == 0) ||
                (strcmp(arguments[0], "./ep1") == 0)){
                binary_commands();
                //limpando arguments para a próxima chamada
                arguments[0] = NULL;
                arguments[1] = NULL;
                arguments[2] = NULL;
        }
        else{
            if ((strcmp(arguments[0],"chmod") == 0) || (strcmp(arguments[0], "id") == 0)){
                sys_commands();
                arguments[0] = NULL;
                arguments[1] = NULL;
                arguments[2] = NULL;
            }
            else{
                if((strcmp(arguments[0],"exit") == 0)) exit(EXIT_SUCCESS);
                else printf("Tentando sair? Digite 'exit'\n");
            }
        }
    }
	return 0;
}