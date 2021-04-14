#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#define ERR(x) fprintf(stderr, "%s", x); \
    exit(EXIT_FAILURE); // Error log macro definition

typedef unsigned long UL;

struct file_list
{
    char ** flist;
    int size;
};

struct file_list readTheWholeDir(const char * path)
{
    struct dirent * dir_data = NULL;

    DIR * dir;
    dir = opendir(path);

    char ** fList;

    int size = 0;

    while ((dir_data = readdir(dir)) != NULL)
    {
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

            strcpy(fList[0], dir_data->d_name);

            size ++;
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

            size ++;
        }
    }

    closedir(dir);

    struct file_list fl;

    fl.flist = fList;
    fl.size = size;

    return fl;
}

void printGroupName(gid_t grpNum) // this function prints the group name refered to grpNum
{
    struct group * grp = getgrgid(grpNum);

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
    struct passwd * pw = getpwuid(uid);

    if (pw) //error check
    {
        printf("%s", pw->pw_name);
    }

    else
    {
        ERR("Error caught when printing the user name!");
    }
}

void printIndexNum(const char * file) // this function prints the inode number of the given file
{
    struct stat * buf = (struct stat *)malloc(sizeof (struct stat)); // allocate memory for the stat buffer
    memset(buf, 0, sizeof (struct stat)); // clear data fields inside a buffer

    if (stat(file, buf) == -1) // error check
    {
        ERR("Error caught when calling the stat function!");
    }

    printf("%d %s", (int)buf->st_ino, file); // print inode number

    free(buf); // deallocate memory
}

void printLongListing(const char * file) // this function prints the long listed format
{
    struct stat * buf = (struct stat *)malloc(sizeof (struct stat));
    memset(buf, 0, sizeof (struct stat));

    if (stat(file, buf) == -1) // error check
    {
        ERR("Error caught when calling the stat function!");
    }

    int flags[9] = { S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH }; // flags which determine whether the user or group or others have a read, write, and execution permission
    char perm[3] = { 'r', 'w', 'x' }; //permission array

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
    printf("1 "); // print others
    printUserName(buf->st_uid); // print owner id
    printf(" "); // print blank space
    printGroupName(buf->st_gid); // print group id
    printf(" %d %s", (int)buf->st_size, file); // print size
    printf(" %s", ctime(&buf->st_mtime)); // print last modification date
    printf(" %s", file); // print file name

    free(buf);
}

void traverseDir(const char * path) // this function goes through the given directory
{
    struct dirent * dir_data = NULL;

    DIR * dir;
    dir = opendir(path);

    struct stat * buf = (struct stat *)malloc(sizeof (struct stat));

    memset(buf, 0, sizeof (struct stat));

    while((dir_data = readdir(dir)) != NULL) // read each file and directory inside a given directory
    {
        char dest[strlen(path) + strlen(dir_data->d_name) + 2];
        sprintf(dest, "%s/%s", path, dir_data->d_name); // string concatenation

        if (strcmp(dir_data->d_name, "..") != 0 && strcmp(dir_data->d_name, ".") != 0) // if directory doesnt match the parent directory go through it
        {
            if (stat(dest, buf) == -1) // error check
            {
                fprintf(stderr, "stat: %s: Permission denied\n", dest);
            }

            if (buf->st_mode & S_IFDIR) // if device is a directory
            {
                printf("%s\n", dest);
                traverseDir(dest); // go recursively
            }

            memset(buf, 0, sizeof (struct stat)); // clear all the data field inside a buffer
        }
    }

    free(buf); // deallocate memory
    closedir(dir); // close the directory
}

int main(int argc, char * argv[])
{
    if (argc == 1) // if no arguments are given just display the current directory content
    {
        struct file_list fl = readTheWholeDir("."); // store the current directory content inside the file_list structure

        for (int i = 0; i < fl.size; i++)
        {
            printf("%s\n", fl.flist[i]); // print directory content
        }

        for (int i = 0; i < fl.size; i++)
        {
            free(fl.flist[i]); // deallocate memory
        }

        free(fl.flist); // deallocate memory
    }

    else if (argc >= 2) // invoke this code fragment if options are given
    {
        for (UL i = 0; i < strlen(argv[1]); i++)
        {            
            if (argv[1][i] == 'i')
            {
                if (argc == 2)
                {
                    struct file_list fl = readTheWholeDir("."); // print directory content using file indexing format

                    for (int i = 0; i < fl.size; i++)
                    {
                        printIndexNum(fl.flist[i]);
                        printf("\n");
                    }

                    for (int i = 0; i < fl.size; i++)
                    {
                        free(fl.flist[i]); // deallocate memory
                    }

                    free(fl.flist); // deallocate memory

                    continue;
                }

                for (int i = 0; i < (argc - 2); i++) // print directory content using file indexing format if file list is given
                {
                    printIndexNum(argv[2 + i]);
                    printf("\n");
                }
            }

            else if (argv[1][i] == 'l')
            {
                if (argc == 2)
                {
                    struct file_list fl = readTheWholeDir("."); // print directory content using long termed format

                    for (int i = 0; i < fl.size; i++)
                    {
                        printLongListing(fl.flist[i]);
                        printf("\n");
                    }

                    for (int i = 0; i < fl.size; i++) // deallocate memory
                    {
                        free(fl.flist[i]);
                    }

                    free(fl.flist); // deallocate memory

                    continue;
                }

                for (int i = 0; i < (argc - 2); i++) // print directory content using long termed format if the file list is given
                {
                    printLongListing(argv[2 + i]);
                    printf("\n");
                }
            }

            else if (argv[1][i] == 'R')
            {
                if (argc == 2)
                {
                    traverseDir("."); // display sub directories

                    continue;
                }

                for (int i = 0; i < (argc - 2); i++)
                {
                    traverseDir(argv[2 + i]); // display sub directories if dirctory list is given
                }
            }

            else if (argv[1][i] != '-')
            {
                char errMsg[20];
                sprintf(errMsg, "Unknow option -%c", argv[1][i]);

                ERR(errMsg);
            }
        }
    }

    return 0;
}
