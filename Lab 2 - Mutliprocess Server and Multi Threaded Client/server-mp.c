/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}
void clean()
{
	while(1){      //check after every 1 second for zombie processes	
		sleep(1);
		int id = 1; 
		while(1){
			id = wait(NULL);
			if(id <= 0)break;
			//printf("The process %d just got reaped\n", id);
		}
	}
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char buffer[512];                      //buffer for messages
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     /* create socket */

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

     /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     /* bind socket to this port number on this machine */

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     /* listen for incoming connection requests */
    pthread_t tid;                                 //create a thread for cleaning up zombies
	pthread_create( &tid, NULL, clean, NULL );
    
	while(1){

		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		/* accept a new request, create a newsockfd */
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			error("ERROR on accept");
			continue;
		}	
		
     
		/* fork off after accepting a new request*/
		int stat = fork();
		if(stat == 0){	 /* Child Process */
			pid_t pid = getpid();

		
			//printf("This process id is %d\n", pid);
			bzero(buffer,512);
			/* read message from client */
			n = read(newsockfd,buffer,511);
			if (n < 0) error("ERROR reading from socket");
			strtok(buffer, "\n");
			int fptr;
			char str[512];
			char name[512];
			strcpy(name, buffer+4);
			//printf("%s\n",name );
			fptr = open(name,O_RDONLY); 
			if(fptr < 0)
			{
				printf("Error in opening file");
				exit(1);
			}
			bzero(str,512);
			int coun = 0;
			while(n = read(fptr,str, 511) > 0){
				int n1 = write(newsockfd,str,strlen(str));
				if (n1 < 0) error("ERROR writing to socket");
			}
			close(fptr);                       //closing file
			//shutdown(newsockfd,2);                 //closing socket
			close(newsockfd);
			return 0 ;   
		}
		//shutdown(newsockfd, 2);
		close(newsockfd);
	}
     
     return 0; 
}



