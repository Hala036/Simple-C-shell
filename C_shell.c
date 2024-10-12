#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <wait.h>
#include <fcntl.h>

#define MAX 1024

int cmd=0;

bool and_or(char** command, int* jobIndex);
void toFile(char* command, char** filename, int i, int* saved);
int redirectToFile(char* filename);
bool jobs(char** command);
void analyse(char** args, bool* success, int* jobIndex);
bool analyse2nd(char** args, int* jobIndex);
void process(char input[MAX], char** args, bool* quotes);
bool execute(char** args, int argc, int* jobIndex);
void exitShell(int quotes_count);

typedef struct J{
    int index;
    int pid;
    char* command;
    struct J* next;
}Job;
Job* head = NULL;

void addJob(char** args, int argc, int pid, int index){
    Job* j = (Job*) malloc(sizeof(Job));
    if(head==NULL){
        j->next=NULL;
        j->pid=pid;
        j->index=index;
        j->command=(char*) malloc(MAX* sizeof(char));
        char ss[MAX]="";
        for (int i = 0; i < argc; ++i) {
            strcat(ss, args[i]);
            strcat(ss, " ");
        }
        strcpy(j->command, ss);
        head= j;
    }
    else{
        Job* current=head;
        while (current->next!=NULL)
            current=current->next;
        j->pid=pid;
        j->index=index;
        j->next=NULL;
        j->command=(char*) malloc(MAX* sizeof(char));
        char ss[MAX]="";
        for (int i = 0; i < argc; ++i) {
            strcat(ss, args[i]);
            strcat(ss, " ");
        }
        strcpy(j->command, ss);
        current->next=j;
    }
}

void deleteJob(int pid){
    if(head->pid==pid){
        Job* temp=head;
        head= head->next;
        free(temp->command);
        free(temp);
        return;
    }
    Job* cur=head;
    while(cur->next!=NULL){
        if(cur->next->pid==pid){
            Job* temp=cur->next;
            cur->next=temp->next;
            free(temp->command);
            free(temp);
            return;
        }
        cur=cur->next;
    }
}

void prompt() {
    printf("#cmd:%d|#alias:0|#script lines:0> ", cmd); //prompt
}

bool and_or(char** command, int* jobIndex){
    bool success=false;
    char **command1;
    char **command2;
    char **command3=NULL;
    char *second;
    int i=0, b=0, c=0;
    command1=command;
    while(command[i]!=NULL && strcmp(command[i],"&&")!=0 && strcmp(command[i],"||")!=0){
        i++;
    }
    char* first= command[i];
    command1[i++]= NULL;

    command2= command+i;
    while(command[i]!=NULL && strcmp(command[i],"&&")!=0 && strcmp(command[i],"||")!=0){
        i++;
        b++;
    }

    if(command[i]!=NULL) {
        second = command[i++];
        command2[b]=NULL;
        command3=command+i;
        while(command[i]!=NULL){
            i++;
            c++;
        }
        command3[c]=NULL;
    }
    else{
        command2[b]=NULL;
    }

    if(strcmp(first,"&&")==0){
        success= analyse2nd(command1, jobIndex);
        if(success){
            success= analyse2nd(command2, jobIndex);
        }
        if(command3!=NULL){
            if(strcmp(second,"&&")==0 && success)
                success= analyse2nd(command3, jobIndex);
            else if(strcmp(second,"||")==0 && !success)
                success= analyse2nd(command3, jobIndex);
        }
    }

    else if(strcmp(first,"||")==0){
        success= analyse2nd(command1, jobIndex);
        if(!success){
            success= analyse2nd(command2, jobIndex);
            if(command3!=NULL){
                if(strcmp(second,"&&")==0 && success)
                    success= analyse2nd(command3, jobIndex);
                else if(strcmp(second,"||")==0 && !success)
                    success= analyse2nd(command3, jobIndex);
            }
        }
    }
    return success;
}

bool readFile(char* filename){
    FILE* fp= fopen(filename, "r");
    if(fp==NULL){
        perror("f_open");
        return false;
    }
    char line[MAX];
    while(fgets(line, MAX, fp)!=NULL){
        fprintf(stdout, "%s", line);
    }
    cmd++;
    fclose(fp);
    return true;
}

void toFile(char* command, char** filename, int i, int* saved){
    int size= (int)strlen(command)-i;

    strncpy(*filename, command+i+3, size);

    (*saved)= redirectToFile(*filename);

    char * ptr= strchr(command, '(');
    if(ptr !=NULL)
        command[i]='\0';
}

int redirectToFile(char* filename) {
    int saved = dup(STDERR_FILENO);
    if (saved < 0) {
        perror("dup");
        exit(1);
    }

    int dp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dp < 0) {
        perror("open");
        close(saved);
        exit(1);
    }
    if (dup2(dp, STDERR_FILENO) < 0) {
        perror("dup2");
        close(saved);
        exit(1);
    }
    close(dp);
    return saved;
}

void restoreDirection(int saved){
    if(dup2(saved, STDERR_FILENO)<0){
        perror("dup2");
        exit(1);
    }
    close(saved);
}

bool jobs(char** command){
    char spaces[16]="               ";
    if( command[1]!=NULL)
        return false;
    Job* cur=head;
    while (cur != NULL) {
        printf("[%d]%s%s&\n",cur->index, spaces, cur->command);
        cur= cur->next;
    }
    cmd++;
    return true;
}

bool analyse2nd(char** args, int* jobIndex){
    bool success= false;
    int i=0;
    if(strcmp(args[0],"jobs")==0){
        success= jobs(args);
    }
    else if(strcmp(args[0],"cat")==0){
        success= readFile(args[1]);
        return success;
    }
    else {
        while (args[i] != NULL) {
            i++;
        }
        success = execute(args, i, jobIndex);
    }
    return success;
}

void analyse(char** args, bool* success, int* jobIndex){
    int i=0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "&&") == 0 || strcmp(args[i], "||") == 0) {
            (*success) = and_or(args, jobIndex);
            return;
        }
        i++;
    }

    (*success) = analyse2nd(args, jobIndex);
}

bool numOfArgs(char** check, int len){
    int num=len;
    int argsInQuotes[MAX], k = 0, limit=5;
    for (int i = 0; i < len; ++i) {
        char* q1, * q2;
        q1 = strchr(check[i], '\'');
        q2 = strchr(check[i], '\"');
        if (q1 != NULL || q2 != NULL) {
            argsInQuotes[k++] = i;
        }
    }
    for (int i = 0; i < k - 1; i += 2) {
        num -= (argsInQuotes[i + 1] - argsInQuotes[i]);
    }

    if (num > limit) {
        return false;
    }
    return true;
}

void process(char input[MAX], char** args, bool* quotes){
    int n= (int)strlen(input);
    if(strcmp(input, "\0")==0)
        return;

    (*quotes) = false;
    for (int i = 0; i < n; ++i) {
        if (input[i] == '\'' || input[i] == '\"') {
            (*quotes) = true;
            break;
        }
    }

    //check size of command line
    if (n > MAX) {
        fprintf(stderr,"ERR\n");
        input[0]='\0';
        return;
    }

    //check number of arguments with token
    char* t;
    t = strtok(input, " ");

    int num = 0, len = 0;
    char** check;
    check = malloc(30 * sizeof(char*));
    if(check==NULL){
        perror("malloc");
        exit(1);
    }

    while (t != NULL) {
        num++;
        check[len++] = t;
        t = strtok(NULL, " ");
    }
    char newInput[MAX]="";
    for (int i = 0; i < len; ++i) {
        strcat(newInput, check[i]);
        strcat(newInput, " ");
    }

    //recount arguments according to quotes
    bool andor= false;
    for (int i = 0; i < len; ++i) {
        if(strcmp(check[i],"&&")==0 || strcmp(check[i],"||")==0){
            andor=true;
            break;
        }
    }
    bool argNum;
    if(!andor)
        argNum= numOfArgs(check, len);
    else{
        int a=0, b=0;
        for (int i = 0; check[i]!=NULL ; ++i) {
            if(strcmp(check[i],"&&")==0 || strcmp(check[i],"||")==0){
                argNum= numOfArgs(check+a, i-b);
                if(!argNum)
                    break;
                a=i+1;
                b=i+2;
            }
        }
    }
    free(check);

    if(!argNum){
        fprintf(stderr,"ERR: too many arguments\n");
        input[0]='\0';
        return;
    }

    for (int i = 0; i < strlen(newInput); ++i) {
        if(newInput[i]=='2' && newInput[i+1]=='>'){
            newInput[i]= '\0';
        }
    }

    //remove quotes
    char input1[MAX]="";
    char* tokenQ, * c = "'\"";
    tokenQ = strtok(newInput, c);
    while (tokenQ != NULL) {
        strcat(input1, tokenQ);
        tokenQ = strtok(NULL, c);
    }

    //remove brackets
    char input2[MAX]="";
    char* tokenB;
    c = "(";
    tokenB = strtok(input1, c);
    while (tokenB != NULL) {
        strcat(input2, tokenB);
        tokenB = strtok(NULL, c);
    }

    char input3[MAX]="";
    char* tokenB2;
    c = ")";
    tokenB2 = strtok(input2, c);
    while (tokenB2 != NULL) {
        strcat(input3, tokenB2);
        tokenB2 = strtok(NULL, c);
    }

    //remove whitespace
    int argc = 0;

    char* tokenS;
    tokenS = strtok(input3, " ");
    while (tokenS != NULL) {
        args[argc++] = tokenS;
        tokenS = strtok(NULL, " ");
    }
    args[argc] = NULL;

}

void sig_handler(int signum) {
    pid_t pid;
    int status;
    while ((pid= waitpid(-1, &status, WNOHANG))> 0) {
        deleteJob(pid);
        cmd++;
    }
}

bool execute(char** args, int argc, int* jobIndex) {
    bool success=false, background=false;
    if(strcmp(args[argc-1], "&")==0) {
        background = true;
        argc--;
        args[argc]=NULL;
    }
    if(strcmp(args[0],"echo")==0) {
        char s[MAX];
        for (int i = 1; i<argc ; ++i) {
            strcat(s,args[i]);
            strcat(s," ");
        }
        int y = (int)strlen(s)-1;
        memcpy(args[1], s, y);
        args[2] = NULL;
        strcpy(s, "");
    }

    pid_t pid;
    int status;
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "%s: Command not found.\n", args[0]);
            exit(1);
        }
    }
    else {
        if(!background) {
            wait(&status);
            if (WEXITSTATUS(status) == 0) {
                cmd++;
                success = true;
            }
        }
        else{
            int t=cmd;
            addJob(args, argc, pid, (*jobIndex));
            printf("[%d] %d\n", (*jobIndex), pid);
            (*jobIndex)++;
            signal(SIGCHLD, sig_handler);
            if(t!=cmd)
                success=true;
        }
    }
    return success;
}

void exitShell(int quotes_count) {
    printf("commands with quotes: %d", quotes_count);
    if(head != NULL) {
        Job* temp=head;
        head=head->next;
        free(temp->command);
        free(temp);
    }
    exit(0);
}

int main() {
    int jobIndex=1;
    int quotes_count = 0;
    while (1) {
        bool quotes, success;
        int len;
        prompt();
        char input[MAX];
        if (fgets(input, MAX, stdin) == NULL) {
            perror("Failed to read input\n");
            break;
        }
        else {
            len = (int)strlen(input);
            if (len > 0 && input[len - 1] == '\n')
                input[len - 1] = '\0';
        }

        if (strcmp(input, "exit_shell") == 0) {
            cmd=0;
            break;
        }

        if(strcmp(input, "")==0)
            continue;

        char* filename=(char*) malloc(MAX*sizeof(char));
        int saved;
        if(filename==NULL){
            perror("malloc");
            exit(1);
        }
        strcpy(filename, "");
        for (int i = 0; i < strlen(input); ++i) {
            if(input[i]=='2' && input[i+1]=='>'){
                toFile(input, &filename, i, &saved);
            }
        }

        char** args = (char**) malloc(30 * sizeof(char*));
        if (args == NULL) {
            perror("malloc");
            exit(1);
        }

        process(input, args, &quotes);
        if(strcmp(input, "")==0)
            continue;
        analyse(args, &success, &jobIndex);
        if(quotes && success)
            quotes_count++;
        if(filename!=NULL && strlen(filename)!=0)
            restoreDirection(saved);
        free(args);
        free(filename);
    }
    exitShell(quotes_count);
    exit(0);
}
