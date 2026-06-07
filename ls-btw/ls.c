#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

void mode_string(mode_t mode, char *str) {
    switch (mode & S_IFMT) {
        case S_IFDIR: str[0] = 'd'; break;
        case S_IFLNK: str[0] = 'l'; break;
        case S_IFCHR: str[0] = 'c'; break;
        case S_IFBLK: str[0] = 'b'; break;
        case S_IFIFO: str[0] = 'p'; break;
        case S_IFSOCK: str[0] = 's'; break;
        default: str[0] = '-'; break;
    }

    str[1]  = (mode & S_IRUSR) ? 'r' : '-';
    str[2]  = (mode & S_IWUSR) ? 'w' : '-';
    str[3]  = (mode & S_IXUSR) ? 'x' : '-';
    str[4]  = (mode & S_IRGRP) ? 'r' : '-';
    str[5]  = (mode & S_IWGRP) ? 'w' : '-';
    str[6]  = (mode & S_IXGRP) ? 'x' : '-';
    str[7]  = (mode & S_IROTH) ? 'r' : '-';
    str[8]  = (mode & S_IWOTH) ? 'w' : '-';
    str[9]  = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

void print_long(const char *dir, const char *name) {
    char fullpath[4096];
    size_t dir_len = strlen(dir);
    if (dir_len > 0 && dir[dir_len - 1] == '/') {
        snprintf(fullpath, sizeof(fullpath), "%s%s", dir, name);
    } else {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);
    }

    struct stat st;
    if (lstat(fullpath, &st) < 0) {
        perror(name);
        return;
    }
    char modes[11];
    mode_string(st.st_mode, modes);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr  = getgrgid(st.st_gid);
    const char *user  = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    char timebuff[64];

    struct tm *tm = localtime(&st.st_mtim.tv_sec);
    strftime(timebuff, sizeof(timebuff), "%b %e %H:%M", tm);

    printf("%s %lu %s %s %ld %s %s\n",
           modes,
           (unsigned long)st.st_nlink,
           user,
           group,
           (long)st.st_size,
           timebuff,
           name);
}

int show_all    = 0;
int long_format = 0;

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "al")) != -1) {
        switch (opt) {
            case 'a': show_all = 1; break;
            case 'l': long_format = 1; break;
            default: fprintf(stderr, "usage: %s [-al] [path]\n", argv[0]); return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') continue;
        if (long_format) {
            print_long(path, entry->d_name);
        } else {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);

    return 0;
}
