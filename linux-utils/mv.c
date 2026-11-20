
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <wchar.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("usage: mv [path] [target]...\n");
        printf("     : mv [path] [dir]...\n");
        return 0;
    }

    const char *target = argv[argc - 1];
    argc--;

    struct stat st;
    if (stat(target, &st)) {
        if (errno == ENOENT && argc == 2) {
            const char *path = argv[1];
            if (rename(path, target)) {
                perror(path);
                return 1;
            }
            return 0;
        }
        perror(target);
        return 1;
    }

    const char *dir = target;
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "%s is not a dir or does not exist\n", dir);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        const char *path = argv[i];
        printf("path: %s\n", path);
        const char *name = strrchr(path, '/');
        if (!name) {
            name = path;
        } else {
            name++;
        }

        size_t path_len = strlen(dir) + strlen(name) + 2;
        char new_path[path_len];
        snprintf(new_path, path_len, "%s/%s", dir, name);

        if (rename(path, new_path)) {
            perror(path);
            return 1;
        }
    }

    return 0;
}
