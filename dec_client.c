#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define BUFFER_SIZE 4096

int validate_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') break; 
        if (c != ' ' && (c < 'A' || c > 'Z')) {
            fclose(fp);
            return 0; 
        }
    }
    fclose(fp);
    return 1; 
}
int read_file(const char *filename, char *buffer, int max_len) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;

    if (!fgets(buffer, max_len, fp)) {
        fclose(fp);
        return -1;
    }

    int len = strlen(buffer);
    if (buffer[len-1] == '\n') buffer[len-1] = '\0';
    fclose(fp);
    return strlen(buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s ciphertext key port\n", argv[0]);
        return 1;
    }
    char *ciphertext_file = argv[1];
    char *key_file = argv[2];
    int port = atoi(argv[3]);
    if (validate_file(ciphertext_file) != 1) {
        fprintf(stderr, "dec_client error: input contains bad characters\n");
        return 1;
    }
    if (validate_file(key_file) != 1) {
        fprintf(stderr, "dec_client error: key contains bad characters\n");
        return 1;
    }
    char ciphertext[BUFFER_SIZE];
    char key[BUFFER_SIZE];
    int ct_len = read_file(ciphertext_file, ciphertext, BUFFER_SIZE);
    int key_len = read_file(key_file, key, BUFFER_SIZE);
    if (ct_len < 0 || key_len < 0) {
        fprintf(stderr, "dec_client error: cannot read files\n");
        return 1;
    }
    if (key_len < ct_len) {
        fprintf(stderr, "Error: key '%s' is too short\n", key_file);
        return 1;
    }
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 2;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error: could not contact dec_server on port %d\n", port);
        return 2;
    }
    if (send(sock_fd, ciphertext, ct_len, 0) < 0) {
        perror("send ciphertext");
        close(sock_fd);
        return 2;
    }

    if (send(sock_fd, key, ct_len, 0) < 0) { 
        perror("send key");
        close(sock_fd);
        return 2;
    }
    char plaintext[BUFFER_SIZE];
    int bytes_received = recv(sock_fd, plaintext, sizeof(plaintext)-1, 0);
    if (bytes_received < 0) {
        perror("recv");
        close(sock_fd);
        return 2;
    }
    plaintext[bytes_received] = '\0';
    printf("%s\n", plaintext);
    close(sock_fd);
    return 0;
}
