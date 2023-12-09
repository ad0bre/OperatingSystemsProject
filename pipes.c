#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    // Check if the number of arguments is correct
    if (argc != 2) {
        printf("Usage: %s <number of lines>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open the input file for reading
    int inputFile = open("test2.txt", O_RDONLY);
    if (inputFile == -1) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    // Create a pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Error creating pipe");
        exit(EXIT_FAILURE);
    }

    // Fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        perror("Error forking process");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // Child process (script.sh)
        // Close write end of the pipe
        close(pipefd[1]);

        // Redirect standard input to read from the pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Execute the script.sh using execlp
        execlp("./script.sh", "./script.sh", &argv[1][0], (char *)NULL);

        // If execlp fails
        perror("Error executing script.sh");
        exit(EXIT_FAILURE);
    } else {  // Parent process (C program)
        // Read the content from the input file and write it to the pipe
        char buffer[4096];
        ssize_t bytesRead;
        ssize_t totalBytesWritten = 0;

        while ((bytesRead = read(inputFile, buffer, sizeof(buffer))) > 0) {
            ssize_t bytesWritten = write(pipefd[1], buffer, bytesRead);
            if (bytesWritten == -1) {
                perror("Error writing to pipe");
                exit(EXIT_FAILURE);
            }
            totalBytesWritten += bytesWritten;
        }

        printf("Wrote to pipe: %zd bytes\n", totalBytesWritten);

        // Close write end of the pipe
        close(pipefd[1]);

        // Wait for the child process to finish
        wait(NULL);

        // Read the output value from the script
        char outputBuffer[10];
        ssize_t bytesReadFromPipe = read(pipefd[0], outputBuffer, sizeof(outputBuffer));

        if (bytesReadFromPipe == -1) {
            perror("Error reading from pipe");
            exit(EXIT_FAILURE);
        }

        // Close the read end of the pipe
        close(pipefd[0]);

        int outputValue = atoi(outputBuffer);

        printf("Read from pipe: %s\n", outputBuffer);
        printf("Number of lines expected: 2 \t Number of lines found: %d\n", outputValue);

        // Close the input file
        close(inputFile);
    }

    return 0;
}
