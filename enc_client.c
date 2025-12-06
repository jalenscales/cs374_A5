#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>


void error(const char *msg, int exit_code) {
    perror(msg);
    exit(exit_code);
}

void send_all(int socket, const void *buffer, size_t length) {
    size_t total_sent = 0;
    const char *ptr = buffer;
    while (total_sent < length) {
        ssize_t sent = send(socket, ptr + total_sent, length - total_sent, 0);
        if (sent == -1) error("Error sending data", 2);
        total_sent += sent;
    }
}

void recv_all(int socket, void *buffer, size_t length) {
    size_t total_received = 0;
    char *ptr = buffer;
    while (total_received < length) {
        ssize_t received = recv(socket, ptr + total_received, length - total_received, 0);
        if (received <= 0) error("Error receiving data (socket closed or failed)", 2);
        total_received += received;
    }
}

long get_file_content(char *filename, char **content) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *content = malloc(length + 1);
    if (*content == NULL) error("Memory allocation failed", 1);
    fread(*content, 1, length, fp);
    fclose(fp);
    if (length > 0 && (*content)[length - 1] == '\n') {
        (*content)[length - 1] = '\0';
        length--;
    } else {
        (*content)[length] = '\0';
    }

    return length;
}

void validate_chars(char *text, long length, const char *filename) {
    for (int i = 0; i < length; i++) {
        if (!((text[i] >= 'A' && text[i] <= 'Z') || text[i] == ' ')) {
            fprintf(stderr, "enc_client error: input contains bad characters in %s\n", filename);
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber;
    struct sockaddr_in serverAddress;
    struct hostent *serverHostInfo;
    char *plaintext = NULL;
    char *key = NULL;

    if (argc < 4) {
        fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
        exit(1);
    }

    long text_len = get_file_content(argv[1], &plaintext);
    long key_len = get_file_content(argv[2], &key);

    validate_chars(plaintext, text_len, argv[1]);
    validate_chars(key, key_len, argv[2]);
    if (key_len < text_len) {
        fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
        exit(1);
    }

    portNumber = atoi(argv[3]);
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) error("CLIENT: ERROR opening socket", 2);
    serverHostInfo = gethostbyname("localhost");
    if (serverHostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(2);
    }
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char*)serverHostInfo->h_addr, (char*)&serverAddress.sin_addr.s_addr, serverHostInfo->h_length);
    serverAddress.sin_port = htons(portNumber);

    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Error: could not contact enc_server on port %d\n", portNumber);
        exit(2);
    }

    char *id = "ENC_REQ";
    send_all(socketFD, id, strlen(id));

    char buffer[16];
    memset(buffer, 0, sizeof(buffer));
    recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
    if (strcmp(buffer, "ACCEPT") != 0) {
        fprintf(stderr, "Error: Connection rejected by server (Wrong Server?)\n");
        close(socketFD);
        exit(2);
    }

    int payload_size = (int)text_len;
    send_all(socketFD, &payload_size, sizeof(payload_size));
    send_all(socketFD, plaintext, text_len);
    send_all(socketFD, key, text_len);
    char *ciphertext = malloc(text_len + 1);
    recv_all(socketFD, ciphertext, text_len);
    ciphertext[text_len] = '\0';
    fprintf(stdout, "%s\n", ciphertext);
    close(socketFD);
    free(plaintext);
    free(key);
    free(ciphertext);

    return 0;
}
//u