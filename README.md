# Operating Systems Project
My project for the Operating Systems lab @ Faculty of Automatics and Computers, UPT

project made by: DOBRE Andrei-Lucian

---

To run the *program.c* file, download this project, then in the command line run:

```bash
gcc -g program.c -o program
./program dir out
```
The first command, `gcc -g program.c -o program` **compiles**[^1] the *program.c* file and generates the executable file *program* .

The second command, `./program dir out` runs the executable file *program*, taking the name of the attached directories *dir* and *out* as **parameters**[^2].

This C program takes a directory as input and, for each of it's entries, creates a statistics file in the *out* directory where informations about the corresponfing entry is written. The names of these statistics file are <name of the entry>_statistics.txt.

[^1]: uses GCC compiler, check if available
[^2]: this project uses a mock directory to demonstrate how the code functions, you are free to use any other directory you want
