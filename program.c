#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define FILE_OUT "statistica.txt"
#define BUFF_SIZE 4

int main(int argc, char** argv)
{
    if(argc != 2 ) // only one parameter allowed
    {
        perror("Usage ./program <input_file>");
        exit(errno);
    }

    int fileinp = open(argv[1], O_RDONLY); //opens file in read only mode

    if(fileinp == -1) //if file cannot be opened
    {
        char* errstr = "\0"; 
        sprintf(errstr, "Could not open file named %s\n", argv[1]); 
        perror(errstr); //prints an error message stating what file cannot be opened
        exit(errno); //stops the program
    }

    int fileout = creat(FILE_OUT, S_IRWXU); //creates output file

    if(fileout == -1) //if output file cannot be created
    {
        perror("Could not create \'statistica.txt\' output file\n"); //prints an error message
        exit(errno); //stops the program
    }



    return 0;
}