#include<sys/wait.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>//had getchar function
#define GEN_WORD_LIMIT 256 //default block size
#define GEN_TOK_SIZE 64 //default token size
#define GEN_TOK_DELIM " \t\r\n\a" //delimiters for strtok
char* gen_read_line(){
	int buffer_size=GEN_WORD_LIMIT;//size of block storing string entered by user
	int index=0;//used to store the entered charecters in their required place
	int c;//used in reading user input
	char *buffer=malloc(sizeof(char)*buffer_size);//string storing block
	if(!buffer){
		fprintf(stderr,"gen:allocation error!\n");
		exit(EXIT_FAILURE);
	}
	while(1){//infinte loop to take input
		c=getchar();
		if(c=='\n'||c==EOF){
			buffer[index]='\0';//end of entered string marked by null charecter
			return buffer;//exit condition on reading new line charecter or encountering end of file or an error
		}
		else{
			buffer[index]=c;
		}
		index++;
		//to reallocate if user entry beyond default block size
		if(index>=buffer_size){
			buffer_size+=GEN_WORD_LIMIT;
			buffer=realloc(buffer, buffer_size);
			if(!buffer){
				fprintf(stderr,"gen:allocation error!");
				exit(EXIT_FAILURE);
			}
		}
	}
}
char** gen_split_line(char* line){//split given command line into arguements through whitespaces
	int buffer_size=GEN_TOK_SIZE;
	int index=0;
	char** tokens = malloc(sizeof(char*)*buffer_size);
	char* token;
	if(!tokens){
		fprintf(stderr,"gen:allocation error!");
		exit(EXIT_FAILURE);
	}
	token=strtok(line, GEN_TOK_DELIM);
	while(token!=NULL){
		tokens[index]=token;
		index++;
		if(index>=buffer_size){
			buffer_size+=GEN_TOK_SIZE;
			tokens=realloc(tokens, buffer_size*sizeof(char*));
			if(!tokens){
				fprintf(stderr, "gen:allocation error!");
				exit(EXIT_FAILURE);
			}
		}
		token=strtok(NULL,GEN_TOK_DELIM);
	}
	tokens[index]=NULL;
	return tokens;
}
int gen_launch(char** args){//launch external programs
	pid_t pid, wpid;
	int status;
	pid=fork();//get process ID
	if(pid==0){//child process
		if(execvp(args[0], args)==-1){
			perror("gen");
		}
		exit(EXIT_FAILURE);
	}
	else if(pid<0){
		perror("gen");
	}
	else{
		do{
			wpid=waitpid(pid,&status,WUNTRACED);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}
/*
  Function Declarations for builtin shell commands:
 */
int gen_cd(char **args);
int gen_help(char **args);
int gen_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &gen_cd,
  &gen_help,
  &gen_exit
};

int gen_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int gen_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "gen: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("gen");
    }
  }
  return 1;
}

int gen_help(char **args)
{
  int i;
  printf("Alterra's Gen shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < gen_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int gen_exit(char **args)
{
  return 0;
}
int gen_execute(char** args){
	int i;
	if(args[0]==NULL)return 1;//no arguement is NULL
	for(i=0;i<gen_num_builtins();i++){
		if(strcmp(args[0],builtin_str[i])==0)return (*builtin_func[i])(args);
	}
	return gen_launch(args);
}
void gen_loop(){//command loop function
        int status=1;
        char *line=NULL;
        char **args=NULL;
        do{
                printf("~#@#~ ");//the charecter already in place by default
                line=gen_read_line();//reads the arguement
                args=gen_split_line(line);//parses for command and its arguements
                status=gen_execute(args);//returns status after execution
                free(line);//free up used memory
                free(args);//free up used memory
        }
        while(status);//loop used when status in running
}
int main(int argc, char **argv){
        //load any config files
        //run command loop
        gen_loop();
        //run shutdown commands
        return EXIT_SUCCESS;
}
