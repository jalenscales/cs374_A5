#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#define BACKLOG 5
#define BUFFER_SIZE 4096

int char_to_num(char c) {
    if (c == ' ') return 26;
    return c - 'A';
}
char num_to_char(int n) {
    if (n == 26) return ' ';
    return 'A' + n;
}
void decrypt(char *ciphertext, char *key, char *plaintext) {
    int len = strlen(ciphertext);
    for (int i = 0; i < len; i++) {
        int c = char_to_num(ciphertext[i]);
        int k = char_to_num(key[i]);
        int p = (c - k + 27) % 27;  
        plaintext[i] = num_to_char(p);
    }
    plaintext[len] = '\0';
}

void handle_client(int client_fd) {
    char ciphertext[BUFFER_SIZE];
    char key[BUFFER_SIZE];
    char plaintext[BUFFER_SIZE];
    int bytes_received;

    bytes_received = recv(client_fd, ciphertext, sizeof(ciphertext)-1, 0);
    if (bytes_received <= 0) {
        perror("recv ciphertext");
        close(client_fd);
        exit(1);
    }
    ciphertext[bytes_received] = '\0';
    bytes_received = recv(client_fd, key, sizeof(key)-1, 0);
    if (bytes_received <= 0) {
        perror("recv key");
        close(client_fd);
        exit(1);
    }
    key[bytes_received] = '\0';
    decrypt(ciphertext, key, plaintext);
    int bytes_sent = send(client_fd, plaintext, strlen(plaintext), 0);
    if (bytes_sent < 0) {
        perror("send");
    }
    close(client_fd);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(1);
    }

    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(listen_fd, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }
    printf("dec_server listening on port %d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_size);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        if (fork() == 0) { 
            close(listen_fd);
            handle_client(client_fd);
        }
        close(client_fd); 
    }

    close(listen_fd);
    return 0;
}

