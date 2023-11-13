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
#include <time.h>

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

char* getUserRights(mode_t mod)
{
    char* rights = malloc(sizeof(char) * 4);
    if(rights == NULL)
    {
        perror("Error at allocating memory\n");
        exit(errno);
    }

    sprintf(rights, "---"); //no permissions

    if(mod & S_IRUSR)
    {
        rights[0] = 'R';
    }

    if(mod & S_IWUSR)
    {
        rights[1] = 'W';
    }

    if(mod & S_IXUSR)
    {
        rights[2] = 'X';
    }

    return rights;
}

char* getGroupRights(mode_t mod)
{
    char* rights = malloc(sizeof(char) * 4);
    if(rights == NULL)
    {
        perror("Error at allocating memory\n");
        exit(errno);
    }

    sprintf(rights, "---"); //no permissions

    if(mod & S_IRGRP)
    {
        rights[0] = 'R';
    }

    if(mod & S_IWGRP)
    {
        rights[1] = 'W';
    }

    if(mod & S_IXGRP)
    {
        rights[2] = 'X';
    }

    return rights;
}

char* getOtherRights(mode_t mod)
{
    char* rights = malloc(sizeof(char) * 4);
    if(rights == NULL)
    {
        perror("Error at allocating memory\n");
        exit(errno);
    }

    sprintf(rights, "---"); //no permissions

    if(mod & S_IROTH)
    {
        rights[0] = 'R';
    }

    if(mod & S_IWOTH)
    {
        rights[1] = 'W';
    }

    if(mod & S_IXOTH)
    {
        rights[2] = 'X';
    }

    return rights;
}

char* getLastModification(time_t time)
{
    struct tm* timeptr = gmtime(&time);
    return asctime(timeptr);
}

void processDir(char* path, struct stat* inf, int fout)
{
    printf("nume director: %s\n", path);
    printf("user id: %u\n", inf->st_uid);
    printf("dimensiune (bytes): %ld\n", inf->st_size);
    printf("drepturi user: %s\n", getUserRights(inf->st_mode));
    printf("drepturi grup: %s\n", getGroupRights(inf->st_mode));
    printf("drepturi altii: %s\n", getOtherRights(inf->st_mode));
    printf("\n");
    
}

void processLink(char* path, struct stat* inf, int fout)
{
    printf("nume legatura: %s\n", path);
    printf("dimensiune (bytes): %ld\n", inf->st_size);
    printf("dimensiune fis target: \n");
    printf("drepturi user: %s\n", getUserRights(inf->st_mode));
    printf("drepturi grup: %s\n", getGroupRights(inf->st_mode));
    printf("drepturi altii: %s\n", getOtherRights(inf->st_mode));
    printf("\n");
}

void processFile(char* path, struct stat* inf, int fout)
{
    printf("nume fisier: %s\n", path);
    printf("dimensiune (bytes): %ld\n", inf->st_size);
    printf("user id: %u\n", inf->st_uid);
    printf("ult modif: %s", getLastModification(inf->st_atime));
    printf("contor legaturi: %lu\n", inf->st_nlink);
    printf("drepturi user: %s\n", getUserRights(inf->st_mode));
    printf("drepturi grup: %s\n", getGroupRights(inf->st_mode));
    printf("drepturi altii: %s\n", getOtherRights(inf->st_mode));
    printf("\n");
}

void processEntry(char* path, int type, struct stat* inf, int fout)
{

    switch (type)
    {
    case 0: //directory
        processDir(path, inf, fout);
        break;
    case 1: //symlink
        processLink(path, inf, fout);
        break;
    case 2: //regular file
        processFile(path, inf, fout);
        break;
    default:
        printf("File %s is of unknown type\n", path);
        break;
    }

}

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
        
        //processes entry and writes in output file according to entry type
        processEntry(relpath, type, &info, fileout);

    }

    //closes input directory
    closeDir(dirIn);

    //closes output file
    closeFile(fileout);

    return 0;
}