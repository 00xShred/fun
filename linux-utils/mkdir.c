#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("usage: mkdir [path]...\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        const char *path = argv[i];
        if (mkdir(path, 0777)) { perror(path); }
        return 1;
    }
    return 0;
}
