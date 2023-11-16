#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// accept up to 16 command-line arguments
#define MAXARG 16

// allow up to 64 environment variables
#define MAXENV 64

// keep the last 500 commands in history
#define HISTSIZE 500

// accept up to 1024 bytes in one command
#define MAXLINE 1024

static char **parseCmd(char cmdLine[])
{
    char **cmdArg, *ptr;
    int i;

    //(MAXARG + 1) because the list must be terminated by a NULL ptr
    cmdArg = (char **)malloc(sizeof(char *) * (MAXARG + 1));
    if (cmdArg == NULL)
    {
        perror("parseCmd: cmdArg is NULL");
        exit(1);
    }
    for (i = 0; i <= MAXARG; i++) // note the equality
        cmdArg[i] = NULL;
    i = 0;
    ptr = strsep(&cmdLine, " ");
    while (ptr != NULL)
    {
        // (strlen(ptr) + 1)
        cmdArg[i] = (char *)malloc(sizeof(char) * (strlen(ptr) + 1));
        if (cmdArg[i] == NULL)
        {
            perror("parseCmd: cmdArg[i] is NULL");
            exit(1);
        }
        strcpy(cmdArg[i++], ptr);
        if (i == MAXARG)
            break;
        ptr = strsep(&cmdLine, " ");
    }
    return (cmdArg);
}

int main(int argc, char *argv[], char *envp[])
{
    char cmdLine[MAXLINE], **cmdArg;
    char history[HISTSIZE][MAXLINE] = { 0 };
    int history_head = 0, history_index = -1;
    int status, i, debug;
    pid_t pid;

    memset(history, 0, HISTSIZE * MAXLINE);

    debug = 0;
    i = 1;
    while (i < argc)
    {
        if (!strcmp(argv[i], "-d"))
            debug = 1;
        i++;
    }

    /* read env */
    char buffer[2048];
    char *key, *val;
    char *envKeys[MAXENV];
    char *envVals[MAXENV];

    int k = 0;
    while (envp[k] != NULL)
    {
        strcpy(buffer, envp[k]);
        val = buffer;
        key = strsep(&val, "=");
        
        envKeys[k] = (char*)malloc(sizeof(char) * strlen(key));
        envVals[k] = (char*)malloc(sizeof(char) * strlen(val));

        strcpy(envKeys[k], key);
        strcpy(envVals[k], val);

        k++;
    }

    int envSize = k;

    while ((1))
    {
        printf("bsh> ");                     // prompt
        fgets(cmdLine, MAXLINE, stdin);      // get a line from keyboard
        cmdLine[strlen(cmdLine) - 1] = '\0'; // strip '\n'
        cmdArg = parseCmd(cmdLine);
        if (debug)
        {
            i = 0;
            while (cmdArg[i] != NULL)
            {
                printf("\t%d (%s)\n", i, cmdArg[i]);
                i++;
            }
        }

        /* add command to history */
        if (history_index == -1) {
            history_index = 0;
        } else if (history_index == history_head) {
            history_head = (history_head + 1) % HISTSIZE;
        } 

        strcpy(history[history_index], cmdLine);
        history_index = (history_index + 1) % HISTSIZE;

        // built-in command exit
        if (strcmp(cmdArg[0], "exit") == 0)
        {
            if (debug)
                printf("exiting\n");

            
            for (int i = 0; envKeys[i] != NULL; i++) {
                free(envKeys[i]);
                free(envVals[i]);
            }

            break;
        }
        // built-in command env
        else if (strcmp(cmdArg[0], "env") == 0)
        {
            for (int i = 0; envKeys[i] != NULL; i++) {
                printf("%s=%s\n", envKeys[i], envVals[i]);
            }    
        }
        // built-in command setenv
        else if (strcmp(cmdArg[0], "setenv") == 0)
        {
            int replaced = 0;
            int i = 0;

            for (i = 0; envKeys[i] != NULL; i++) {
                if (strcmp(cmdArg[1], envKeys[i]) == 0) {
                    free(envVals[i]);
                    envVals[i] = (char*)malloc(sizeof(char) * strlen(cmdArg[2]));
                    strcpy(envVals[i], cmdArg[2]);

                    replaced = 1;
                    break;
                }
            }

            if (!replaced) {
                envKeys[i] = (char*)malloc(sizeof(char) * strlen(cmdArg[1]));
                envVals[i] = (char*)malloc(sizeof(char) * strlen(cmdArg[2]));

                strcpy(envKeys[i], cmdArg[1]);
                strcpy(envVals[i], cmdArg[2]);
            }
        }
        // built-in command unsetenv
        else if (strcmp(cmdArg[0], "unsetenv") == 0)
        {
            int i;

            for (i = 0; envKeys[i] != NULL && strcmp(cmdArg[1], envKeys[i]) != 0; i++);

            if (envKeys[i] != NULL) {
                free(envKeys[i]);
                free(envVals[i]);
            }

            for (i; envKeys[i+2] != NULL; i++) {
                envKeys[i] = envKeys[i + 1];
                envVals[i] = envVals[i + 1];
            }
        }
        // built-in command cd
        else if (strcmp(cmdArg[0], "cd") == 0)
        {
            int i;
            char path[MAXLINE];

            if (cmdArg[1] == NULL || strlen(cmdArg[1]) == 0) {
                for (i = 0; envKeys[i] != NULL; i++) {
                    if (strcmp("HOME", envKeys[i]) == 0) break;
                }

                if (chdir(envVals[i]) != 0) {
                    printf("Invalid path!\n");
                }
            } else {
                if (chdir(cmdArg[1]) != 0) {
                    printf("Invalid path!\n");
                }
            }

            getcwd(path, MAXLINE);

            for (i = 0; envKeys[i] != NULL; i++) {
                if (strcmp(envKeys[i], "PWD") == 0) {
                    free(envVals[i]);
                    envVals[i] = (char*)malloc(sizeof(char) * strlen(path));
                    strcpy(envVals[i], path);
                }
            }
        }
        // built-in command history
        else if (strcmp(cmdArg[0], "history") == 0)
        {
            int index;
            for (int i = 0; i < HISTSIZE; i++) {
                index = (history_head + i) % HISTSIZE;

                if (strlen(history[index]) > 0) {
                    printf("%s\n", history[index]);
                } else {
                    break;
                }
            }
        }

        // implement how to execute Minix commands here

        // the following is a template for using fork() and execv()
        //***remove this else case from your final code for bsh.c
        else
        {
            if (debug)
                printf("calling fork()\n");
            pid = fork();
            if (pid != 0)
            {
                if (debug)
                    printf("parent %d waiting for child %d\n", getpid(), pid);
                waitpid(pid, &status, 0);
            }
            else
            {
                status = execv(cmdArg[0], cmdArg);
                if (status)
                {
                    printf("\tno such command (%s)\n", cmdArg[0]);
                    return 1;
                }
            }
        }
        //***remove up to this point

        // clean up before running the next command
        i = 0;
        while (cmdArg[i] != NULL)
            free(cmdArg[i++]);
        free(cmdArg);
    }

    return 0;
}
