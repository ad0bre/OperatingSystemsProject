#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#define FILE_OUT "statistica.txt"
#define BUFF_SIZE 4
#define PATH_SIZE 256
DIR* openDir(char* path)
{
    DIR* dIn = opendir(path);
    if (dIn == NULL)
    {
        char* errstr = "\0"; 
        sprintf(errstr, "Could not open directory %s\n", path); 
        perror(errstr); //prints an error message stating what file cannot be opened
        exit(errno); //stops the program
    }

    return dIn;
}

int openFileReadOnly(char* path)
{
    int fd = 0;
    if((fd = open(path, O_RDONLY)) < 0)
    {
        char* errstr = "\0"; 
        sprintf(errstr, "Could not open file %s\n", path); 
        perror(errstr); //prints an error message stating what file cannot be opened
        exit(errno); //stops the program
    }

    return fd;
}

int createFile(char* path)
{
    int out = creat(path, S_IRWXU); //creates output file

    if(out == -1) //if output file cannot be created
    {
        perror("Could not create \'statistica.txt\' output file\n"); //prints an error message
        exit(errno); //stops the program
    }

    return out;
}

void closeDir(DIR* dir)
{
    if(closedir(dir) != 0) //closes directory
    {
        perror("Could not close directory\n");
        exit(errno); //stops program if directory cannot be closed
    }
}

void closeFile(int fd)
{
    if(close(fd) != 0)
    {
        perror("Could not close file\n");
        exit(errno);
    }
}

char* generateRelativePath(char* parentPath, char* fileName)
{
    char* path = malloc(sizeof(char) * PATH_SIZE);
    if(path == NULL)
    {
        perror("Error at allocating memory\n");
        exit(errno);
    }
    sprintf(path, "%s/%s", parentPath, fileName);
    return path;
}

void getStat(char* path, struct stat* infop)
{
    if(stat(path, infop) != 0)
    {
        perror("Something went wrong getting the stats for a file...\n");
        exit(errno);
    }
}

int getEntryType(mode_t mod)
{
    if(S_ISDIR(mod))
    {
        return 0;
    }

    if(S_ISLNK(mod))
    {
        return 1;
    }

    if(S_ISREG(mod))
    {
        return 2;
    }

    return -1;
}

// void processFile(char* path, mode_t type)
// {
//     int file = openFileReadOnly(path);

//     closeFile(file);

// }

int main(int argc, char** argv)
{
    if(argc != 2 ) // only one parameter allowed
    {
        perror("Usage ./program <input_dir>");
        exit(errno);
    }

    //open directory from path received in command line arguments aray
    DIR* dirIn = openDir(argv[1]);

    //creates output file with the name 'statistics.txt'
    int fileout = createFile(FILE_OUT);

    //reads directory entries until the end
    struct dirent* entry;
    while((entry = readdir(dirIn)) != NULL)
    {
        //generates relative path of current entry
        char* relpath = generateRelativePath(argv[1], entry->d_name); 

        //gets info on current entry
        struct stat info;
        getStat(relpath, &info);

        //gets the type of the current entry: 0 - dir, 1 - symlink, 2 - regular file, -1 - unknown
        int type = getEntryType(info.st_mode);
        

    }

    //closes input directory
    closeDir(dirIn);

    //closes output file
    closeFile(fileout);

    return 0;
}