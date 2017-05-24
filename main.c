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

func_t methods[METHOD_COUNT];

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

    int block1 = 0;
    int block2 = 0;
    block1 = stoi(_inode_table[fname1_inode].XX,2);
    block2 = stoi(_inode_table[fname2_inode].XX,2);
    if(block1 != 0 && block2 != 0){
        diff += countDiff(block1, block2);
    }

    block1 = stoi(_inode_table[fname1_inode].YY,2);
    block2 = stoi(_inode_table[fname2_inode].YY,2);
    if(block1 != 0 && block2 != 0){
        diff += countDiff(block1, block2);
    }

    block1 = stoi(_inode_table[fname1_inode].ZZ,2);
    block2 = stoi(_inode_table[fname2_inode].ZZ,2);
    if(block1 != 0 && block2 != 0){
        diff += countDiff(block1, block2);
    }
    
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

    if(strlen(newname) == 0){
        printf("New name must be provided\n");
        return;
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
    char itype;
	int blocks[3];
	_directory_entry _directory_entries[4];

	int i,j;

	int empty_dblock=-1,empty_dentry=-1;
	int empty_ientry;

	// non-empty name
	if (strlen(fname2)==0) {
		printf("New name must be provided\n");
		return;
	}

    if(strcmp(fname1, fname2) == 0){
        printf("New file name must be different from the original file\n");
        return;
    }

	// do we have free inodes
	if (free_inode_entries == 0) {
		printf("Error: Inode table is full.\n");
		return;
	}

	// read inode entry for current directory
	// in FS304, an inode can point to three blocks at the most
	itype = _inode_table[CD_INODE_ENTRY].TT[0];
	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);

    int fname1_blockIdx = -1;
    int fname1_dirIdx = -1;
    int fname1_inode = -1;
    int fname1_blocks[3];

	// its a directory; so the following should never happen
	if (itype=='F') {
		printf("Fatal Error! Aborting.\n");
		exit(1);
	}

	// now lets try to see if the name already exists
	for (i=0; i<3; i++) {
		if (blocks[i]==0) { 	// 0 means pointing at nothing
			if (empty_dblock==-1) empty_dblock=i; // we can later add a block if needed
			continue;
		}

		readFS304(blocks[i],(char *)_directory_entries); // lets read a directory entry; notice the cast

		// so, we got four possible directory entries now
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') { // means unused entry
				
                if (empty_dentry==-1) { empty_dentry=j; empty_dblock=i; } // AAHA! lets keep a note of it, just in case we have to create the new directory
				continue;
			} else {
                if(strcmp(fname1, _directory_entries[j].fname) == 0){
                    fname1_blockIdx = blocks[i];
                    fname1_dirIdx = j;
                    fname1_inode = stoi(_directory_entries[j].MMM, 3);
                }
            }

			if (strncmp(fname2,_directory_entries[j].fname, 252) == 0) { // compare with user given name
					printf("%.252s: Already exists.\n",fname2);
					return;
			}
		}

	}

    if(fname1_blockIdx == -1){
        printf("Source file not found: %s\n", fname1);
        return;
    }

    fname1_blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	fname1_blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	fname1_blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);

    int needed_blocks = 0;
    for(int i=0; i<3; i++){
        if(fname1_blocks[i] != 0){
            needed_blocks++;
        }
    }

    if(free_disk_blocks < needed_blocks){
        printf("There is not enough memory to copy file\n");
        return;
    }


	// so directory name is new

	// if we did not find an empty directory entry and all three blocks are in use; then no new directory can be made
	if (empty_dentry==-1 && empty_dblock==-1) {
		printf("Error: Maximum directory entries reached.\n");
		return;
	}
	else { // otherwise
		if (empty_dentry == -1) { // Great! didn't find an empty entry but not all three blocks have been used
			empty_dentry=0;

			if ((blocks[empty_dblock] = getBlock())==-1) {  // first get a new block using the block bitmap
				printf("Error: Disk is full.\n");
				return;
			}

			writeFS304(blocks[empty_dblock],NULL);	// write all zeros to the block (there may be junk from the past!)

			switch(empty_dblock) {	// update the inode entry of current dir to reflect that we are using a new block
				case 0: itos(_inode_table[CD_INODE_ENTRY].XX,blocks[empty_dblock],2); break;
				case 1: itos(_inode_table[CD_INODE_ENTRY].YY,blocks[empty_dblock],2); break;
				case 2: itos(_inode_table[CD_INODE_ENTRY].ZZ,blocks[empty_dblock],2); break;
			}
		}


		// NOTE: all error checkings have already been done at this point!!
		// time to put everything together

		empty_ientry = getInode();	// get an empty place in the inode table which will store info about blocks for this new directory

		readFS304(blocks[empty_dblock],(char *)_directory_entries);	// read block of current directory where info on this new directory will be written
		_directory_entries[empty_dentry].F = '1';			// remember we found which directory entry is unused; well, set it to used now
		strncpy(_directory_entries[empty_dentry].fname,fname2,252);	// put the name in there
		itos(_directory_entries[empty_dentry].MMM,empty_ientry,3);	// and the index of the inode that will hold info inside this directory
		writeFS304(blocks[empty_dblock],(char *)_directory_entries);	// now write this block back to the disk

        int new_blocks[needed_blocks];
        for(int i=0; i<needed_blocks;i++){
            new_blocks[i] = getBlock();
        }
        
		strncpy(_inode_table[empty_ientry].TT,"FI",2);		// create the inode entry...first, its a directory, so DI
		strncpy(_inode_table[empty_ientry].XX,"00",2);		// directory is just created; so no blocks assigned to it yet
		strncpy(_inode_table[empty_ientry].YY,"00",2);
		strncpy(_inode_table[empty_ientry].ZZ,"00",2);

        char id[2];
        if(new_blocks[0] != 0){
            itos(id, new_blocks[0], 2);
            strncpy(_inode_table[empty_ientry].XX,id,2);		// directory is just created; so no blocks assigned to it yet
        }
        if(new_blocks[1] != 0){
            itos(id, new_blocks[1], 2);
            strncpy(_inode_table[empty_ientry].YY,id,2);
        }
        if(new_blocks[2] != 0){
            itos(id, new_blocks[2], 2);
            strncpy(_inode_table[empty_ientry].ZZ,id,2);
        }
		writeFS304(BLOCK_INODE_TABLE, (char *)_inode_table);	// phew!! write the inode table back to the disk
	
        for(int i=0; i<needed_blocks; i++){
            if((fname1_blocks[i] != 0) && (new_blocks[i] != 0)){
                char tempbuff[1024];
                readFS304(fname1_blocks[i], tempbuff);
                writeFS304(new_blocks[i], tempbuff);
            }
        }


    }
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