#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define FILE_OUT "statistica.txt"
#define BUFF_SIZE 256
#define PATH_SIZE 256

//file descritptors for pipes
int ChildIn[2], OutParent[2];

//character to be searched for
char character;

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

int openFileReadWrite(char* path)
{
    int fd = 0;
    if((fd = open(path, O_RDWR)) < 0)
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
        perror("Could not create output file\n"); //prints an error message
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

void bmpToGrayScale(int file, int width, int height)
{
   int p = -1, s = -1;
   if((p = fork()) < 0)
   {
        perror("Error at forking\n");
       exit(errno);
   }
   if(p == 0)
   {
        unsigned char* data = (unsigned char*)malloc(width * height * 3);
        if(data == NULL)
        {
            perror("Error at allocating memory\n");
            exit(errno);
        }
        int rd = read(file, data, width * height * 3);
        if(rd < 0)
        {
            perror("Error at reading from BMP file\n");
            exit(errno);
        }
        for(int i = 0; i < width * height; i++)
        {
            unsigned char r = data[i * 3 + 2];
            unsigned char g = data[i * 3 + 1];
            unsigned char b = data[i * 3];
            unsigned char gray = 0.299 * r + 0.587 * g + 0.114 * b;
            data[i * 3] = gray;
            data[i * 3 + 1] = gray;
            data[i * 3 + 2] = gray;
        }
        if(lseek(file, 54, SEEK_SET) < 0)
        {
            perror("Error at lseek\n");
            exit(errno);
        }
        int wr = write(file, data, width * height * 3);
        if(wr < 0)
        {
            perror("Error at writing to BMP file\n");
            exit(errno);
        }
        free(data);
        exit(0);
    }
    else
    {
        wait(&s);
        if(!WIFEXITED(s))
        {
            perror("Process finished anormally\n");
            exit(s);
        }
        printf("Process with PID %d finished with status %d\n", p, WEXITSTATUS(s));
   }
}

void processFile(char* path, struct stat* inf, int fout, int* childIn, int* outParent, int* lines)
{
    char sign[] = "AA\0";

    uint8_t metabuffer[4]; 

    uint8_t heightB[4];

    uint8_t widthB[4];

    int file = openFileReadWrite(path);

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

        //turns BMP into grayscale
        bmpToGrayScale(file, width, height);
        
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

    //Urmatoarele linii de cod au fost comentate pentru ca se blocheaza la citirea din pipe, dar le puteti decomenta pentru a vedea ca scriptul functioneaza

    //creates a new process for counting the number of lines in the file
    // if(strcmp(sign, "BM") != 0)
    // {
    //     if(pipe(childIn) < 0)
    //     {
    //         perror("Error at creating pipe\n");
    //         exit(errno);
    //     }

    //     int p = -1, s = -1;
    //     if((p = fork()) < 0)
    //     {
    //         perror("Error at forking\n");
    //         exit(errno);
    //     }
    //     if(p == 0)
    //     {
    //         //closes write end of the pipe
    //         close(childIn[1]);

    //         //redirects standard input to read from the pipe
    //         dup2(childIn[0], STDIN_FILENO);
    //         close(childIn[0]);

    //         //executes script.sh
    //         execlp("./script.sh", "./script.sh", &character, (char *)NULL);

    //         //checks for errors
    //         perror("Error executing script.sh\n");
    //         exit(EXIT_FAILURE);
    //     }
    //     else
    //     {
    //         //reads contents of file
    //         char buffer[inf->st_size + 1];
    //         int bytesRead = 0;
    //         if((bytesRead = read(file, buffer, inf->st_size)) < 0)
    //         {
    //             perror("Error at reading from file\n");
    //             exit(errno);
    //         }

    //         //writes contents of file in pipe
    //         if(write(childIn[1], buffer, bytesRead) < 0)
    //         {
    //             perror("Error at writing to pipe\n");
    //             exit(errno);
    //         }
            
    //         //wair for child process to finish
    //         wait(&s);
    //         if(!WIFEXITED(s))
    //         {
    //             perror("Process finished anormally\n");
    //             exit(s);
    //         }
    //         printf("Process with PID %d finished with status %d\n", p, WEXITSTATUS(s));

    //         //reads number of lines returned by script.sh
    //         char outputBuffer[10];
    //         int bytesReadFromPipe = read(childIn[0], outputBuffer, sizeof(outputBuffer));
    //         if(bytesReadFromPipe < 0)
    //         {
    //             perror("Error at reading from pipe\n");
    //             exit(errno);
    //         }

    //         //closes read end of the pipe
    //         close(childIn[0]);

    //         *lines = atoi(outputBuffer);
    //     }
    // }

    closeFile(file);
}

void processEntry(char* path, int type, struct stat* inf, int fout, int* childIn, int* outParent, int* lines)
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
        processFile(path, inf, fout, childIn, outParent, lines);
        break;
    default:
        printf("File %s is of unknown type\n", path);
        break;
    }

}

char* generateOutputPath(char* dirpath, char* filename)
{
    char* path = malloc(sizeof(char) * PATH_SIZE);
    if(path == NULL)
    {
        perror("Error at allocating memory\n");
        exit(errno);
    }
    
    sprintf(path, "%s/%s_statistica.txt", dirpath, filename);
    return path;
}


int main(int argc, char** argv)
{
    if(argc != 4 ) 
    {
        perror("Usage ./program <input_dir> <output_dir> <character>");
        exit(errno);
    }

    if(isalnum(argv[3][0]) == 0)
    {
        perror("Third argument must be an alphanumeric character\n");
        exit(errno);
    }

    character = argv[3][0];

    //open directory from path received in command line arguments aray
    DIR* dirIn = openDir(argv[1]); 

    //initializes pid variable and status variable to be used when creating processes
    int pid = -1, status = -1;

    //counts number of processes spawned
    int count = 0;

    //count number of lines returned by the shell script in each child process
    int lines = 0;

    //reads directory entries until the end
    struct dirent* entry;
    while((entry = readdir(dirIn)) != NULL)
    {
        //generates relative path of current entry to program.c
        char* relpath = generateRelativePath(argv[1], entry->d_name);

        //generates path of the _statistics.txt file
        char* outpath = generateOutputPath(argv[2], entry->d_name);

        // //gets info on current entry
        struct stat info;
        getStat(relpath, &info);

        //gets the type of the current entry: 0 - dir, 1 - symlink, 2 - regular file, -1 - unknown
        int type = getEntryType(info.st_mode);
        
        if((pid = fork()) < 0) //creates new child process for writing in file
        {
            perror("Error at forking\n");
            exit(errno);
        }

        if(pid == 0) //is a child process
        {
            //creates statistics file in output directory
            int fileout = createFile(outpath);

            //number of correct lines for current entry, zero if not a regular file or has no correct lines
            int lines = 0;

            //processes entry and writes in output file according to entry type
            processEntry(relpath, type, &info, fileout, ChildIn, OutParent, &lines);

            //close output file
            closeFile(fileout);

            //exits current process
            exit(count);
        }

        //increments number of processes
        count++;
        
        //frees memory allocated with functions
        free(relpath); free(outpath);
        
        //adds a waiting time of 1 second between processes
        sleep(1);
    }

    //goes through all child processes and waits for them to be completed
    for(int i = 0; i < count; i++)
    {
        //gets pid of the last child process completed
        int p = wait(&status);

        //checks for errors
        if(p < 0)
        {
            perror("Error at finishing process\n");
            exit(status);
        }

        //prints the status of each process
        if(!WIFEXITED(status))
        {
            perror("Process finished anormally\n");
            exit(status);
        }

        //prints process and status
        printf("Process with PID %d finished with status %d\n", p, WEXITSTATUS(status));
    }

    printf("Au fost identificate in total %d propozitii corecte care contin caracterul %c\n", lines, character);

    //closes input directory
    closeDir(dirIn);

    return 0;
}