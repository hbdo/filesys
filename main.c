#include "stdio.h"
#include "stdlib.h"
#include "stdlib.h"
#include "errno.h"
#include "fs304.h"
#define BUFFSIZE 1024
#define METHOD_COUNT 9
void compare(char* fname1, char* fname2);
void rname(char* oldname, char* newname);
void copy(char* fname1, char* fname2);
void quit();
int countDiff(int blockNum1, int blockNum2);

char input[BUFFSIZE];
char* cmd;
char* args[2];
char *commands[METHOD_COUNT] = {"ls", "rd", "stats", "exit", "cd", "md", "compare", "rename", "copy"};
void (*funcs[METHOD_COUNT]) = {ls, rd, stats, quit, cd, md, compare, rname, copy};
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
        new_func.argcount = i<6?(i<4?0:1):2;
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
    char itype;
	int blocks[3];
	_directory_entry _directory_entries[4];

	int i,j;
	int e_inode;

    int fname1_inode = -1;
    int fname2_inode = -1;

    int diff = 0;
	// read inode entry for current directory
	// in FS304, an inode can point to three blocks at the most
	itype = _inode_table[CD_INODE_ENTRY].TT[0];
	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);

	// its a directory; so the following should never happen
	if (itype=='F') {
		printf("Fatal Error! Aborting.\n");
		exit(1);
	}

	// lets traverse the directory entries in all three blocks
	for (i=0; i<3; i++) {
		if (blocks[i]==0) continue;	// 0 means pointing at nothing

		readFS304(blocks[i],(char *)_directory_entries);	// lets read a directory entry; notice the cast

		// so, we got four possible directory entries now
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') continue;	// means unused entry

			e_inode = stoi(_directory_entries[j].MMM,3);	// this is the inode that has more info about this entry

			if (_inode_table[e_inode].TT[0]=='F')  { // entry is for a file
				if(strcmp(_directory_entries[j].fname, fname1) == 0){
                    fname1_inode = e_inode;
                }
                if(strcmp(_directory_entries[j].fname, fname2) == 0){
                    fname2_inode = e_inode;
                }
			}
		}
	}


    if(fname1_inode == -1){
        printf("File name not found: %s\n", fname1);
        return;
    }

    if(fname2_inode == -1){
        printf("File name not found: %s\n", fname2);
        return;
    }

    diff += countDiff(stoi(_inode_table[fname1_inode].XX,2), stoi(_inode_table[fname2_inode].XX,2));
    diff += countDiff(stoi(_inode_table[fname1_inode].YY,2), stoi(_inode_table[fname2_inode].YY,2));
    diff += countDiff(stoi(_inode_table[fname1_inode].ZZ,2), stoi(_inode_table[fname2_inode].ZZ,2));

    printf("The files have %d differences in total\n", diff);

}

void rname(char* oldname, char* newname){
    char itype;
	int blocks[3];
	_directory_entry _directory_entries[4];
    
	int i,j;
	int e_inode;

    int oldname_exists = -1;
    int newname_exists = -1;

    int blockIdx;
    int dirIdx;
	// read inode entry for current directory
	// in FS304, an inode can point to three blocks at the most
	itype = _inode_table[CD_INODE_ENTRY].TT[0];
	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);

	// its a directory; so the following should never happen
	if (itype=='F') {
		printf("Fatal Error! Aborting.\n");
		exit(1);
	}

	// lets traverse the directory entries in all three blocks
	for (i=0; i<3; i++) {
		if (blocks[i]==0) continue;	// 0 means pointing at nothing

		readFS304(blocks[i],(char *)_directory_entries);	// lets read a directory entry; notice the cast

		// so, we got four possible directory entries now
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') continue;	// means unused entry

			e_inode = stoi(_directory_entries[j].MMM,3);	// this is the inode that has more info about this entry

            if(strcmp(_directory_entries[j].fname, oldname) == 0){
                oldname_exists = 1;
                blockIdx = i;
                dirIdx = j;
            }

            if(strcmp(_directory_entries[j].fname, newname) == 0){
                newname_exists = 1;
            }
            
		}
	}


    if(oldname_exists == -1){
        printf("File/directory name not found: %s\n", oldname);
        return;
    }

    if(newname_exists != -1){
        printf("File/directory name already exists: %s\n", newname);
        return;
    }

    readFS304(blocks[blockIdx],(char *)_directory_entries);
    memset((void*) _directory_entries[dirIdx].fname,0,sizeof(char)*252);
    strcpy(_directory_entries[dirIdx].fname, newname);
    writeFS304(blocks[blockIdx], (char *) _directory_entries);
    ls();
}

void copy(char* fname1, char* fname2){

}

void quit(){
    fflush(df);
    fclose(df);
    exit(0);
}

int countDiff(int blockNum1, int blockNum2){
    char block1[1024];
    char block2[1024];
    int diff = 0;
    readFS304(blockNum1, block1);
    readFS304(blockNum2, block2);

    for(int i=0; i<1024;i++){
        diff += block1[i] == block2[i] ? 0 : 1;
    }

    return diff;
}