#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAX 1000

FILE *fptr;
char *parsed[MAX];
int Background[MAX];
int BackGroundIndex=0;

void logfile(int ErrorType)
{
    fptr=fopen("/tmp/Process.log","a+");
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
    else if(ErrorType==6)
        fprintf(fptr,"INFO: Background programs terminated\n");
    else if(ErrorType==7)
        fprintf(fptr,"INFO: Shell terminated\n");
    else if(ErrorType==8)
        fprintf(fptr,"INFO: Shell initiated\n");
    else
        fprintf(fptr,"INFO: Child process %s\" was terminated\n",command);
    fclose(fptr);
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

void execute(int last)
{
    if(strcmp("cd",parsed[0])!=0)
    {
        signal(SIGCHLD,logfile);
        pid_t pid = fork();
        if (strcmp(parsed[last-1],"&")==0)
            Background[BackGroundIndex++]=pid;
        if (!pid)
        {
            if (strcmp(parsed[last-1],"&")==0)
                parsed[last-1]=NULL;
            if (execvp(parsed[0],parsed)<0)
            {
                char command[MAX]="";
                int i=0;
                while (parsed[i]!=NULL)
                {
                    strcat(command,parsed[i]);
                    ++i;
                    if(parsed[i]!=NULL)
                        strcat(command," ");
                }
                printf("Could not execute command %s\n", command);
                logfile(3);
            }
            exit(0);
        }
        else
        {
            sleep(1);
            if (strcmp(parsed[last-1],"&")!=0)
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

void BeforeExit()
{
    for(int i=0;i<BackGroundIndex;++i)
        kill(Background[i],SIGTERM);
    if(BackGroundIndex)
        logfile(6);
    logfile(7);
}

int main()
{
    logfile(8);
    char *LineStart=strcat(getenv("USER"),"'s Shell >>> ");
    while(1)
    {
        signal(SIGHUP,BeforeExit);
        fflush(stdin);
        char *command = readline(LineStart);
        if (strlen(command)!=0)
        {
            add_history(command);
            int index=ParseCommand(command);
            if(!strcmp("exit",parsed[0])&&parsed[1]==NULL)
            {
                BeforeExit();
                exit(0);
            }
            execute(index);
        }
    }
}