/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wiringPi.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int fd;
     int i;
     int a2dChannel = 0; //analog channel
     int a2dVal;
     float a2dVol;
     float Vref = 3.3;

     wiringPiSetup () ;
     pinMode (0, OUTPUT) ;

     int sockfd, newsockfd, portno;
     socklen_t clilen;
     double msg1 = 0;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     if ((fd = wiringPiI2CSetup(0x48)) < 0) {
     printf("wiringPiI2CSetup failed:\n");
     }

     int ref_val = 0;
     int total_ref_measurements = 50; 
     for(i=0;i<total_ref_measurements;i++) {
      wiringPiI2CWrite(fd,0x40 | a2dChannel);
      a2dVal = wiringPiI2CRead(fd); // Previously byte
      a2dVal = wiringPiI2CRead(fd);
      a2dVol = a2dVal * Vref / 255;
      // printf("a2dVal=%d   -  ",abs(a2dVal - 128));
      //printf("a2dVol=%f[V]\n",a2dVol);
      ref_val += abs(a2dVal - 128);
      //sleep(0.1);
     }
     
     ref_val = ref_val / total_ref_measurements;
     printf("ref_val=%d \n",ref_val);

     // create a socket
     // socket(int domain, int type, int protocol)
     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

     // clear address structure
     bzero((char *) &serv_addr, sizeof(serv_addr));

     portno = atoi(argv[1]);

     /* setup the host_addr structure for use in bind call */
     // server byte order
     serv_addr.sin_family = AF_INET;  

     // automatically be filled with current host's IP address
     serv_addr.sin_addr.s_addr = INADDR_ANY;  

     // convert short integer value for port must be converted into network byte order
     serv_addr.sin_port = htons(portno);

     // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
     // bind() passes file descriptor, the address structure, 
     // and the length of the address structure
     // This bind() call will bind  the socket to the current IP address on port, portno
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     // This listen() call tells the socket to listen to the incoming connections.
     // The listen() function places all incoming connection into a backlog queue
     // until accept() call accepts the connection.
     // Here, we set the maximum size for the backlog queue to 5.
     listen(sockfd,5);

     // The accept() call actually accepts an incoming connection
     clilen = sizeof(cli_addr);

     // This accept() function will write the connecting client's address info 
     // into the the address structure and the size of that structure is clilen.
     // The accept() returns a new socket file descriptor for the accepted connection.
     // So, the original socket file descriptor can continue to be used 
     // for accepting new connections while the new socker file descriptor is used for
     // communicating with the connected client.
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

     //     printf("server: got connection from %s port %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));


     // This send() function sends the 13 bytes of the string to the new socket
     //send(newsockfd, "Hello, world!\n", 13, 0);

     struct timespec peer_time;
     struct timespec raw_time;
     time_t seconds_offset;
     long nanos_offset;
     
     n = read(newsockfd,&peer_time,sizeof(peer_time));
     if (n < 0) error("ERROR reading from socket");

     a2dVal = 0;
     while(a2dVal < ref_val + 20){
      wiringPiI2CWrite(fd,0x40 | a2dChannel);
      a2dVal = wiringPiI2CRead(fd); // Previously byte
      a2dVal = wiringPiI2CRead(fd);
      a2dVal = abs(a2dVal - 128);
     }

     clock_gettime(CLOCK_MONOTONIC_RAW, &raw_time);

     seconds_offset = peer_time.tv_sec - raw_time.tv_sec;
     nanos_offset = peer_time.tv_nsec - raw_time.tv_nsec;
     //printf("Here is the message: %ld\n",tmpBuff);
    
     // blink 2 seconds after negotiated time

     peer_time.tv_sec += 1;

     // subtract additional 90ms for calibration
     // TODO: VERY BAD!! MAY FAIL IF VALUE GOES NEGATIVE
     peer_time.tv_nsec -= 9e+7;

    while(peer_time.tv_sec - (raw_time.tv_sec + seconds_offset) >= 0 || peer_time.tv_nsec >= (raw_time.tv_nsec + nanos_offset)){ // wait until trigger time

        clock_gettime(CLOCK_MONOTONIC_RAW, &raw_time);
    }

     digitalWrite (0, HIGH) ;
     sleep(1);
     digitalWrite (0,  LOW) ;

     
     

     close(newsockfd);
     close(sockfd);
     return 0; 
}//
