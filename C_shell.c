#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <stdbool.h>
#define MAX 1024

int cmd_count = 0, alias_count = 0, script_count = 0, quotes_count = 0;
char pr[50];
bool quotes;

typedef struct {
    char* key;
    char* command;
}Alias;

Alias* aliases = NULL;
void splitKeyCmd(char* str, char* key, char* command, char c);
void addAlias(char* key, char* command);
void deleteAliasByKey(char* key);
void exitShell();

void prompt() {
    sprintf(pr, "#cmd:%d|#alias:%d|#script lines:%d> ", cmd_count, alias_count, script_count); //prompt
}

void addAlias(char* key, char* command) {
    char com[MAX]="", k[MAX]=""; //remove whitespace from key and command
    char* tt= strtok(command, " ");
    while(tt!=NULL){
        strcat(com, tt);
        tt= strtok(NULL, " ");
        if(tt!=NULL)
            strcat(com, " ");
    }
    char* tk= strtok(key, " ");
    while(tk!=NULL){
        strcat(k, tk);
        tk= strtok(NULL, " ");
    }
    if(strcmp(k,"")==0 || strcmp(com,"")==0){
        printf("ERR\n");
        return;
    }
    if (aliases == NULL) {
        aliases = (Alias*)malloc(sizeof(Alias));
        if (aliases == NULL) {
            perror("malloc");
            exitShell();
        }
    }
    for (int i = 0; i < alias_count; ++i) { //if alias already exists, delete it to replace with new command
        if (strcmp(aliases[i].key, key) == 0) {
            deleteAliasByKey(key);
            break;
        }
    }

    Alias* temp = (Alias*)realloc(aliases, (alias_count + 1) * sizeof(Alias));
    if (temp == NULL) {
        perror("realloc");
        exitShell();
    }
    aliases = temp;

    aliases[alias_count].key = (char *) malloc((strlen(key) + 1) * sizeof(char));
    aliases[alias_count].command = (char *) malloc((strlen(com) + 1) * sizeof(char));

    if (aliases[alias_count].key == NULL || aliases[alias_count].command == NULL) {
        perror("malloc2");
        exitShell();
    }

    strcpy(aliases[alias_count].key, key);
    strcpy(aliases[alias_count].command, com);
    alias_count++;
    cmd_count++;
    if (quotes)
        quotes_count++;
}

void splitKeyCmd(char* str, char* key, char* command, char c) {
    char* index = strchr(str, c);
    if (index != NULL) {
        size_t len = index - str;
        strncpy(key, str, len);
        key[len] = '\0';
        strcpy(command, index + 1);
    }
    else {
        strcpy(key, str);
        command = NULL;
    }
}

void deleteAliasByKey(char* key) {
    bool found = false;
    for (int i = 0; i < alias_count; ++i) {
        if (strcmp(aliases[i].key, key) == 0) {
            free(aliases[i].key);
            free(aliases[i].command);
            found = true;
            for (int j = i; j < alias_count - 1; ++j) {
                aliases[j] = aliases[j + 1];
            }
            alias_count--;
            if (alias_count > 0) {
                Alias* temp = (Alias*)realloc(aliases, alias_count * sizeof(Alias));
                if (temp == NULL) {
                    perror("realloc");
                    exitShell();
                }
                aliases = temp;
            }
            else {
                free(aliases);
                aliases = NULL;
            }
            break;
        }
    }
    if (!found) {
        printf("ERR\n");
        cmd_count--;
        if(quotes)
            quotes_count--;
    }
}

void execute(char input[MAX], int n) {
    if(strcmp(input, "\0")==0)
        return;

    quotes = false;
    for (int i = 0; i < n; ++i) {
        if (input[i] == '\'' || input[i] == '\"') {
            quotes = true;
            break;
        }
    }

    //check size of command line
    if (n > MAX) {
        printf("ERR\n");
        return;
    }

    //check number of arguments with token
    char* t;
    t = strtok(input, " ");

    int num = 0, len = 0;
    char** check;
    check = malloc(10 * sizeof(char*));

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
    for (int i = 0; i < MAX; ++i) {
    }    if (quotes) {
        int argsInQuotes[MAX], k = 0;
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
    }
    free(check);
    if (num > 5) {
        printf("ERR\n");
        return;
    }

    //replace alias in command
    char temp[MAX]="";
    for (int i = 0; i < alias_count; ++i) {
        if(strncmp(newInput, aliases[i].key, strlen(aliases[i].key))==0 && (newInput[strlen(aliases[i].key)]==' ')){
            sprintf(temp, "%s %s", aliases[i].command, newInput + strlen(aliases[i].key) + 1);
            strcpy(newInput, temp);
            break;
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

    //remove whitespace
    char* tokenS;
    tokenS = strtok(input1, " ");
    int argc = 0;
    char** args;
    args = malloc(10 * sizeof(char*));
    while (tokenS != NULL) {
        args[argc++] = tokenS;
        tokenS = strtok(NULL, " ");
    }

    // **Alias command**
    if (strcmp(args[0], "alias") == 0) {
        char key[MAX] = "", command[MAX] = "";
        char arg1[MAX] = "";
        if(args[1]==NULL){
            for (int i = 0; i < alias_count; ++i) {
                fprintf(stdout, "%s = '%s'\n", aliases[i].key, aliases[i].command);
                cmd_count++;
            }
            return;
        }
        for (int i = 1; i<argc; ++i) {
            strcat(arg1, args[i]);
            strcat(arg1, " ");
        }
        splitKeyCmd(arg1, key, command, '=');
        addAlias(key, command);
        return;
    }

    // **Unalias command**
    if (strcmp(args[0], "unalias") == 0) {
        deleteAliasByKey(args[1]);
        cmd_count++;
        if (quotes)
            quotes_count++;
        return;
    }

    // **Source command**
    if (strcmp(args[0], "source") == 0) {
        FILE* fp;

        fp = fopen(args[1], "r");
        if (fp == NULL) {
            printf("ERR\n");
            return;
        }
        char bash[15] = "";

        fgets(bash, 15, fp);
        if (strcmp(bash, "#!/bin/bash")==0) {
            printf("ERR");
            return;
        }

        char command[MAX] = "";
        while(fgets(command, MAX, fp)!=NULL){
            int l=(int)strlen(command);
            if(command[0]!='#' && strcmp(command,"")!=0){
                if (l > 0 && command[l - 1] == '\n')
                    command[l - 1] = '\0';
                if(command[0]!='#')
                    execute(command, l);
            }
            script_count++;
        }
        fclose(fp);
        cmd_count++;
        return;
    }

    else {
        char s[MAX]="";

        for (int i = 1; args[i+1]!=NULL ; ++i) {
            strcat(s,args[i]);
            strcat(s," ");
        }
        memcpy(args[1],s, strlen(s));
        args[2]=NULL;

        pid_t pid;
        int status;
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exitShell();
        }
        if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("exec");
                exitShell();
            }
            exit(1);
        }
        else {
            wait(&status);
            if (WEXITSTATUS(status) == 0) {
                cmd_count++;
                if(quotes)
                    quotes_count++;
            }
        }
    }
    free(args);

}

void exitShell() {
    printf("%d\n", quotes_count);
    if (aliases != NULL) {
        for (int i = 0; i < alias_count; ++i) {
            free(aliases[i].key);
            free(aliases[i].command);
        }
        free(aliases);
    }
    exit(0);
}

int main() {
    while (1) {
        int len;
        prompt();
        printf("%s", pr);
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
            exitShell();
        }
        execute(input, len);
    }
}