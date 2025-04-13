#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <stdbool.h>

#define ERR(x)                \
    do {                      \
        fprintf(stderr, "%s", x); \
        exit(EXIT_FAILURE);   \
    } while (0)

#define I_OPT 1
#define L_OPT 2
#define R_OPT 4
#define MAX_DIRECTORY_CHARS 4096 // Increased for safety

typedef unsigned long UL;

struct file_list {
    char *directory;
    char **flist;
    int size;
};

struct options {
    int opt;
};

struct file_list readTheWholeDir(const char *path) {
    struct file_list fl = {0};
    struct dirent *dir_data = NULL;
    DIR *dir = opendir(path);
    char **fList = NULL;
    int size = 0;

    if (!dir) {
        ERR("Cannot open directory\n");
    }

    while ((dir_data = readdir(dir)) != NULL) {
        if (strcmp(dir_data->d_name, "..") == 0 || strcmp(dir_data->d_name, ".") == 0) {
            continue;
        }

        char **temp = realloc(fList, sizeof(char *) * (size + 1));
        if (!temp) {
            closedir(dir);
            for (int i = 0; i < size; i++) free(fList[i]);
            free(fList);
            ERR("Memory allocation failed\n");
        }
        fList = temp;

        fList[size] = malloc(strlen(dir_data->d_name) + 1);
        if (!fList[size]) {
            closedir(dir);
            for (int i = 0; i < size; i++) free(fList[i]);
            free(fList);
            ERR("Memory allocation failed\n");
        }
        strcpy(fList[size], dir_data->d_name);
        size++;
    }
    closedir(dir);

    fl.flist = fList;
    fl.size = size;

    char directory[MAX_DIRECTORY_CHARS];
    snprintf(directory, sizeof(directory), "%s", path);
    if (path[strlen(path) - 1] != '/') {
        strncat(directory, "/", sizeof(directory) - strlen(directory) - 1);
    }

    fl.directory = malloc(strlen(directory) + 1);
    if (!fl.directory) {
        for (int i = 0; i < size; i++) free(fList[i]);
        free(fList);
        ERR("Memory allocation failed\n");
    }
    strcpy(fl.directory, directory);

    return fl;
}

void printGroupName(gid_t grpNum) {
    struct group *grp = getgrgid(grpNum);
    if (grp) {
        printf("%s", grp->gr_name);
    } else {
        printf("%d", (int)grpNum);
    }
}

void printUserName(uid_t uid) {
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        printf("%s", pw->pw_name);
    } else {
        printf("%d", (int)uid);
    }
}

void printIndexNum(const char *file) {
    struct stat buf;
    if (stat(file, &buf) == -1) {
        fprintf(stderr, "stat: %s: Cannot access file\n", file);
        return;
    }
    printf("%ld ", (long)buf.st_ino);
}

void printLongListing(const char *file) {
    struct stat buf;
    if (stat(file, &buf) == -1) {
        fprintf(stderr, "stat: %s: Cannot access file\n", file);
        return;
    }

    int flags[9] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    char perm[3] = {'r', 'w', 'x'};

    for (int i = 0; i < 9; i++) {
        printf("%c", (buf.st_mode & flags[i]) ? perm[i % 3] : '-');
    }
    printf(" %ld ", (long)buf.st_nlink);
    printUserName(buf.st_uid);
    printf(" ");
    printGroupName(buf.st_gid);
    printf(" %ld ", (long)buf.st_size);

    char *date = ctime(&buf.st_mtime);
    if (date) {
        date[strlen(date) - 1] = '\0';
        printf("%s ", date);
    }
}

void custom_print(struct file_list *fl, struct options *opt, bool is_first_layer) {
    struct stat buf;

    for (int i = 0; i < fl->size; i++) {
        char full_path[MAX_DIRECTORY_CHARS];
        snprintf(full_path, sizeof(full_path), "%s%s", fl->directory, fl->flist[i]);

        if (stat(full_path, &buf) == -1) {
            fprintf(stderr, "stat: %s: Permission denied or file does not exist\n", full_path);
            continue;
        }

        if (!is_first_layer || !(buf.st_mode & S_IFDIR)) {
            if (opt->opt & I_OPT) {
                printIndexNum(full_path);
            }
            if (opt->opt & L_OPT) {
                printLongListing(full_path);
            }
            printf("%s", fl->flist[i]);
            printf("%s", (opt->opt & R_OPT) || (opt->opt & L_OPT) ? "\n" : " ");
        }

        if ((opt->opt & R_OPT) && (buf.st_mode & S_IFDIR)) {
            struct file_list subfl = readTheWholeDir(full_path);
            printf("\n%s:\n", full_path);
            custom_print(&subfl, opt, false);
            for (int j = 0; j < subfl.size; j++) {
                free(subfl.flist[j]);
            }
            free(subfl.flist);
            free(subfl.directory);
        }
    }
}

void sort_flist(struct file_list *fl) {
    for (int i = 0; i < fl->size; i++) {
        for (int j = i + 1; j < fl->size; j++) {
            if (strcmp(fl->flist[i], fl->flist[j]) > 0) {
                char *temp = fl->flist[i];
                fl->flist[i] = fl->flist[j];
                fl->flist[j] = temp;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    struct options opt = {0};
    struct file_list fl = {0};
    bool is_first_layer = false;

    if (argc == 1) {
        fl = readTheWholeDir(".");
        sort_flist(&fl);
        for (int i = 0; i < fl.size; i++) {
            printf("%s ", fl.flist[i]);
        }
        printf("\n");
        for (int i = 0; i < fl.size; i++) {
            free(fl.flist[i]);
        }
        free(fl.flist);
        free(fl.directory);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (size_t j = 1; j < strlen(argv[i]); j++) {
                if (argv[i][j] == 'i') opt.opt |= I_OPT;
                else if (argv[i][j] == 'l') opt.opt |= L_OPT;
                else if (argv[i][j] == 'R') opt.opt |= R_OPT;
                else {
                    fprintf(stderr, "Unknown option: %c\n", argv[i][j]);
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            char **temp = realloc(fl.flist, sizeof(char *) * (fl.size + 1));
            if (!temp) {
                for (int j = 0; j < fl.size; j++) free(fl.flist[j]);
                free(fl.flist);
                free(fl.directory);
                ERR("Memory allocation failed\n");
            }
            fl.flist = temp;

            fl.flist[fl.size] = malloc(strlen(argv[i]) + 1);
            if (!fl.flist[fl.size]) {
                for (int j = 0; j < fl.size; j++) free(fl.flist[j]);
                free(fl.flist);
                free(fl.directory);
                ERR("Memory allocation failed\n");
            }
            strcpy(fl.flist[fl.size], argv[i]);
            fl.size++;
        }
    }

    is_first_layer = fl.size > 0;

    if (fl.size == 0) {
        fl = readTheWholeDir(".");
    } else {
        fl.directory = malloc(2);
        if (!fl.directory) {
            for (int i = 0; i < fl.size; i++) free(fl.flist[i]);
            free(fl.flist);
            ERR("Memory allocation failed\n");
        }
        strcpy(fl.directory, "");
    }

    sort_flist(&fl);
    custom_print(&fl, &opt, is_first_layer);

    for (int i = 0; i < fl.size; i++) {
        free(fl.flist[i]);
    }
    free(fl.flist);
    free(fl.directory);

    return 0;
}