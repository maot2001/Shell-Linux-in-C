#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

char **spliter(char *line, const char *separator);
void work(char **args);
void cd(char **UDargs);
char *reroot(char **args);
char **linked(char **args);
char **quitlinks(char **args);
void history(char *line);
char *againcomp(int number);
char *again(char *line);
char *parser(char *line);
void execute(char *line);
bool first(char *line);

#define MAX_LEN 512
#define SPACE " "
#define HASHTAG "#\n"
#define PIPE "|"

bool cont;
char root[MAX_LEN];
char line[MAX_LEN];
bool save;
int pipes;
bool error;
int main()
{
    cont = true;
    getcwd(root, 128);

    do {
        error = false;
        save = true;
        printf("my - prompt $ ");
        fgets(line, MAX_LEN, stdin);
        char *new_line = parser(line);
        if (error) { continue; }
        execute(new_line);
    } while(cont);

    return 0;
}

char *parser(char *line)
{
    char *new_line;
    save = first(line);
    if(!error) new_line = again(line);
    if(!error && save) { history(new_line); }
    return new_line;
}

bool first(char *line)
{
    if (!isalnum(line[0]) && line[0] != ' ') { 
        fprintf(stderr, "ERROR: syntax error\n"); 
        error = true; 
        return true; 
    }
    if (line[0] != ' ' && line[0] != '\n') { return true; }
    char line2[MAX_LEN];
    bool beggin = true;
    int index = 0;
    for (int i = 1; i < MAX_LEN; i++)
    {
        if(line[i] == ' ' && beggin) { continue; }
        beggin = false;
        line2[index++] = line[i];
    }
    line2[MAX_LEN-1] = '\0';
    strcpy(line, line2);
    return false;
}

void execute(char *line)
{
    if(line[0] == '#') return;
    char **pipers = spliter(spliter(line, HASHTAG)[0], PIPE);
    if(pipers[0] == NULL) return;
    if(pipers[1] == NULL) 
    {
        char **args = spliter(pipers[0], SPACE);
        if(args[0] == NULL) return;
        if(strcmp(args[0],"exit")==0) cont = false;
        else if(strcmp(args[0],"cd")==0) cd(args);
        else
        {
            pid_t pid = fork();
            if(pid == -1) { perror("Error"); exit(EXIT_FAILURE); }
            if(pid == 0)
            {
                char **new_args = linked(args);
                work(new_args);
            }
            waitpid(pid, NULL, 0);
        }
    }
    else
    {
        int cant = pipes;
        char ***commands = (char ***)malloc(cant * sizeof(char **));
        char ***new_commands = (char ***)malloc(cant * sizeof(char **));
        int pipe_fds[cant-1][2];
        pid_t pids[cant];

        for (int i = 0; i < cant; i++) { 
            first(pipers[i]);
            commands[i] = spliter(pipers[i], SPACE); 
        }

        for (int i = 0; i < cant-1; i++) {
            if (pipe(pipe_fds[i]) == -1) 
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < cant; i++) {
            pids[i] = fork();
            if (pids[i] == -1) 
            {
                perror("fork");
                exit(EXIT_FAILURE);
            } 
            else if (pids[i] == 0) 
            {
                if (i == 0) { dup2(pipe_fds[i][1], 1); } 
                else if (i == cant-1) { dup2(pipe_fds[i-1][0], 0); }
                else 
                {
                    dup2(pipe_fds[i-1][0], 0);
                    dup2(pipe_fds[i][1], 1);
                }

                for (int j = 0; j < cant-1; j++) {
                    if (i == j) {
                        close(pipe_fds[j][0]);
                        close(pipe_fds[j][1]);
                    } else {
                        close(pipe_fds[j][0]);
                        close(pipe_fds[j][1]);
                    }
                }
                                
                new_commands[i] = linked(commands[i]);
                work(new_commands[i]);
            }
        }

        for (int i = 0; i < cant-1; i++) {
            close(pipe_fds[i][0]);
            close(pipe_fds[i][1]);
        }

        for (int i = 0; i < cant; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }
}

char *again(char *line) {
    if (strncmp(line, "again ", 6) == 0) {
        char **args = spliter(line, SPACE);
        if (args[2] == NULL && args[1] != NULL) {
            int index = 0;
            for (int i = 0; i < strlen(args[1]) && args[1][i] != '\n'; i++) {
                if (!isdigit(args[1][i])) {
                    fprintf(stderr, "ERROR: Syntax error\n"); 
                    error = true;
                    return line;
                }
                index = index * 10 + (args[1][i] - '0');
            }
            if (index < 1 || index > 10) {
                fprintf(stderr, "ERROR: index out of the history\n"); 
                error = true;
                return line;
            }
            return againcomp(index);
        }
        else { 
            fprintf(stderr, "ERROR: Syntax error\n"); 
            error = true;
            return line;
        }        
    }    
    else { return line; }
}

char *againcomp(int number)
{
    FILE *archiv;
    int bufsize = MAX_LEN;
    char **argshist = (char **)malloc(bufsize * sizeof(char *));
    int index = 0;
    if (!argshist) { perror("ERROR"); exit(EXIT_FAILURE); }
    char ruta[strlen(root) + 11];
    strcpy(ruta, root);
    strcat(ruta, "/historial");
    archiv = fopen(ruta, "r");

    for (;; index++)
    {
        if (index >= bufsize)
        {
            bufsize += MAX_LEN;
            argshist = (char **)realloc(argshist, bufsize * sizeof(char*));
            if (!argshist) { perror("ERROR"); exit(EXIT_FAILURE); }
        }
                    
        argshist[index] = (char *)malloc(128 * sizeof(char));
        if (!argshist[index]) { perror("ERROR"); exit(EXIT_FAILURE); }

        if (fgets(argshist[index], 128, archiv)) continue;
        break;
    }

    if (number > index) { 
        fprintf(stderr, "ERROR2: Syntax error"); 
        error = true; 
        return NULL; 
    }
    else if (number == index) return argshist[index-1];
    else
    {
        if (index > 10) return argshist[index - 1 - (10 - number)];
        else return argshist[number-1];
    }
}

void history(char *line)
{  
    char file[11 + strlen(root)];
    strcpy(file, root);
    strcat(file, "/historial");
    int out = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(out == -1) { perror("ERROR"); error = true; return; }

    int salida = dup(1);
    if (salida == -1) { perror("ERROR"); error = true; return; }
        
    int dup2out = dup2(out, 1);
    if(dup2out == -1) { perror("ERROR"); error = true; return; }
    close(out);

    printf("%s", line);

    int dup2return = dup2(salida, 1);
    if(dup2return == -1) { perror("ERROR"); error = true; return; }
    close(salida);
}

char **quitlinks(char **args) {
    int bufsize = MAX_LEN;
    int position = 0;
    bool dir = false;
    char **new_args = (char **)malloc(bufsize * sizeof(char*));
    if (!new_args) { perror("ERROR"); exit(EXIT_FAILURE); }

    for (int i = 0; args[i]; i++)
    {
        if (strcmp(args[i], "<") == 0 ||
            strcmp(args[i], ">>") == 0 ||
            strcmp(args[i], ">") == 0 ||
            strcmp(args[i], "2>>") == 0 ||
            strcmp(args[i], "2>") == 0)
        { dir = true; continue; }

        if (dir) { dir = false; continue; }
        
        new_args[position] = args[i];
        position++;

        if (position >= bufsize) 
        {
            bufsize += MAX_LEN;
            new_args = (char **)realloc(new_args, bufsize * sizeof(char*));
            if (!new_args) { perror("ERROR"); exit(EXIT_FAILURE); }
        }        
    }
    
    return new_args;
}

char **linked(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "<") == 0)
        {
            int in = open(args[i+1], O_RDONLY);
            if(in == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            int dup2in = dup2(in, 0);
            if(dup2in == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            close(in);
        }
        else if (strcmp(args[i], ">>") == 0)
        {
            int out = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if(out == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            int dup2out = dup2(out, 1);
            if(dup2out == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            close(out);
        }
        else if (strcmp(args[i], ">") == 0)
        {
            int out = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(out == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            int dup2out = dup2(out, 1);
            if(dup2out == -1) { perror("ERROR"); exit(EXIT_FAILURE); }
            close(out);                    
        }
        else if (strcmp(args[i], "2>>") == 0)
        {
            int err = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if(err == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            int dup2err = dup2(err, 2);
            if(dup2err == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            close(err); 
        }
        else if (strcmp(args[i], "2>") == 0)
        {
            int err = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(err == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            int dup2err = dup2(err, 2);
            if(dup2err == -1) { perror("ERROR"); exit(EXIT_FAILURE); }

            close(err);
        }
    }
    
    char **new_args = quitlinks(args);
    return new_args;
}

void cd(char **UDargs)
{
    char *new_root = reroot(UDargs);
    if(chdir(new_root) != 0) { perror("ERROR"); };
}

char *reroot(char **args)
{
    int bufsize = MAX_LEN;
    int position = 0;
    char *new_root = (char *)malloc(bufsize * sizeof(char));

    if (!new_root) { perror("ERROR"); exit(EXIT_FAILURE); }

    for (int i = 1; args[i] != NULL; i++)
    {
        char *p = args[i];
        int len = strlen(p);

        for (int j = 0; j < len; j++)
        {
            if (p[j] == '\\') new_root[position] = ' ';
            else new_root[position] = p[j];
            
            position++;
            if (position >= bufsize) 
            {
                bufsize += MAX_LEN;
                new_root = (char *)realloc(new_root, bufsize * sizeof(char));
                if (!new_root) { perror("ERROR"); exit(EXIT_FAILURE); }
            }
        } 

        if (new_root[position - 1] != ' ')
        {
            break;
        }              
    }
    return new_root; 
}

void work(char **args)
{
    if (strcmp(args[0], "exit") == 0 ) { exit(EXIT_FAILURE); }
    if (strcmp(args[0], "cd") == 0)
    {
        cd(args);
        exit(EXIT_FAILURE);
    }
    if (strcmp(args[0], "history") == 0)
    {
        char toexec[strlen(args[0]) + strlen(root)];
        strcpy(toexec, root);
        strcat(toexec, "/history");
        args[0] = root;
        execvp(toexec, args);
        perror("history");
        exit(EXIT_FAILURE);
    }
    if (strcmp(args[0], "help") == 0)
    {
        printf("hola");
        char toexec[strlen(args[0]) + strlen(root)];
        strcpy(toexec, root);
        strcat(toexec, "/help");
        args[0] = root;
        execvp(toexec, args);
        perror("help");
        exit(EXIT_FAILURE);
    }

    printf("mundo");
    execvp(args[0], args);
    fprintf(stderr, "ERROR: Command not found\n");
    exit(EXIT_FAILURE);
}

char **spliter(char *line, const char *separador)
{
    int bufsize = MAX_LEN;
    char **tokens = (char **)malloc(bufsize * sizeof(char*));
    char *tok;

    if (!tokens) { perror("ERROR"); exit(EXIT_FAILURE); }
    
    tok = strtok(line, separador);
    int i = 0;
    while (tok != NULL) 
    {
        if (i >= bufsize) {            
            bufsize *= 2;
            tokens = (char **)realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) { perror("ERROR"); exit(EXIT_FAILURE); }
        }
        tokens[i++] = tok;
        tok = strtok(NULL, separador);
    }
    tokens[i] = NULL;
    pipes = i;
    return tokens;
}