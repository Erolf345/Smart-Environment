#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <wiringPi.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    wiringPiSetup () ;
    pinMode (0, OUTPUT) ;


    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
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
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    int delay_seconds = 1;
    struct timespec rawtime;
    struct timespec trigger_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &trigger_time);
    trigger_time.tv_sec += delay_seconds; // add delay
    printf("clock: %ld\n",trigger_time.tv_nsec);
    printf("pointer: %p\n",&trigger_time);
    n = write(sockfd, &trigger_time, sizeof(trigger_time));
    if (n < 0) 
         error("ERROR writing to socket");
    
    printf("now waiting to trigger\n");
    while(trigger_time.tv_sec - rawtime.tv_sec >= 0 || trigger_time.tv_nsec >= rawtime.tv_nsec){ // wait until trigger time
        clock_gettime(CLOCK_MONOTONIC_RAW, &rawtime);
    }
    printf("playing audio!\n");
    system("play -n -c1 synth 1 sine 1000");


    // blink 2 seconds after negotiated time
    trigger_time.tv_sec += 2;
    while(trigger_time.tv_sec - rawtime.tv_sec >= 0 || trigger_time.tv_nsec >= rawtime.tv_nsec){ // wait until trigger time
        clock_gettime(CLOCK_MONOTONIC_RAW, &rawtime);
    }
    
    digitalWrite (0, HIGH) ; 
	sleep(1);
    digitalWrite (0,  LOW) ;


    close(sockfd);
    return 0;
}
