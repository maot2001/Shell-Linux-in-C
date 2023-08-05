#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char format[] = ".txt";
    char *root;
    char root2[100];

    if (argv[1] == NULL) {
        root = "help.txt";        
    }
    else {
        strcpy(root2, argv[1]);
        strcat(root2, format);
        root = root2;
    }
   
    char **argvs = (char **)malloc(2 * sizeof(char *));
    argvs[0] = "cat";
    argvs[1] = root;

    execvp(argvs[0], argvs);
    perror("ERROR");
    return 0;
}