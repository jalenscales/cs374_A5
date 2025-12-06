#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void send_all(int socket, const void *buffer, size_t length) {
    size_t total_sent = 0;
    const char *ptr = buffer;
    while (total_sent < length) {
        ssize_t sent = send(socket, ptr + total_sent, length - total_sent, 0);
        if (sent == -1) error("Error sending data");
        total_sent += sent;
    }
}

void recv_all(int socket, void *buffer, size_t length) {
    size_t total_received = 0;
    char *ptr = buffer;
    while (total_received < length) {
        ssize_t received = recv(socket, ptr + total_received, length - total_received, 0);
        if (received <= 0) error("Error receiving data (socket closed or failed)");
        total_received += received;
    }
}

char decrypt_char(char c, char k) {
    int c_val = (c == ' ') ? 26 : (c - 'A');
    int k_val = (k == ' ') ? 26 : (k - 'A');
    int p_val = c_val - k_val;
    if (p_val < 0) {
        p_val += 27;
    }
    
    return (p_val == 26) ? ' ' : (p_val + 'A');
}

void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void setupAddressStruct(struct sockaddr_in *address, int portNumber) {
    memset((char *)address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

void handle_client(int connectionSocket) {
    char auth_buffer[16];
    memset(auth_buffer, 0, sizeof(auth_buffer));
    ssize_t charsRead = recv(connectionSocket, auth_buffer, sizeof(auth_buffer) - 1, 0);
    if (charsRead < 0) error("ERROR reading from socket");

    if (strcmp(auth_buffer, "DEC_REQ") != 0) {
        char *reject = "REJECT";
        send(connectionSocket, reject, strlen(reject), 0);
        fprintf(stderr, "Error: Client rejected (wrong ID: %s)\n", auth_buffer);
        close(connectionSocket);
        exit(2); 
    }
    
    char *ack = "ACCEPT";
    send_all(connectionSocket, ack, strlen(ack));

    int text_length;
    recv_all(connectionSocket, &text_length, sizeof(text_length));
    
    char *ciphertext = malloc(text_length + 1);
    recv_all(connectionSocket, ciphertext, text_length);
    ciphertext[text_length] = '\0';

    char *key = malloc(text_length + 1);
    recv_all(connectionSocket, key, text_length);
    key[text_length] = '\0';

    char *plaintext = malloc(text_length + 1);
    for (int i = 0; i < text_length; i++) {
        if (ciphertext[i] == '\n') {
            plaintext[i] = '\n';
            continue;
        }
        plaintext[i] = decrypt_char(ciphertext[i], key[i]);
    }
    plaintext[text_length] = '\0';
    send_all(connectionSocket, plaintext, text_length);
    free(plaintext);
    free(key);
    free(ciphertext);
    close(connectionSocket);
    exit(0);
}

int main(int argc, char *argv[]) {
    int listenSocket, connectionSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo;

    if (argc < 2) { 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1); 
    }
    listenSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (listenSocket < 0) error("ERROR opening socket");
    int yes = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        error("ERROR on binding");

    listen(listenSocket, 5); 
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    while (1) {
        sizeOfClientInfo = sizeof(clientAddress); 
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0) error("ERROR on accept");
        pid_t pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        } 
        if (pid == 0) {
            close(listenSocket); 
            handle_client(connectionSocket);
        } else {
            close(connectionSocket);
        }
    }
    close(listenSocket);
    return 0; 
}
//u