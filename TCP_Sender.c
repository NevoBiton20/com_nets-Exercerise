#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <time.h>
#include "Head.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define BUFFER_SIZE 1024

void send_file(char buffer[1024], int sockfd)
{
    const int file_Size = 2*1024*1024; 
    if (send(sockfd, &file_Size, sizeof(file_Size), 0) == -1) {
        perror("ERROR writing to socket");
        exit(1);
    }

    for (int i = 0; i < file_Size / BUFFER_SIZE; i++)
    {
        memset(buffer, 'A', BUFFER_SIZE);
        ssize_t n = write(sockfd, buffer, BUFFER_SIZE);
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    int sockfd, PORT= atoi(argv[4]);//socket
    struct sockaddr_in serv_addr;

    char buffer[2 * 1024];
    char *ip = argv[2];
    char *algo = argv[6];


    if(argc!=7) //args check
    {
        fprintf(stderr, "Usage: %s -ip IP -p PORT -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1],"-ip")!=0 || strcmp(argv[3],"-p")!=0 || strcmp(argv[5],"-algo")!=0) //args check
    {
        fprintf(stderr, "Usage: %s -ip IP -p PORT -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (ip == NULL || PORT == 0) {
        fprintf(stderr, "Usage: %s -ip IP -p PORT -algo ALGO\n", argv[0]);
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
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address format\n");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    do {
        send_file(buffer, sockfd);

        printf("File sent. Send again? (y/n): ");
        scanf("%s", buffer);

        long n = write(sockfd, buffer, BUFFER_SIZE);
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }
    } while (strcmp(buffer, "n") != 0);

    close(sockfd);
    printf("Connection closed.\n");

    return 0;
}
