#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#define MAX 1000

FILE *fptr;
char *parsed[MAX];

void RemoveLeadingSpaces(char *str)
{
    char str1[MAX];
    int count = 0,k,j;

    while (str[count] == ' ' || str[count] == '\t')
        ++count;

    for (j = count, k = 0;str[j] != '\0'; j++, k++)
        str1[k] = str[j];

    str1[k-1] = '\0';
    strcpy(str, str1);
}

void TrimTrailing(char *str)
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

void logfile(int ErrorType)
{
    char command[MAX]="\"";
    int i=0;
    while (parsed[i]!=NULL)
    {
        strcat(command,parsed[i]);
        ++i;
        if(parsed[i]!=NULL)
            strcat(command," ");
    }
    time_t t;
    time(&t);
    fprintf(fptr,"%s ",strtok(ctime(&t), "\n"));
    if(ErrorType == 1)
        fprintf(fptr,"SUCCESS: Background process %s\"\n",command);
    else if(ErrorType == 2)
        fprintf(fptr,"INFO: Changing to directory %s\"\n",command);
    else if(ErrorType == 3)
        fprintf(fptr,"ERROR: command %s\" couldn't be executed\n",command);
    else if(ErrorType == 4)
        fprintf(fptr,"ERROR: Failed to reach directory %s\"\n",command);
    else if(ErrorType == 5)
        fprintf(fptr,"SUCCESS: reached directory %s\"\n",command);
    else
        fprintf(fptr,"INFO: Child process %s\" was terminated\n",command);
}

void ChangeDirectory(char *path)
{
    int fail;
    if(!strcmp(path,"~"))
        fail=chdir(getenv("HOME"));
    else
        fail=chdir(path);
    if (fail)
    {
        printf("Failed to go to %s",path);
        logfile(4);
    }
    else
        logfile(5);
}

int ParseCommand(char *command)
{
    char *argument = strtok(command, " ");
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
    return index;
}

void execute(char *command)
{
    int index=ParseCommand(command);
    if(strcmp("cd",parsed[0])!=0)
    {
        signal(SIGCHLD,logfile);
        pid_t pid = fork();
        if (!pid) {
            if (execvp(parsed[0], parsed) < 0)
            {
                printf("Could not execute command %s\n", *parsed);
                logfile(3);
            }
            exit(0);
        } else {
            sleep((unsigned int) 0.5);
            if (strcmp(parsed[index - 1], "&")!=0)
                wait(NULL);
            else
                logfile(1);
        }
    }
    else
    {
        logfile(2);
        ChangeDirectory(parsed[1]);
    }
}

int main() {
    fptr=fopen("Process.log","w");
    while(1)
    {
        fflush(stdin);
        printf("\n%s's Shell >>> ",getenv("USER"));
        char command[MAX];
        fgets(command, MAX, stdin);
        RemoveLeadingSpaces(command);
        TrimTrailing(command);
        if(!strcmp("exit",command))
        {   printf("Goodbye!");
            fclose(fptr);
            exit(0);
        }
        execute(command);
    }
}