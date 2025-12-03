#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: keygen keylength\n");
        return 1;
    }
    int keylength = atoi(argv[1]);
    if (keylength <= 0) {
        fprintf(stderr, "Error: key length must be positive\n");
        return 1;
    }
    srand(time(NULL));
    for (int i = 0; i < keylength; i++) {
        int r = rand() % 27;  
        char c = (r == 26) ? ' ' : 'A' + r;
        putchar(c);
    }
    putchar('\n'); 
    return 0;
}
