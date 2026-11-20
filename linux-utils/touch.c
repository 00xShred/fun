#include <stdio.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    static struct option longopts[] = {
        {"version", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0},
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "v", longopts, NULL)) != -1) {
        if (opt == 'v') {
            printf("touch 0.1\n");
            return 0;
        }
        return 1;
    }

    if (optind == argc) {
        printf("usage: touch [path]...\n");
        return 0;
    }

    for (int i = optind; i < argc; i++) {
        const char *path = argv[i];
        FILE *fp         = fopen(path, "w");
        if (!fp) {
            perror(path);
            return 1;
        }
        fclose(fp);
    }
    return 0;
}
