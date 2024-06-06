#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stddef.h>

#define SERVER_ROOT "/home/dzmitry/Desktop/serverRoot"

struct connectionData {
    int working;
    int sockfd;
    char path[2048];
};

int sockfd;
struct sockaddr_in address;

char *ltrim(char *str) {
    while((*str) == ' ')
        str++;
    return str;
}

int startsWith(char *str, char *substr) {
    return !strncmp(str, substr, strlen(substr));
}

char *extractArgs(char *message) {
    char *args = strchr(message, ' ');
    return args ? args + 1 : "";
}

void handleMessage(struct connectionData *data, char *message) {
    printf("Got message: %s\n", message);
    char *args = extractArgs(message);

    if (startsWith(message, "ECHO")) {
        write(data->sockfd, args, strlen(args));
        printf("Sent ECHO: %s\n", args);
    }
    if (startsWith(message, "QUIT")) {
        char *message = "Bye bye.";
        write(data->sockfd, message, strlen(message));
        data->working = 0;
    }
}

void handleConnection(struct connectionData *data) {
    while (data->working) {
        char hint[1024] = {0};
        char buffer[1024] = {0};

        strcpy(hint, data->path);
        strcat(hint, "> ");

        write(data->sockfd, hint, strlen(hint));

        if (read(data->sockfd, buffer, sizeof(buffer)) == -1) {
            fprintf(stderr, "Socker read error");
        }
        printf(stderr, "Received message from %d: %s", data->sockfd, buffer);

        handleMessage(data, ltrim(buffer));
    }

    printf("Connection %d closed.\n", data->sockfd);
    free(data);
}

void shutdownServer() {
    printf("Shutting down...");
    close(sockfd);
    exit(0);
}

void launchServer() {
    struct sigaction act = {.sa_flags = 0, .sa_handler = shutdownServer};
    sigaction(SIGINT, &act, NULL);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == -1) {
        fprintf(stderr, "Failed to create socket\n");
        exit(1);
    }

    bzero(&address, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(1337);

    if (bind(sockfd, &address, sizeof(address)) == -1) {
        fprintf(stderr, "Failed to bind socket\n");
        exit(1);
    }

    if (listen(sockfd, 10) == -1) {
        fprintf(stderr, "Failed to listen socket\n");
        exit(1);
    }

    while (1) {
        int connection = accept(sockfd, NULL, NULL);
        if (connection == -1) {
            fprintf(stderr, "Connection failure\n");
            continue;
        }
        printf("Accepted connection: %d\n", connection);

        pthread_t thread;
        struct connectionData *data = malloc(sizeof(struct connectionData));
        data->sockfd = connection;
        data->working = 1;
        strcpy(data->path, "/");

        pthread_create(&thread, NULL, handleConnection, data);
    }
}

int main() {
    launchServer();
}