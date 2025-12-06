#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: You must provide the key length.\n");
        fprintf(stderr, "Usage: %s keylength\n", argv[0]);
        exit(1);
    }

    int key_length = atoi(argv[1]);
    srand(time(NULL));
    char allowed_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    for (int i = 0; i < key_length; i++) {
        int random_index = rand() % 27;
        fprintf(stdout, "%c", allowed_chars[random_index]);
    }
    fprintf(stdout, "\n");

    return 0;
}
//u