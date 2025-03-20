#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

char prev[1024]; // Global variable to store the previous directory

int handle_exit(int status) {
    return status != 0 ? status : 0;
}

void handle_cd(char* path) {
    char curr[1024];

    // Save the current directory before changing
    if (getcwd(curr, sizeof(curr)) == NULL) {
        perror("getcwd() error");
        return;
    }

    if (strcmp(path, "-") == 0) {
        if (chdir(prev) == -1) {
            perror("cd failed");
            return;
        }
        printf("%s\n", prev);
    } else {
        if (chdir(path) == -1) {
            perror("cd failed");
            return;
        }
    }

    // Update the previous directory
    strncpy(prev, curr, sizeof(prev));

    // Update the PWD environment variable
    if (getcwd(curr, sizeof(curr)) == NULL) {
        perror("getcwd() error");
        return;
    }

    if (setenv("PWD", curr, 1) == -1) {
        perror("setenv() error");
    }
}

void showpid(int idList[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("%d\n", idList[i]);
    }
}

int main(int ac, char** argv) {
    char input[1024];
    char *token;
    char* List[10];
    int i;
    char* command = NULL;
    pid_t pid;
    int execStatus, status;
    int idList[5] = {0};
    int a = 0;

   
    if (getcwd(prev, sizeof(prev)) == NULL) {
        perror("getcwd() error");
        exit(1);
    }

    for (i = 0; i < 10; i++) {
        List[i] = NULL;
    }

    while (1) {
        char* curr = getcwd(NULL, 0);
        if (curr == NULL) {
            perror("getcwd");
            exit(1);
        } else {
            printf("\033[0;32m%s$ ", curr);
            printf("\033[0m");
            free(curr);
        }

        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("problem with input");
            exit(1);
        }

        strtok(input, "\n");
        token = strtok(input, " ");
        if (token == NULL) {
            continue;
        }

        i = 0;
        while (token != NULL) {
            if (i == 0) {
                command = strdup(token);
            }
            List[i] = strdup(token);
            token = strtok(NULL, " ");
            i++;
        }

        if (strcmp(command, "cd") == 0) {
            handle_cd(List[1]);
        } else if (strcmp(command, "exit") == 0) {
            printf("exiting shell....");
            break;
        } else if (strcmp(command, "showpid") == 0) {
            showpid(idList, 5);
        } else {
            if ((pid = fork()) == 0) {
                execStatus = execvp(command, List);
                if (execStatus == -1) {
                    perror("Error: command not executed\n");
                    exit(1);
                }
            } else if (pid > 0) {
                if (a == 5) {
                    a = 0;
                }
                idList[a] = pid;
                a++;
                waitpid(pid, &status, 0);
            } else {
                perror("fork failed");
            }
        }

        for (i = 0; i < 10; i++) {
            if (List[i] != NULL) {
                free(List[i]);
                List[i] = NULL;
            }
        }

        if (command != NULL) {
            free(command);
            command = NULL;
        }
    }

    return EXIT_SUCCESS;
}
