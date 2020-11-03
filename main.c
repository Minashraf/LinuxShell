#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX 1000

FILE *fptr;
char *process;

void removeLeadingSpaces(char *str)
{
    char str1[MAX];
    int count = 0,k,j;

    while (str[count] == ' ')
        ++count;

    for (j = count, k = 0;str[j] != '\0'; j++, k++)
        str1[k] = str[j];

    str1[k-1] = '\0';
    strcpy(str, str1);
}

void trimTrailing(char *str)
{
    int index=-1,i=0;
    while(str[i] != '\0')
    {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
            index= i;

        i++;
    }
    str[index+1]='\0';
}

void ChangeDirectory(char *path)
{
    int fail;
    if(!strcmp(path,"~"))
        fail=chdir(getenv("HOME"));
    else
        fail=chdir(path);
    if (fail)
        printf("Failed to go to %s",path);
}

void logfile()
{
    fprintf(fptr,"Child process (%s) was terminated\n",process);
}

void execute(char *command)
{
    char *argument = strtok(command, " ");
    char *parsed[MAX];
    int index=0;
    while(argument!=NULL)
    {
        parsed[index++]=argument;
        argument = strtok(NULL, " ");
    }
    int i=index;
    while(i)
    {
        if(parsed[i]==NULL)
            break;
        parsed[i]=NULL;
        ++i;
    }
    process=*parsed;
    if(strcmp("cd",parsed[0])!=0)
    {
        signal(SIGCHLD,logfile);
        pid_t pid = fork();
        if (!pid) {
            if (execvp(parsed[0], parsed) < 0)
                printf("Could not execute command %s\n", *parsed);
            exit(0);
        } else {
            if (strcmp(parsed[index - 1], "&")!=0)
                wait(NULL);
            sleep(1);
        }
    }
    else
        ChangeDirectory(parsed[1]);
}

int main() {
    fptr=fopen("Children.log","w");
    while(1)
    {
        fflush(stdin);
        printf("\n%s's Shell >>> ",getenv("USER"));
        char command[MAX];
        fgets(command, MAX, stdin);
        removeLeadingSpaces(command);
        trimTrailing(command);
        if(!strcmp("exit",command))
        {   printf("Goodbye!");
            fclose(fptr);
            exit(0);
        }
        execute(command);
    }
}