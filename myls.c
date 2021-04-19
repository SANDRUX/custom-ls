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
    fprintf(stderr, "%s", x); \
    exit(EXIT_FAILURE); // Error log macro definition

#define I_OPT 1
#define L_OPT 2
#define R_OPT 4

#define MAX_DIRECTORY_CHARS 1000

typedef unsigned long UL;

struct file_list
{
    char *directory;
    char **flist;
    int size;
};

struct options
{
    int opt;
};

struct file_list readTheWholeDir(const char *path)
{
    struct dirent *dir_data = NULL;

    DIR *dir;
    dir = opendir(path);

    char **fList = NULL;

    int size = 0;

    while ((dir_data = readdir(dir)) != NULL)
    {
        if (strcmp(dir_data->d_name, "..") == 0 || strcmp(dir_data->d_name, ".") == 0)
            continue;
        if (size == 0)
        {
            if ((fList = (char **)malloc(sizeof(char *))) == NULL)
            {
                ERR("\nCould not allocate memory on the heap");
            }

            if ((fList[0] = (char *)malloc(strlen(dir_data->d_name) + 1)) == NULL)
            {
                ERR("\nCould not allocate memory on the heap");
            }

            if (strcmp(dir_data->d_name, "..") != 0 && strcmp(dir_data->d_name, ".") != 0) // if directory doesnt match the parent directory go through it
            {
                strcpy(fList[0], dir_data->d_name);
            }

            // sprintf(fList[0], "%s/%s", path, dir_data->d_name);

            size++;
        }
        else
        {
            if ((fList = (char **)realloc(fList, (sizeof(char *) * (size + 1)))) == NULL)
            {
                ERR("\nCould not allocate memory on the heap");
            }

            if ((fList[size] = (char *)malloc(strlen(dir_data->d_name) + 1)) == NULL)
            {
                ERR("\nCould not allocate memory on the heap");
            }

            strcpy(fList[size], dir_data->d_name);

            size++;
        }
    }

    closedir(dir);

    struct file_list fl;

    fl.flist = fList;
    fl.size = size;

    char directory[MAX_DIRECTORY_CHARS];
    strcpy(directory, path);

    if (path[strlen(path) - 1] != '/' && path[strlen(path) - 1] != '\\')
    {
        strcat(directory, "/");
    }

    fl.directory = (char *)malloc(strlen(directory) + 1);
    strcpy(fl.directory, directory);

    return fl;
}

void printGroupName(gid_t grpNum) // this function prints the group name refered to grpNum
{
    struct group *grp = getgrgid(grpNum);

    if (grp) // error check
    {
        printf("%s", grp->gr_name);
    }

    else
    {
        ERR("Error caught when printing the group name!");
    }
}

void printUserName(uid_t uid) // this function prints the user name refered to uid
{
    struct passwd *pw = getpwuid(uid);

    if (pw) //error check
    {
        printf("%s", pw->pw_name);
    }

    else
    {
        ERR("Error caught when printing the user name!");
    }
}

void printIndexNum(const char *file) // this function prints the inode number of the given file
{
    struct stat *buf = (struct stat *)malloc(sizeof(struct stat)); // allocate memory for the stat buffer
    memset(buf, 0, sizeof(struct stat));                           // clear data fields inside a buffer

    if (stat(file, buf) == -1) // error check
    {
        ERR("Error caught when calling the stat function!");
    }

    printf("%d ", (int)buf->st_ino); // print inode number

    free(buf); // deallocate memory
}

void printLongListing(const char *file) // this function prints the long listed format
{
    struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
    memset(buf, 0, sizeof(struct stat));

    if (stat(file, buf) == -1) // error check
    {
        char errorText[MAX_DIRECTORY_CHARS + 100];
        snprintf(errorText, sizeof(errorText), "Error caught when calling the stat function on file %s!\n\n", file);
        ERR(errorText);
    }

    int flags[9] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH}; // flags which determine whether the user or group or others have a read, write, and execution permission
    char perm[3] = {'r', 'w', 'x'};                                                                   //permission array

    for (int i = 0; i < 9; i++) // this loop here prints out permissions for the given users
    {
        if (buf->st_mode & flags[i])
        {
            printf("%c", perm[i % 3]); // print wether the desired user has read, write, execute permission
        }

        else
        {
            printf("-"); // print '-' if user has no such permission
        }
    }

    printf(" ");
    printf("%d ", (int)buf->st_nlink);         // print others
    printUserName(buf->st_uid);                // print owner id
    printf(" ");                               // print blank space
    printGroupName(buf->st_gid);               // print group id
    printf(" %d ", (int)buf->st_size); // print size

    char date[strlen(ctime(&buf->st_mtime)) + 1];
    strcpy(date, ctime(&buf->st_mtime));
    date[strlen(date) - 1] = '\0';

    printf(" %s ", date); // print last modification date

    free(buf);
}

//void traverseDir(const char * path) // this function goes through the given directory
//{
//    struct dirent * dir_data = NULL;

//    DIR * dir;
//    dir = opendir(path);

//    struct stat * buf = (struct stat *)malloc(sizeof (struct stat));

//    memset(buf, 0, sizeof (struct stat));

//    while((dir_data = readdir(dir)) != NULL) // read each file and directory inside a given directory
//    {
//        char dest[strlen(path) + strlen(dir_data->d_name) + 2];
//        sprintf(dest, "%s/%s", path, dir_data->d_name); // string concatenation

//        if (strcmp(dir_data->d_name, "..") != 0 && strcmp(dir_data->d_name, ".") != 0) // if directory doesnt match the parent directory go through it
//        {
//            if (stat(dest, buf) == -1) // error check
//            {
//                fprintf(stderr, "stat: %s: Permission denied\n", dest);
//            }

//            if (buf->st_mode & S_IFDIR) // if device is a directory
//            {
//                printf("%s\n", dest);
//                traverseDir(dest); // go recursively
//            }

//            memset(buf, 0, sizeof (struct stat)); // clear all the data field inside a buffer
//        }
//    }

//    free(buf); // deallocate memory
//    closedir(dir); // close the directory
//}

void custom_print(struct file_list *fl, struct options *opt, bool is_first_layer)
{
    struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
    memset(buf, 0, sizeof(struct stat));

    if (buf == NULL)
    {
        ERR("\n Could not allocate memory on the heap");
    }

    for (int i = 0; i < fl->size; i++)
    {
        char full_path[MAX_DIRECTORY_CHARS];

        strcpy(full_path, fl->directory);
        strcat(full_path, fl->flist[i]);

        //printf("Full path is now %s\n\n", full_path);

        if (stat(full_path, buf) == -1)
        {
            fprintf(stderr, "stat: %s: Permission denied or file does not exists!\n", full_path);
        }

        if (!is_first_layer || !(buf->st_mode & S_IFDIR))
        {
            if (opt->opt & I_OPT)
            {
                printIndexNum(full_path);
            }

            if (opt->opt & L_OPT)
            {
                printLongListing(full_path);
            }

            printf("%s", fl->flist[i]);

            if ((opt->opt & R_OPT) || (opt->opt & L_OPT))
            {
                printf("\n");
            }
            else
            {
                printf(" ");
            }
        }

        if ((opt->opt & R_OPT) == 0 && !is_first_layer)
        {
            continue;
        }

        if (buf->st_mode & S_IFDIR)
        {
            struct file_list subfl = readTheWholeDir(full_path);

            custom_print(&subfl, opt, false);

            for (int i = 0; i < subfl.size; i++)
            {
                free(subfl.flist[i]);
            }

            free(subfl.directory);
        }
    }

    free(buf);
}

int main(int argc, char *argv[])
{
    if (argc == 1) // if no arguments are given just display the current directory content
    {
        struct file_list fl = readTheWholeDir("."); // store the current directory content inside the file_list structure

        char temp[30];

        for (int i = 0; i < fl.size; i++)
        {
            for (int j = (i + 1); j < fl.size; j++)
            {
                if (strcmp(fl.flist[i], fl.flist[j]) > 0)
                {
                    strcpy(temp, fl.flist[i]);
                    strcpy(fl.flist[i], fl.flist[j]);
                    strcpy(fl.flist[j], temp);

                    // printf("%s\n", fl.flist[i]);
                    // printf("%s", fl.flist[j]);
                }
            }
        }

        // qsort(fl.flist, fl.size, sizeof(const char *), stringComparer);

        for (int i = 0; i < fl.size; i++)
        {
            printf("%s ", fl.flist[i]); // print directory content
        }

        for (int i = 0; i < fl.size; i++)
        {
            free(fl.flist[i]); // deallocate memory
        }

        free(fl.flist); // deallocate memory
        free(fl.directory);

        return 0;
    }

    struct options *opt = (struct options *)malloc(sizeof(struct options));
    struct file_list *fl = (struct file_list *)malloc(sizeof(struct file_list));

    memset(fl, 0, sizeof(struct file_list));
    memset(opt, 0, sizeof(struct file_list));

    if (argc > 1) // invoke this code fragment if options are given
    {
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                for (UL j = 1; j < strlen(argv[i]); j++)
                {
                    if (argv[i][j] == 'i')
                    {
                        opt->opt |= I_OPT;
                    }
                    else if (argv[i][j] == 'l')
                    {
                        opt->opt |= L_OPT;
                    }
                    else if (argv[i][j] == 'R')
                    {
                        opt->opt |= R_OPT;
                    }
                    else
                    {
                        fprintf(stderr, "Unknow option %c", argv[i][j]);
                        exit(EXIT_FAILURE);
                    }
                }
            }

            else
            {
                if (fl->size == 0)
                {
                    if ((fl->flist = (char **)malloc(sizeof(char *))) == NULL)
                    {
                        ERR("\nCould not allocate memory on the heap");
                    }

                    if ((fl->flist[0] = (char *)malloc(strlen(argv[i]) + 1)) == NULL)
                    {
                        ERR("\nCould not allocate memory on the heap");
                    }

                    strcpy(fl->flist[0], argv[i]);

                    fl->size++;
                }

                else
                {
                    if ((fl->flist = (char **)realloc(fl->flist, sizeof(char *) * (fl->size + 1))) == NULL)
                    {
                        ERR("\nCould not reallocate memory on the heap");
                    }

                    if ((fl->flist[fl->size] = (char *)realloc(fl->flist[fl->size], strlen(argv[i]) + 1)) == NULL)
                    {
                        ERR("\nCould not reallocate memory on the heap");
                    }

                    strcpy(fl->flist[fl->size], argv[i]);

                    fl->size++;
                }
            }
        }
    }

    bool is_first_layer = fl->size > 0;

    if (fl->size == 0)
    {
        *fl = readTheWholeDir("./"); // store the current directory content inside the file_list structure
    }
    else
    {
        fl->directory = (char *)malloc(1);
        strcpy(fl->directory, "");
    }

    // qsort(fl->flist, fl->size, sizeof(const char *), stringComparer);

    char temp[30];

    for (int i = 0; i < fl->size; i++)
    {
        for (int j = (i + 1); j < fl->size; j++)
        {
            if (strcmp(fl->flist[i], fl->flist[j]) > 0)
            {
                strcpy(temp, fl->flist[i]);
                strcpy(fl->flist[i], fl->flist[j]);
                strcpy(fl->flist[j], temp);
            }
        }
    }

    custom_print(fl, opt, is_first_layer);

    for (int i = 0; i < fl->size; i++)
    {
        free(fl->flist[i]);
    }

    free(fl->directory);

    free(fl);

    return 0;
}