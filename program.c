#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <dirent.h>

#define FILE_OUT "statistica.txt"
#define BUFF_SIZE 4

DIR* openDir(char* path)
{
    DIR* dIn = opendir(path);
    if (dIn == NULL)
    {
        char* errstr = "\0"; 
        sprintf(errstr, "Could not open directory named %s\n", path); 
        perror(errstr); //prints an error message stating what file cannot be opened
        exit(errno); //stops the program
    }

    return dIn;
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

int main(int argc, char** argv)
{
    if(argc != 2 ) // only one parameter allowed
    {
        perror("Usage ./program <input_dir>");
        exit(errno);
    }

    DIR* dirIn = openDir(argv[1]);

    int fileout = createFile(FILE_OUT);

    return 0;
}