#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#define LSH_TOK_BUFSIZE 64
#define DELIM " \t\r\n\a"


FILE * cfptr;
char hist[50][100];
char pidhist[50][100];
int pidh=0;
int h=0;

struct current_pids *head_pid;

struct current_pids{
	int pid;
	struct current_pids *next;
};

int insert_pid(int pid){
	struct current_pids *node = (struct current_pids *)malloc(
		sizeof(struct current_pids));
	struct current_pids *loop_node;

	node->pid = pid;
	node->next = NULL;

	/* Go through everything */
	loop_node = head_pid;

	while (loop_node->next != NULL){
		loop_node = loop_node->next;
	}

	loop_node->next = node;

	return 1;
}

int delete_pid(int pid){
	struct current_pids *loop_node;
	struct current_pids *prev;

	loop_node = head_pid;
	prev = head_pid;

	while(loop_node != NULL){

		if (loop_node->pid == pid){
			prev->next = loop_node->next;
			free(loop_node);
			break;
		}

		prev = loop_node;
		loop_node = loop_node->next;
	}

	return 1;
}


char *readline(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line, &bufsize, stdin);
  return line;
}

char **split(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, p = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *tok;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  //fprintf(cfptr,"%s\n",line);
  
  tok = strtok(line, DELIM);
  
  while (tok != NULL) {
    tokens[p] = tok;
    p++;

    if (p >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    tok = strtok(NULL, DELIM);
  }
  tokens[p] = NULL;
  return tokens;
}


/*
  List of builtin commands, followed by their corresponding functions.
 */
int ecd(char **args);
int eexit(char **args);
int ehist(char **args);
int epid(char **args);
int ehistn(char **args);


char *builtin_str[] = {
  "cd",
  "exit",
  "hist",
  "pid",
};

int (*builtin_func[]) (char **) = {
  &ecd,
  &eexit,
  &ehist,
  &epid,
};

int num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int ecd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, " expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
    
  }
  return 1;
}



int eexit(char **args)
{
  exit(0);
}

int ehist(char **args)
{
  
  
  for(int i=0;i<h;i++)
  {
    printf("%d. %s",i,hist[i]);
  }
  //printf("%s",hist[0]);
}

int epid(char **args)
{
  printf("%d", (int)getpid());
}

int ehistn(char **args)
{
  int n=args[0][5]-'0';
  for(int i=0;i<n;i++)
  {
    printf("%d. %s",i,hist[i]);
  }
}

int pidall()
{
  FILE * fptr=fopen("pidhist.txt","r");
  char str1[100];
  while(fscanf(fptr, "%[^\n]", str1)==1)
  {
    char *token = strtok(str1," ");
    char pname[20];
    strcpy(pname,token);
    token=strtok(NULL," ");
    char pid[20];
    strcpy(pid,token);


    printf("command name : %s   process id : %s\n",pname,pid);
    fseek(fptr, SEEK_CUR, SEEK_SET+1);
  }
  fclose(fptr);
  return 1;
}

int pidc()
{
  struct current_pids *loop_node;

	loop_node = head_pid;

	while(loop_node != NULL)
  {
		printf("PID:\t%d\n", loop_node->pid);
		loop_node = loop_node->next;
	}
  return 1;
}

int redirect(char *line)
{
    if(strstr(line,"<")!=NULL)
    {
       if(strstr(line,"<<")!=NULL)
       {
           return 2;
       }
        return 1; 
    }
    else if(strstr(line,">")!=NULL)
    {
        if(strstr(line,">>")!=NULL)
        {
            return 4;
        }
        return 3;
    }
    else
    {
        return -1;
    }   
}

char ** redirects(char *line)//to split the line into arguments to be executed and the file name
{
    int bufsize = LSH_TOK_BUFSIZE, p = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *tok;
  #define DELIMR "<>"

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  tok = strtok(line, DELIMR);
  while (tok != NULL) {
    tokens[p] = tok;
    p++;

    if (p >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    tok = strtok(NULL, DELIMR);
  }
  tokens[p] = NULL;
  return tokens;
}

void output(char **args,char *fn,int t)
{
    int out = open(fn, O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == out) { perror("opening cout.log"); return 255; }

    int err = open("cerr.log", O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == err) { perror("opening cerr.log"); return 255; }

    int save_out = dup(fileno(stdout));
    int save_err = dup(fileno(stderr));

    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    if (-1 == dup2(err, fileno(stderr))) { perror("cannot redirect stderr") ; return 255; }

    // puts("doing an ls or something now");
    if (execvp(args[0], args) == -1) {
      perror("rsh");
    }
    exit(EXIT_FAILURE);
    fflush(stdout); close(out);
    fflush(stderr); close(err);

    dup2(save_out, fileno(stdout));
    dup2(save_err, fileno(stderr));

    close(save_out);
    close(save_err);
    
}

void input(char **args,char *fn,int t)
{
    int in=open(fn, O_RDONLY);
    if (in == -1)
    {
      perror("opening cout.log");
     }

    if (-1 == dup2(in, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    //if (-1 == dup2(err, fileno(stderr))) { perror("cannot redirect stderr") ; return 255; }
    if ((execvp(args[0], args) == -1)) {
      perror("rsh");
    }
 

   /* cat < infile */

   
}


int start(char **args,char *fn, int t)
{
  pid_t pid, wpid;
  int status;
  FILE *fptr;
  fptr =fopen("pidhist.txt","a");
  int i=0,bg=0;
  if(strncmp(args[0],"&",1)==0)
  {
    args[0]=NULL;
    bg=1;
  }

  pid = fork();
  if (pid == 0) {
    // Child process
    if(t==3||t==4)
    {
        output(args,fn,t);
    }
    else if(t==1)
    {
        input(args,fn,t);
    }
    else if (execvp(args[0], args) == -1) 
    {
      perror("execution error");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("error");
  } else {
    // Parent process
    fprintf(fptr,"%s %d\n",args[0],pid);
    fclose(fptr);
    if(bg>0)
    {
      insert_pid(pid);
    }
    else
    {
      wait(NULL);
    }
    //wait(NULL);
  }

  return 1;
}

int execute(char **args,char *fn,int t)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  if((strncmp(args[0],"pid",strlen("pid"))==0 )&&(args[1]!=NULL) &&(strncmp(args[1],"all",strlen("all"))==0))
    return pidall();
  if((strncmp(args[0],"pid",strlen("pid"))==0 )&&(args[1]!=NULL)&& (strncmp(args[1],"current",strlen("current"))==0))
    return pidc();

  for (i = 0; i < num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  if(strstr(args[0],"!hist")!=NULL)
  {return ehistn(args);}
  
  
  
  /*if(strncmp(args[0],"exit",strlen("exit"))==0)
  {return eexit(args);}*/

  

  return start(args,fn,t);
}

int pexecute(char *line)
{
    char **args;// to store the instructions after tokenising
  char **tempr;
  char *fn;
    int t=redirect(line); // checking if there is < or > in the command
        if(t==-1)
        {
            args = split(line);
            execvp(args[0],args);
            //printf("%d",redirect(line));
        }
        else
        {
            
            tempr=redirects(line);
            args=split(tempr[0]);
            fn=tempr[1];
            //printf("%d \n",t);
            //printf("%s \n",tempr[1]);
            if(t==3||t==4)
            {
              output(args,fn,t);
            }
            else if(t==1)
            {
              input(args,fn,t);
            }
        }   
}

int pexec(char *line)
{
  char **args;// to store the instructions after tokenising
  char **tempr;
  char *fn;
  args = split(line);
  execvp(args[0],args);
  
}

int pstart(char **pipecommand,int p)
{
    int numPipes =p;
    char **args;
    int cp=0;
    int status;
    int i = 0;
    pid_t pid;

    int pipefds[2*numPipes];

    for(i = 0; i < (numPipes); i++){
        if(pipe(pipefds + i*2) < 0) {
            perror("couldn't pipe");
            exit(EXIT_FAILURE);
        }
    }


    int j = 0;
   while(cp<=p)
     {
        pid = fork();
        if(pid == 0) 
        {

            //if not last command
            if(cp!=p){
                if(dup2(pipefds[j + 1], 1) < 0){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            //if not first command&& j!= 2*numPipes
            if(j != 0 ){
                if(dup2(pipefds[j-2], 0) < 0){
                    perror(" dup2");///j-2 0 j+1 1
                    exit(EXIT_FAILURE);

                }
            }


            for(i = 0; i < 2*numPipes; i++){
                    close(pipefds[i]);
            }
            //args=split(pipecommand[cp]);
           // printf("%s \n",args[0]);
            
            
            if( pexec(pipecommand[cp]) < 0 ){
                    perror(args[0]);
                    exit(EXIT_FAILURE);
            }
        } 

        

        else if(pid < 0){
            perror("error");
            exit(EXIT_FAILURE);
        }

       cp++;
       j+=2;
      
    }
    /**Parent closes the pipes and wait for children*/

    for(i = 0; i < 2 * numPipes; i++){
        close(pipefds[i]);
    }

    for(i = 0; i < numPipes + 1; i++)
        wait(&status);

}



int pipecheck(char *line)
{
    if(strstr(line,"|")!=NULL)
    {
        //printf("1 \n");
    return 1;
    }
    else
    {
        //printf("-1 \n");
        return -1;
    }
    
}

char ** pipesplit(char *line) //splits line wrt |
{
    int bufsize = LSH_TOK_BUFSIZE, p = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *tok;
 

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  tok = strtok(line, "|");
  while (tok != NULL) {
    tokens[p] = tok;
    p++;

    if (p >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    tok = strtok(NULL,"|");
  }
  tokens[p] = NULL;
  return tokens;
}


void sigchld_handler(int signum)
{
  /*pid_t pid;
  while ( (pid = waitpid(-1, NULL, WNOHANG)) != -1){
		delete_pid(pid);
		printf("\n  [~BG %d]\n", pid);
		//printf("%s@%s:~%s/ ", username, nodename, disp_dir());
  }*/
  pid_t pid;
	pid = waitpid(-1, NULL, WNOHANG);

	if ((pid != -1) && (pid != 0)){
		//printf("  [~%d]\n", pid);
		delete_pid(pid);
	}
	return;
}


void lsh_loop(void)
{
  
  char *line;//to store what to execute before tokenising
  char **args;// to store the instructions after tokenising
  char **tempr;//to store the string tokenised with<,>
  char **pipet;//to store string tokenised with |
  char *fn; //to store the location
  //char *cmd="cmd";
  int status=1;
  int t,pipen;
  
  
  //cfptr = fopen("commandslist.txt","w+");
  //fprintf(cfptr,"1");


  //signal(SIGCHLD,sigchld_handler);

  while (status) {
    char s[100];
    printf("cmd %s~",getcwd(s, 100));
    line = readline();
    strcpy(hist[h],line);
    //hist[h]=line;
    //printf("%s",hist[h]);
    h++;
    pipen=pipecheck(line); //check if | is there
    int p=0;

    if(pipen==1)
    {
        pipet=pipesplit(line);
        //printf("1 \n");
        while(pipet[p]!=NULL)
        {
            p++;
        }
        pstart(pipet,p);
    }
    else
    {
        //pipet=malloc(2* sizeof(char*));
        //pipet[0]=line;
    
        //printf("2 \n");
    
        t=redirect(line); // checking if there is < or > in the command
        if(t==-1)
        {
            args = split(line);
            //printf("%d",redirect(line));
        }
        else
        {
            
            tempr=redirects(line);
            args=split(tempr[0]);
            fn=tempr[1];
            printf("%d \n",t);
            printf("%s \n",tempr[1]);
        }   
    
    
    
        status=execute(args,fn,t);
        
    }
    
    //args = split(line);
    
    free(line);
    free(args);
    
  } //while (status);
  fclose(cfptr);
    //remove("commandslist.txt");
}



int main(int argc, char **argv)
{
  //int bufsize = LSH_TOK_BUFSIZE;
  //hist = malloc(bufsize * sizeof(char*));

  head_pid = (struct current_pids *)malloc(sizeof(struct current_pids));
	head_pid->pid = getpid();
	head_pid->next = NULL;
	signal(SIGCHLD, sigchld_handler);
  
  lsh_loop();
  return EXIT_SUCCESS;
}

