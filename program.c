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
#define BUFF_SIZE 256
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

void writeInFile(int file, void* buffer, size_t nbytes)
{
    if(write(file, buffer, nbytes) <= 0)
    {
        perror("Eroare la scrierea in fisier\n");
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
    if(lstat(path, infop) != 0)
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
    char buffer[BUFF_SIZE];
    
    //prints dir name
    sprintf(buffer, "nume director: %s\n", path);
    writeInFile(fout, buffer, strlen(buffer));

    //prints user id
    sprintf(buffer, "identificatorul utilizatorului: %u\n", inf->st_uid);
    writeInFile(fout, buffer, strlen(buffer));
    
    //prints user permissions
    sprintf(buffer, "drepturi de acces user: %s\n", getUserRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    //prints group permissions
    sprintf(buffer, "drepturi de acces grup: %s\n", getGroupRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    //prints others permissions
    sprintf(buffer, "drepturi de acces altii: %s\n", getOtherRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    writeInFile(fout, "\n", 1);
    
}

void processLink(char* path, struct stat* inf, int fout)
{
    char buffer[BUFF_SIZE];

    struct stat target;

    if(stat(path, &target) != 0)
    {
        perror("Something went wrong getting the stats for a file...\n");
        exit(errno);
    }
    
    //prints link name
    sprintf(buffer, "nume legatura: %s\n", path);
    writeInFile(fout, buffer, strlen(buffer));

    //prints link size
    sprintf(buffer, "dimensiune legatura: %ld\n", inf->st_size);
    writeInFile(fout, buffer, strlen(buffer));

    //prints target size
    sprintf(buffer, "dimensiune fisier target: %ld\n", target.st_size);
    writeInFile(fout, buffer, strlen(buffer));
    
    //prints user permissions
    sprintf(buffer, "drepturi de acces user: %s\n", getUserRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    //prints group permissions
    sprintf(buffer, "drepturi de acces grup: %s\n", getGroupRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    //prints others permissions
    sprintf(buffer, "drepturi de acces altii: %s\n", getOtherRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    writeInFile(fout, "\n", 1);
}

void readFromFile(int file, void* buffer, size_t nbytes)
{
    int rd = read(file, buffer, nbytes);
    if(rd < 0)
    {
        perror("Error at reading from file\n");
        exit(errno);
    }
}

uint32_t bytesToNumber(const uint8_t bytes[4]) {
    
    // Assuming little-endian byte order
    return (uint32_t)(
        (bytes[0] << 0) |
        (bytes[1] << 8) |
        (bytes[2] << 16) |
        (bytes[3] << 24)
    );
}

void processFile(char* path, struct stat* inf, int fout)
{
    char sign[] = "AA\0";

    uint8_t metabuffer[4]; 

    uint8_t heightB[4];

    uint8_t widthB[4];

    int file = openFileReadOnly(path);

    //reads signature
    readFromFile(file, sign, 2);

    //if file is BMP
    if((strcmp(sign, "BM")) == 0)
    {
        for(int i = 0; i < 4; i++)
        {
            //reads unimportant data from header
            readFromFile(file, metabuffer, 4);
        }
        
        //reads image width
        readFromFile(file, widthB, 4);

        //reads image length
        readFromFile(file, heightB, 4);
    }

    closeFile(file);

    char buffer[BUFF_SIZE];

    //prints file name
    sprintf(buffer, "numele fisierului: %s\n", path);
    writeInFile(fout, buffer, strlen(buffer));

    //if file is BMP
    if(strcmp(sign, "BM") == 0)
    {
        //transforms bytes arrays into numbers
        u_int32_t width = bytesToNumber(widthB);
        uint32_t height = bytesToNumber(heightB);
        
        //prints file name
        sprintf(buffer, "inaltime: %u\n", height);
        writeInFile(fout, buffer, strlen(buffer));

        //prints file name
        sprintf(buffer, "lungime: %u\n", width);
        writeInFile(fout, buffer, strlen(buffer));
    }

    //prints file size
    sprintf(buffer, "dimensiune fisier: %ld\n", inf->st_size);
    writeInFile(fout, buffer, strlen(buffer));

    //prints user id
    sprintf(buffer, "identificatorul utilizatorului: %u\n", inf->st_uid);
    writeInFile(fout, buffer, strlen(buffer));

    //prints last modification
    sprintf(buffer, "timpuul ultimei modificari: %s", getLastModification(inf->st_atime));
    writeInFile(fout, buffer, strlen(buffer));

    //prints number of links
    sprintf(buffer, "contorul de legaturi: %lu\n", inf->st_nlink);
    writeInFile(fout, buffer, strlen(buffer));
    
    //prints user permissions
    sprintf(buffer, "drepturi de acces user: %s\n", getUserRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    //prints group permissions
    sprintf(buffer, "drepturi de acces grup: %s\n", getGroupRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    //prints others permissions
    sprintf(buffer, "drepturi de acces altii: %s\n", getOtherRights(inf->st_mode));
    writeInFile(fout, buffer, strlen(buffer));

    writeInFile(fout, "\n", 1);
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
    if(argc != 3 ) // only one parameter allowed
    {
        perror("Usage ./program <input_dir> <output_dir>");
        exit(errno);
    }

    //open directory from path received in command line arguments aray
    DIR* dirIn = openDir(argv[1]); 

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