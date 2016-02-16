#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

int count[100];
float total_time[100];
int argc, duration, sleep_time;
char ** argv;
int mode;
int j;
void error(char *msg)
{
    perror(msg);
    exit(0);
}

	
void getFiles(void * id1){
	//printf("Thread id %lu\n", pthread_self());

    int id = (int)id1;
    time_t begin;

    begin = time(NULL);
    while(1){
        time_t seconds;
        struct timeval start, end;
       

        seconds = time(NULL);
        if(seconds - begin >= duration)  //checking if duration exceeded
            {
                //printf("Thread over\n");
                break;
            }

        int x;
        if(mode == 0)x = 0; //fixed mode
    	else //random mode
        {
            srand(time(NULL));
            int r = rand();
            x = r % 10000; 
        }
    	char s[1];
    	sprintf(s, "%d", x);
        int sockfd, portno, n;

        struct sockaddr_in serv_addr;
        struct hostent *server;

        char buffer[512];
        strcpy(buffer,"get files/foo");
        strcat(buffer,s);
        strcat(buffer,".txt");
        //printf("buffer %s",buffer);
        if (argc < 3) {
           fprintf(stderr,"usage %s hostname port\n", argv[0]);
           exit(0);
        }

        /* create socket, get sockfd handle */
        portno = atoi(argv[2]);
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
            error("ERROR opening socket");

        /* fill in server address in sockaddr_in datastructure */

        server = gethostbyname(argv[1]);
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
             (char *)&serv_addr.sin_addr.s_addr,
             server->h_length);
        serv_addr.sin_port = htons(portno);

        /* connect to server */

        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
            error("ERROR connecting");

        gettimeofday(&start,NULL); //request sent


    	n = write(sockfd,buffer,strlen(buffer)); //send message to server
        if (n < 0){ 
             error("ERROR writing to socket");
             printf("ERROR writing to socket");
        }

        /* read reply from server */
    	strtok(buffer, "\n");
        
        bzero(buffer,512);
        while(n = read(sockfd,buffer,511) > 0) //read the file recieved
        {
    		
        }
        gettimeofday(&end,NULL);

        count[id] = count[id] + 1;
        total_time[id] += end.tv_sec - start.tv_sec + (1.0*end.tv_usec - 1.0*start.tv_usec)/1000000;

        shutdown(sockfd, 2);
        close(sockfd);// close socket
        sleep(sleep_time);
    }

    return 0;
}
int main(int argc0, char *argv0[])
{
	argc = argc0;
	argv = argv0;
	int ntimes = atoi(argv0[3]);
    duration = atoi(argv0[4]);
    sleep_time = atoi(argv0[5]);
    if(strcmp(argv0[6], "random") == 0) mode = 1;
    else mode = 0;
	int i;
	pthread_t *tid = malloc( ntimes * sizeof(pthread_t) );
	j = 0;

	for( i=0; i<ntimes; i++ ){
	   count[i] = 0;
       total_time[i] = 0;
		pthread_create( &tid[i], NULL, getFiles, (void*)i ); // create new thread

	}

	for( i=0; i<ntimes; i++ ) 
		pthread_join( tid[i], NULL ); //join threads
        int c = 0;
        float t = 0.0;	
    for( i =0; i < ntimes; i++){
        c += count[i];
        t += total_time[i];
    }
    printf("Done!\n");
    printf("throughput = %lf req/s\n",(1.0*c)/duration );
    printf("average response time = %lf sec\n",(1.0*t)/c );
    // printf("%d\n", c);
    // printf("%d %f %f\n", ntimes, (1.0*c)/duration ,(1.0*t)/c );
}	
