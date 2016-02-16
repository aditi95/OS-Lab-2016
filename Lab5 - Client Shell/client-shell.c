#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int c_id,p_id[64];
pid_t fg = 0, bg = 0;
int count = 0;
char server_ip[100], server_port[100];
int parallel = 0;
char **tokenize(char *line)
{
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for(i =0; i < strlen(line); i++){

        char readChar = line[i];
        if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
            token[tokenIndex] = '\0';
            if (tokenIndex != 0){
	           tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	           strcpy(tokens[tokenNo++], token);
	           tokenIndex = 0; 
            }
        } else {
            token[tokenIndex++] = readChar;
        }
  }
 
    free(token);
    tokens[tokenNo] = NULL ;
    return tokens;
}

void sig_handler(int signum)    //signal handling function
{

    if(signum == SIGINT) //kill foreground process group
      if(fg > 0) 
        {
            
          if(killpg(fg, SIGINT) == 0)
          {
            printf("Foreground processes were killed\n");
          }
          else perror("Error ");
          fg = 0;

        }
    return ;
}

void  main(void)
{
    signal(SIGINT, sig_handler);   //call to signal handler
    bg = getpid();
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i;
    while (1) {           
       
        printf("Hello>");
        int k, stat;   
        while((k = waitpid(-1,&stat, WNOHANG)) > 0)
        {
            printf("%d done!!\n",k );
        }  
        bzero(line, MAX_INPUT_SIZE);
        gets(line);           
        //printf("Got command %s\n", line);
        line[strlen(line)] = '\n'; //terminate with new line
        tokens = tokenize(line);
        if(tokens[0] == NULL)continue;            
       //do whatever you want with the commands, here we just print them

       /*for(i=0;tokens[i]!=NULL;i++){
	       printf("found token %s\n", tokens[i]);
       }
       */ if(strcmp(tokens[0], "server") == 0)  //command for setting server IP and port
        {
        
          strcpy(server_ip, tokens[1]);
          strcpy(server_port, tokens[2]);
          //printf("%s sajkasjaks  %s\n",server_ip, tokens[1] );
        }
        else if(strcmp(tokens[0], "getfl") == 0) //get files from server
        {
            //printf("In getfl\n");
            if(tokens[1] == NULL)
            {
                printf("Not a valid command.\n");
                continue;
            }
            if(tokens[2] == NULL)
            {
                    c_id = fork();
                    if(fg == 0)
                    {
                        setpgid(c_id, 0);
                        fg = c_id;
                    }
                    else setpgid(c_id, fg);
                // printf("%d\n",c_id );
                if( c_id == 0)
                {
                    //printf("ajkashkjlflajf %s %s\n",tokens[2], tokens[3] );

                    char s[100] = "./client";
                    char *params[6];
                    params[0] = s;
                    params[1] = tokens[1];
                    params[2] = server_ip;
                    params[3] = server_port;
                    params[4] = "display";
                    params[5] = NULL;
                    //printf("%s\n", s);
                    //printf("ajkashkjlflajf %s %s\n",params[2], params[3] );
                    execvp(s,params);
                    perror("Error ");
                    return 0;
                }
                else 
                {
                    int k;
                    waitpid(c_id, &k,0);
                    //printf("out of wait\n");
                    c_id = 0;
                    fg= 0;
                }

            }
            else if(strcmp(tokens[2], ">") == 0 ) //output redirection
            {
                if(tokens[3]!=NULL && tokens[4] == NULL)

                {
                     c_id = fork();
                     if(fg == 0)
                    {
                        setpgid(c_id, 0);
                        fg = c_id;
                    }
                    else setpgid(c_id, fg);
                // printf("%d\n",c_id );
                if( c_id == 0)
                {
                    int fd = open(tokens[3], O_WRONLY |O_CREAT| O_TRUNC);
                    close(1);
                    dup(fd);
                    close(fd);
                        

                    char s[100] = "./client";
                    char *params[6];
                    params[0] = s;
                    params[1] = tokens[1];
                    params[2] = server_ip;
                    params[3] = server_port;
                    params[4] = "display";
                    params[5] = NULL;

                    execvp(s,params);
                    perror("Error ");
                    return 0;

                }
                else 
                {
                    int k;
                    waitpid(c_id, &k,0);
                    c_id = 0;
                    fg= 0;
                }

                }
                else {
                    printf("Not a valid command.\n");
                }
            }
            else if(strcmp(tokens[2], "|") == 0) //pipe
            {
                if(tokens[3] != NULL)
                {
                    int p[2];
                    int p1,p2;
                    if(pipe(p) < 0)
                        printf("Pipe not opened\n");
                    if(p1 = fork() == 0){
                        if(fg == 0)
                        {
                            setpgid(p1, 0);
                            fg = p1;
                        }
                        else setpgid(p1, fg);
                        //printf("%d\n",p1 );
                        close(1);
                        dup(p[1]);
                        close(p[0]);
                        close(p[1]);
                        char s[100] = "./client";
                        char *params[6];
                        params[0] = s;
                        params[1] = tokens[1];
                        params[2] = server_ip;
                        params[3] = server_port;
                        params[4] = "display";
                        params[5] = NULL;
                        //printf("%s\n", s);
                        //printf("ajkashkjlflajf %s %s\n",params[2], params[3] );
                        execvp(s,params);
                        perror("Error ");
                        return 0;
                        }
                    if(p2 =fork() == 0){
                        if(fg == 0)
                        {
                            setpgid(p2, 0);
                            fg = p2;
                        }
                        else setpgid(p2, fg);
                        //printf("%d\n",p2 );
                    close(0);
                    dup(p[0]);
                    close(p[0]);
                    close(p[1]);
                    char ** params = &tokens[3];
                    char s[100];
                    strcpy(s, "/bin/");
                    strcat(s,params[0]);
                    execvp(s,params);
                    perror("Error ");
                    char buffer[512];
                    int n;
                    while(n = read(0,buffer, 511) > 0){bzero(buffer,512);}
                    return 0;

                    }
                    else{
                    close(p[0]);
                    close(p[1]);
                    int k1, k2;
                    waitpid(p1,&k1,0);
                    waitpid(p2,&k2,0);
                    //wait(&k1);
                    //wait(&k2);
                    fg= 0;
                }


                }

                else{
                    printf("Not a valid command.\n");
                }
            }
        }
        else if(strcmp(tokens[0], "getbg") == 0) //download in background
        {
            if(tokens[1] == NULL)
            {
                printf("Not a valid command.\n");
                continue;
            }
            count ++;
            c_id = fork();
            setpgid(c_id, bg);
            if( c_id == 0)
            {
                if(tokens[2] != NULL)
                {
                    if(tokens[3] != NULL && tokens[4] == NULL)
                    {
                        int fd = open(tokens[3], O_WRONLY |O_CREAT);
                        close(1);
                        dup(fd);
                    }
                    else printf("Error in command\n");
                }
    
                char s[100] = "./client";
                char *params[6];
                params[0] = s;
                params[1] = tokens[1];
                params[2] = server_ip;
                params[3] = server_port;
                params[4] = "nodisplay";
                    params[5] = NULL;
                //printf("%s\n", s);
                //printf("ajkashkjlflajf %s %s\n",params[2], params[3] );
                execvp(s,params);
                perror("Error ");
                return 0;
           }
           // no need to wait
           /*else 
            {
              printf("Blocking call 3 \n");
              int k;
              waitpid(c_id, &k,0);
              printf("%d\n",k );
              c_id = 0;
            }
           */ 
        
        }
        else if(strcmp(tokens[0], "getpl") == 0) //parallel download
        {
            if(tokens[1] == NULL)
            {
                printf("Not a valid command.\n");
                continue;
            }
            //printf("In getpl\n");
            int i=1;
            pid_t pid = getpid();
            while(tokens[i]!=NULL){
                p_id[i] = fork();
                if(fg == 0)
                {
                    setpgid(p_id[i], 0);
                    fg = p_id[i];
                }
                else setpgid(p_id[i], fg);
                //printf("%d\n",p_id );
                if( p_id[i] == 0)
                {
                    //printf("Errorcmdmlkamvdkl\n");
                    char s[100] = "./client";
                    char *params[6];
                    params[0] = s;
                    params[1] = tokens[i];
                    params[2] = server_ip;
                    params[3] = server_port;
                    params[4] = "nodisplay";
                    params[5] = NULL;
                    //printf("%s\n", s);
                    //printf("ajkashkjlflajf %s %s\n",params[2], params[3] );
                    execvp(s,params);
                    perror("Error ");
                    return 0;
                }
                i++;
            }
            
            int c;
            for(c =1;c<i;c++){
                //printf("Blocking call while %d\n",c);
                int k;
                waitpid(p_id[c], &k,0);
                //printf("%d\n",k );
                p_id[c] = 0;
            }
            fg = 0;
        }
        
        else if(strcmp(tokens[0], "getsq") == 0) //sequential download
        {
            if(tokens[1] == NULL)
            {
                printf("Not a valid command.\n");
                continue;
            }
            //printf("In getsq\n");
            int i=1;
            while(tokens[i]!=NULL){
            c_id = fork();
            if(fg == 0)
                {
                    setpgid(c_id, 0);
                    fg = c_id;
                }
                else setpgid(c_id, fg);
            setpgid(c_id, 200000);
            //printf("%d\n",c_id );
           if( c_id == 0)
           {
                //printf("Errorcmdmlkamvdkl\n");

                    
                char s[100] = "./client";
                char *params[6];
                params[0] = s;
                params[1] = tokens[i];
                params[2] = server_ip;
                params[3] = server_port;
                params[4] = "nodisplay";
                params[5] = NULL;

                //printf("%s\n", s);
                //printf("ajkashkjlflajf %s %s\n",params[2], params[3] );
                execvp(s,params);
                perror("Error ");
                return 0;
           }
           else 
            {
              //printf("Blocking call 1\n");
              int k;
              waitpid(c_id, &k,0);
              //printf("%d\n",k );
              c_id = 0;
              fg = 0;
            }
            i++;
        }
        }


        else if(strcmp(tokens[0], "cd") == 0) //cd command
        {
            //do cd??
            if(tokens[2] != NULL || tokens[1] == NULL){
                fprintf(stderr,"Not a valid command\n");
                continue;
            }
            int x = chdir(tokens[1]);
            if(x != 0)perror("Error ");
        }
        else if(strcmp(tokens[0], "exit") == 0) //kill all background processes and exit
        {
                killpg(bg, SIGINT);
                int i;
                for(i = 0; i <count; i++)
                    wait();

                exit(0);
        }
        else {
            c_id = fork();
            if(fg == 0)
                {
                    setpgid(c_id, 0);
                    fg = c_id;
                }
                else setpgid(c_id, fg);
            //printf("%d\n",c_id );
            if( c_id == 0) //assuming some inbuilt command
            {
                pid_t pid = getpid();
                //printf("This process id is %d\n", pid);
                char s[100];
                strcpy(s, "/bin/");
                strcat(s,tokens[0]);
                execvp(s,tokens);
                perror("Error ");
                return 0;
            }
            else 
            {
                //printf("Blocking call 2 \n");
                int k;
                waitpid(c_id, &k,0);
                //printf("%d\n",k );
                c_id = 0;
                fg = 0;
            }
        }
       // Freeing the allocated memory	
        for(i=0;tokens[i]!=NULL;i++){
	       free(tokens[i]);
        }
        free(tokens);
    }
}

                
