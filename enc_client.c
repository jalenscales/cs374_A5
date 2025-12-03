#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s plaintext key port\n", argv[0]);
        return 1;
    }
    char *plaintext_file = argv[1];
    char *key_file = argv[2];
    int port = atoi(argv[3]);
    FILE *pt_fp = fopen(plaintext_file, "r");
    if (!pt_fp) {
        perror("fopen plaintext");
        return 1;
    }
    fseek(pt_fp, 0, SEEK_END);
    long pt_len = ftell(pt_fp);
    fseek(pt_fp, 0, SEEK_SET);
    char *plaintext = malloc(pt_len + 1);
    fread(plaintext, 1, pt_len, pt_fp);
    fclose(pt_fp);
    plaintext[pt_len - 1] = '\0'; 
    FILE *key_fp = fopen(key_file, "r");
    if (!key_fp) {
        perror("fopen key");
        return 1;
    }
    fseek(key_fp, 0, SEEK_END);
    long key_len = ftell(key_fp);
    fseek(key_fp, 0, SEEK_SET);
    char *key = malloc(key_len + 1);
    fread(key, 1, key_len, key_fp);
    fclose(key_fp);
    key[key_len - 1] = '\0'; 
    if (key_len < pt_len) {
        fprintf(stderr, "Error: key '%s' is too short\n", key_file);
        return 1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 2;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error: could not contact enc_server on port %d\n", port);
        return 2;
    }

    close(sock_fd);
    free(plaintext);
    free(key);
    return 0;
}
