#include "stdio.h"
#include "stdlib.h"
#include "stdlib.h"
#include "errno.h"
#include "fs304.h"
#define BUFFSIZE 1024
#define METHOD_COUNT 8
void compare(char* fname1, char* fname2);
void rname(char* oldname, char* newname);
void copy(char* fname1, char* fname2);

char input[BUFFSIZE];
char* cmd;
char* args[2];
char *commands[8] = {"ls", "rd", "stats", "cd", "md", "compare", "rename", "copy"};
void (*funcs[8]) = {ls, rd, stats, cd, md, compare, rname, copy};
//char
typedef struct _func_t {
    char* name;
    void (*func)();
    int argcount;
} func_t;

func_t methods[8];

int main(int* argc, char** argv){
    mountFS304();

    // Initialize methods
    for(int i =0; i<METHOD_COUNT; i++){
        func_t new_func;
        new_func.name = commands[i];
        new_func.func = funcs[i];
        new_func.argcount = i<5?(i<3?0:1):2;
        methods[i] = new_func;
    }
    /*
    printf("%s\n", methods[4].name);
    printf("%d\n", methods[4].argcount);
    */
    while(1){
        printPrompt();
        fgets(input, BUFFSIZE, stdin);
        char* c = input;
        while(*c != '\n'){c++;}
        *c = '\0';
        cmd = strtok(input, " ");
        for(int i=0; i<METHOD_COUNT; i++){
            if(strcmp(cmd, methods[i].name) == 0){
                switch(methods[i].argcount){
                    case 0:
                        (methods[i].func)();
                        break;
                    case 1:
                        args[0] = strtok(NULL, " ");
                        (*methods[i].func)(args[0]);
                        break;
                    case 2:
                        args[0] = strtok(NULL, " ");
                        args[1] = strtok(NULL, " ");
                        (*methods[i].func)(args[0], args[1]);
                        break;
                }
                break;
            }
        }
        /*
        args[0] = strtok(NULL, " ");
        args[1] = strtok(NULL, " ");
        */

    }
}

void compare(char* fname1, char* fname2){

}

void rname(char* oldname, char* newname){

}

void copy(char* fname1, char* fname2){

}

