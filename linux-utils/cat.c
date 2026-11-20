#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("usage: cat [path]...\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        const char *path = argv[i];
        FILE *fp         = fopen(path, "r");
        if (!fp) {
            perror(path);
            return 1;
        }

        char c;
        while ((c = fgetc(fp)) != EOF) { putchar(c); }

        fclose(fp);
    }
    return 0;
}
