#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>


int sockfd;
struct sockaddr_in address;
pthread_t senderThread;
pthread_t receiverThread;


void sender() {
    while (1) {
        char buffer[1024];
        printf("%s", buffer);
        fgets(buffer, sizeof(buffer), stdin);
        int sent = write(sockfd, buffer, strlen(buffer));
        if (sent == -1)  {
            fprintf(stderr, "Socket write error\n");
            return 1;
        }
    }
}

void receiver() {
    while (1) {
        char buffer[1024] = {0};
        if (read(sockfd, buffer, sizeof(buffer) - 1) == -1) {
            fprintf(stderr, "Socket read error\n");
            return 1;
        }
    }
}


int main() {
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == -1) {
        fprintf(stderr, "Failed to create socket\n");
        return 1;
    }

    bzero(&address, sizeof(address)); 
   
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    address.sin_port = htons(1337);
    
    if (connect(sockfd, &address, sizeof(address)) == -1) {
        fprintf(stderr, "Connection failure\n");
        return 1;
    }

    pthread_create(&receiverThread, NULL, receiver, NULL);
    pthread_create(&senderThread, NULL, sender, NULL);
    pthread_join(&senderThread, NULL);
    pthread_join(&receiverThread, NULL);

    close(sockfd);
}