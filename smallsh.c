#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_CHAR_SIZE 2048

//Free memory allocated to each separate command string.
void freeMem(char** args, int argc) {
    for (int i = 0; i < argc; i++) {
        free(args[i]);
    }
}

//Function to tokenize and save each command line argument.
void processArgs(char* cmdLine, char** args, char** execArgs, int* argc) {
    char* token;
    char* rest = cmdLine;

    while (token = strtok_r(rest, " ", &rest)) {
        args[*argc] = calloc(strlen(token) + 1, sizeof(char));
        execArgs[*argc] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(args[*argc], token);
        strcpy(execArgs[*argc], token);
        //Increase argument count after every iteration.
        (*argc)++;
    }
    execArgs[*argc] = NULL;
}

//Routing function that analyzes the commands present in the args array.
//Matches token to commands and executes.
void router(char** args, char** execArgs, int argc, int* status) {
    //Creating template strings for the various shell commands
    char reDirInt[] = "<";
    char reDirOut[] = ">";
    char cdCmd[] = "cd";
    char exitCmd[] = "exit";
    char statusCmd[] = "status";
    char lsCmd[] = "ls";
    char catCmd [] = "cat";
    char pwdCmd[] = "pwd";
    char echoCmd[] = "echo";
    char wcCmd[] = "wc";
    char testCmd[] = "test";
    //Initializing variables used in forking
    int childStatus;
    int childPid;
    pid_t spawnPid = -5;
    //cd command (Built in)
    if (strcmp(args[0], cdCmd) == 0) {
        if (argc == 1) {
            chdir(getenv("HOME"));
        } else {
            chdir(args[1]);
        }
    //exit command (Built in)
    } else if (strcmp(args[0], exitCmd) == 0) {
        freeMem(args, argc);  //Free all memory prior to calling exit.
        freeMem(execArgs, argc);
        exit(0);
    //status command (Built in)
    } else if (strcmp(args[0], statusCmd) == 0) {
        printf("exit value %d\n", *status);
    //ls command - Fork process and use execlp
    } else if (strcmp(args[0], lsCmd) == 0) {
        //If arg count is greater than 1, check if the next argument is redirection.
        if (argc > 1) {
            if (strcmp(args[1], reDirOut) == 0) {
                int targetFD = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0640);
                spawnPid = fork();
                if (spawnPid == 0) {
                    int result = dup2(targetFD, 1);
                    execlp(args[0], args[0], NULL);
                } else {
                    childPid = wait(&childStatus);
                    *status = WIFEXITED(childStatus);
                }
            }
        //Run ls command with no additional arguments.
        } else {
            spawnPid = fork();
            if (spawnPid == 0) {
                execvp(args[0], execArgs);
            } else {
                childPid = wait(&childStatus);
                *status = WIFEXITED(childStatus);
            }
        }
    //cat command - Fork process and use execlp
    } else if (strcmp(args[0], catCmd) == 0 && argc > 1) {
        spawnPid = fork();
        if (spawnPid == 0) {
            execvp(args[0], execArgs);
        } else {
            childPid = wait(&childStatus);
            *status = WIFEXITED(childStatus);
        }
    //pwd command - Fork process and use execlp
    } else if (strcmp(args[0], pwdCmd) == 0) {
        spawnPid = fork();
        if (spawnPid == 0) {
            execvp(args[0], execArgs);
        } else {
            childPid = wait(&childStatus);
            *status = WIFEXITED(childStatus);
        }
    //echo command - Fork process and use execvp
    } else if (strcmp(args[0], echoCmd) == 0) {
        spawnPid = fork();
        if (spawnPid == 0) {
            execvp(args[0], execArgs);
        } else {
            childPid = wait(&childStatus);
            *status = WIFEXITED(childStatus);
        }
    //wc command - Fork process and use execvp
    } else if (strcmp(args[0], wcCmd) == 0) {
        //Redirection to console routed to a new file
        if (argc > 3) {
            if (strcmp(args[1], reDirInt) == 0 && strcmp(args[3], reDirOut) == 0) {
                int targetFD = open(args[4], O_WRONLY | O_CREAT | O_TRUNC, 0640);
                spawnPid = fork();
                if (spawnPid == 0) {
                    int result = dup2(targetFD, 1);
                    execlp(args[0], args[0], args[2], NULL);
                } else {
                    childPid = wait(&childStatus);
                    *status = WIFEXITED(childStatus);
                }
            }
        //Redirection to console    
        } else if (strcmp(args[1], reDirInt) == 0) {
            spawnPid = fork();
            if (spawnPid == 0) {
                execlp(args[0], args[0], args[2], NULL);
                fflush(stdout);
            } else {
                childPid = wait(&childStatus);
                *status = WIFEXITED(childStatus);
            }
        }
    //test command - Fork process and use execvp    
    } else if (strcmp(args[0], testCmd) == 0) {
        spawnPid = fork();
        if (spawnPid == 0) {
            execvp(args[0], execArgs);
        } else {
            childPid = wait(&childStatus);
            *status = WIFEXITED(childStatus);
        }
    //catch all for command line arguments that haven't been implemented or don't exist.
    } else {
        printf("%s: no such file or directory\n", args[0]);
        fflush(stdout);
    }
    //Free memory allocated to token strings before returning to shell.
    freeMem(args, argc);
    freeMem(execArgs, argc);
}

int main() {
    char cmdLine[MAX_CHAR_SIZE];
    char* args[512];
    char* execArgs[512];
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
            processArgs(cmdLine, args, execArgs, &argc);
            router(args, execArgs, argc, &status);
        }
    }
    return 0;
}