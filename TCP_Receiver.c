#include <arpa/inet.h>
#include <netinet/tcp.h> 
#include <unistd.h>
#include <time.h>
#include <sys/time.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define BUFFER_SIZE 1024
#define FILE_SIZE 2*1024*1024

int main(int argc, char *argv[]) {

    int sockfd, newsockfd, PORT= atoi(argv[2]);//socket
    socklen_t clilen; //socket
    struct sockaddr_in serv_addr, cli_addr;//socket

    char buffer[BUFFER_SIZE]; //buffer input

    char *algo = argv[4]; //congestion control algorithm

    struct timeval start_time, end_time; //time management
    float time_measurements[300];
    float bandwidth[300]; //time management

    int count = 0;
    float total_bytes_received = 0;
    float total_time = 0;
    float all_bytes = 0;
    
    if(argc!=5) //args check
    {
        fprintf(stderr, "Usage: %s -p PORT -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1],"-p")!=0 || strcmp(argv[3],"-algo")!=0) //args check
    {
        fprintf(stderr, "Usage: %s -p PORT -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (PORT == 0) {
        fprintf(stderr, "Usage: %s -p PORT -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) != 0) {
        perror("setsockopt TCP_CONGESTION failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }
    printf("start_timeing Receiver...\n");

    if(listen(sockfd, 5)==-1)
    {
        perror("ERROR listening");
        exit(EXIT_FAILURE);
    }
    
    printf("Waiting for TCP connection...\n");

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        perror("ERROR on accept");
        exit(EXIT_FAILURE);
    }
    printf("Sender connected, beginning to receive file...\n");        

    while(1) {
        gettimeofday(&start_time, NULL);  

        total_bytes_received = 0;
        float bytes_received = 0;

        while (total_bytes_received <= FILE_SIZE) {

            bytes_received = recv(newsockfd, buffer, sizeof(buffer), 0);

            total_bytes_received += bytes_received;

            if (bytes_received <= 0) //stopped receiving
            {
               break;      
            }
        }

        gettimeofday(&end_time, NULL);

        printf("File transfer completed.\n");

        float time_step = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0f + end_time.tv_usec) - start_time.tv_usec; // saved in ms
        time_step /= 1000.0f; // Convert to milliseconds, adjusted for float
        time_measurements[count] = time_step;
        total_time += time_step;

        float speed = (total_bytes_received / (1024.0f * 1024.0f)) / (time_step / 1000.0f); // Bandwidth in Mb/s

        bandwidth[count] = speed;
        all_bytes += total_bytes_received;
        printf("file transfer number: #%d\n finished", count);
        count++;
        
        ssize_t n = read(newsockfd, buffer, BUFFER_SIZE);
        if (n < 1 || strncmp(buffer, "N", 1) == 0 || strncmp(buffer, "n", 1) == 0)
        {
            break;
        } 

    } 

    printf("Sender sent exit message.\n");

    // statistics
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");

    //Individual files transfers prints
    for (int i = 0; i < count; i++) { 
        printf(" Run %d Data: Time=%.2fms, Speed=%.9fMB/s\n", i + 1, (double)time_measurements[i], (double)bandwidth[i]);
    }

    double average_time = (double)total_time / count;
    double average_badnwidth = (double)all_bytes / (1024.0 * 1024.0) / ((double)total_time / 1000.0); // MB/s 

    printf("-\n- Average time: %.3fms\n- Average bandwidth: %.3fMB/s\n", average_time, average_badnwidth);
    printf("----------------------------------\n");
    printf("Receiver exit.\n");

    close(newsockfd);
    close(sockfd);

    return 0;
}