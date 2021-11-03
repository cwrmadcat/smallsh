#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CHAR_SIZE 2048

//Free memory allocated to each separate command string.
void freeMem(char** args, int argc) {
    for (int i = 0; i < argc; i++) {
        free(args[i]);
    }
}

//Function to tokenize and save each command line argument.
void processArgs(char* cmdLine, char** args, int* argc) {
    char* token;
    char* rest = cmdLine;

    while (token = strtok_r(rest, " ", &rest)) {
        args[*argc] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(args[*argc], token);
        //Increase argument count after every iteration.
        (*argc)++;
    }
}

//Routing function that analyzes the commands present in the args array.
void router(char** args, int argc, int status) {
    char reDirInt[] = "<";
    char reDirOut[] = ">";
    char cdCmd[] = "cd";
    char exitCmd[] = "exit";
    char statusCmd[] = "status";
    char lsCmd[] = "ls";
    //cd command (Built in)
    if (strcmp(args[0], cdCmd) == 0) {
        if (argc == 1) {
            chdir(getenv("HOME"));
            freeMem(&*args, argc);
        } else {
            chdir(args[1]);
            freeMem(&*args, argc);
        }
    //exit command (Built in)
    } else if (strcmp(args[0], exitCmd) == 0) {
        freeMem(&*args, argc);  //Free all memory prior to exit.
        exit(0);
    //status command (Built in)
    } else if (strcmp(args[0], statusCmd) == 0) {
        printf("Exit value %d\n", status);
        freeMem(&*args, argc);
    //Fork process and use exec
    } else if (strcmp(args[0], lsCmd) == 0) {
        int childStatus;
        int childPid;
        pid_t spawnPid = -5;
        spawnPid = fork();
        if (strcmp(args[1]), )
        if (spawnPid == 0) {
            execlp("ls", "ls", NULL);
        } else {
            childPid = wait(&childStatus);
        }
        freeMem(&*args, argc);
    }
}

int main() {
    char cmdLine[MAX_CHAR_SIZE];
    char* args[512];
    int status = 0;

    while (1) {
        int argc = 0;
        printf(": ");
        fflush(stdout);
        fgets(cmdLine, MAX_CHAR_SIZE, stdin);
        //Check if command line begins with a comment # or a newline.
        //Process command line otherwise.
        if (cmdLine[0] == '#' || cmdLine[0] == '\n') {
            continue;
        } else {
            //Modify command line input to eliminate newline before processing.
            cmdLine[strlen(cmdLine)-1] = '\0';
            processArgs(cmdLine, &*args, &argc);
            router(&*args, argc, status);
        }
    }

    return 0;
}