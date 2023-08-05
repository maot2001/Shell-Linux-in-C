#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void sep(char **temp);
void sep2(char **temp);
char **spliter(char *line);

#define MAX_HIST 100
int main(int argc, char *argv[])
{
    int bufsize = MAX_HIST;
    char **args = (char **)malloc(bufsize * sizeof(char *));
    char *printeable[10];
    int index = 0;
    int index2 = 0;
    if (!args) { perror("ERROR"); exit(EXIT_FAILURE); }

    char *ruta = argv[0];
    strcat(ruta, "/historial");
    int in = open(ruta, O_RDONLY);
    if(in == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

    int dup2in = dup2(in, 0);
    if(dup2in == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

    close(in);

    for (;; index++)
    {
        if (index >= bufsize)
        {
            bufsize += MAX_HIST;
            args = (char **)realloc(args, bufsize * sizeof(char*));
            if (!args) { perror("ERROR"); exit(EXIT_FAILURE); }
        }
        
        args[index] = (char *)malloc(128 * sizeof(char));
        if (!args[index]) { perror("ERROR"); exit(EXIT_FAILURE); }

        if (fgets(args[index], 128, stdin)) continue;
        index--; 
        break;
    }
    
    if (index > 9) index2 = 9;
    else index2 = index;
    
    for (index2; index2 > -1; index2--)
    {
        printeable[index2] = args[index];
        index--;
    }
    
    sep(printeable);


    return 0;
}

void sep(char **temp)
{
    char *p;
    for (int i = 0; temp[i] != NULL && i < 10; i++)
    {
        p = temp[i];
        printf("%d: %s", i+1, p);
    }    
}

void sep2(char **temp)
{
    char *p;
    for (int i = 0; temp[i] != NULL && i < 10; i++)
    {
        p = temp[i];
        printf("%s", p);
    }    
}

#define JUMP "\n"
#define MAX_LEN 128
char **spliter(char *line)
{
    int bufsize = MAX_LEN;
    int position = 0;
    char **tokens = (char **)malloc(bufsize * sizeof(char*));
    char *tok;

    if (!tokens) { perror("ERROR"); exit(EXIT_FAILURE); }
    
    tok = strtok(line, JUMP);
    while (tok != NULL) 
    {
        tokens[position] = tok;
        position++;
        if (position >= bufsize) 
        {
            bufsize += MAX_LEN;
            tokens = (char **)realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) { perror("ERROR"); exit(EXIT_FAILURE); }
        }
        tok = strtok(NULL, JUMP);
    }
    tokens[position] = NULL;
    return tokens;
}