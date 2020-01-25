#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include "math.h"
#include <unistd.h>

#define DIR_NUM 555
#define FILE_NUM 10000

#define DEBUG 0 /*1 for information prints, 0 otherwise*/

char* dir_paths[DIR_NUM];
char* file_paths[FILE_NUM];
char* file_names[FILE_NUM];

char* curr_path;
int dir_created=1;

int FAILED = 0;

/*Generate a random string with length size*/
char *get_rand_string(char *str, size_t size)
{
    size_t n;
    int key;
    const char charset[] = "wxyzWXYZ1234";
    if (size) {
        for (n = 0; n < size; n++) {
            key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

/*Add a "layer" of dir_num directories to curr_path*/
void add_dir_layer(int dir_num){
    int i,length;
    char* dir_name, *new_path;
    struct stat st={0};

    length = rand()%(5)+3; /*Directory name length between 3 and 7 chars*/
    if(DEBUG){printf("Directory name length is %d\n",length);}
    if(DEBUG){printf("Current path is %s, with length %ld\n",curr_path,strlen(curr_path));}

    for (i=0;i<dir_num;i++){
        /*Allocate and generate directory name*/
        dir_name=(char*)malloc(length+1);
        get_rand_string(dir_name,(size_t)length);
        if(DEBUG){printf("Directory name is %s\n",dir_name);}

        /*Allocate and set new directory path*/
        new_path = (char*)malloc(length+ strlen(curr_path)+2);
        new_path[0]='\0';
        strcat(strcat(strcat(new_path,curr_path),"/"), dir_name);
        if(DEBUG){printf("New directory path is %s, with length %ld\n",new_path,strlen(new_path));}

        /*Checks if the directory doesnt exist already*/
        if(stat(new_path,&st)==-1){
            /*Allocate and set new directory path in dir_paths array*/
            dir_paths[dir_created+i]=(char*)malloc(strlen(new_path)+1);
            strcpy(dir_paths[dir_created+i],new_path);
            if(DEBUG){printf("Directory path %s was added to the array in index %d\n",dir_paths[dir_created+i],dir_created+i);}

            /*Create a new directory*/
            if(mkdir(new_path,0777)){
                printf("ERROR: Could not create new directory %s\n",strerror(errno));
                exit(1);
            }
            if(DEBUG){printf("Directory path %s was added to curr_path",dir_paths[dir_created+i]);}
        }
        else{ i--; } /*If directory already exist - redo to loop*/

        /*Freeing allocated memory*/
        if(new_path){free(new_path);}
        if(dir_name){free(dir_name);}
    }
}

/*Set correct variables and calls add_dir_layer*/
void layer_step(int step_num){
    int i=0,start=0,lim=0,dir_num=0;
    switch(step_num) {
        case 1: start=0;lim=1;dir_num=3;break;
        case 2: start=1;lim=4;dir_num=4;break;
        case 3: start=4;lim=16;dir_num=5;break;
        case 4: start=16;lim=76;dir_num=2;break;
        case 5: start=76;lim=196;dir_num=3;break;
    }
    if(DEBUG){printf("We are now adding the layer number %d\n",step_num);}

    for(i=start;i<lim;i++){
        if(DEBUG){printf("We are now adding directory number %d in layer number %d\n",i,step_num);}
        /*Allocate and set current path value*/
        curr_path=(char*)malloc(strlen(dir_paths[i])+1);
        strcpy(curr_path, dir_paths[i]);

        /*Calls add_dir_layer and update number of directories created*/
        add_dir_layer(dir_num);
        dir_created+=dir_num;
        if(DEBUG){printf("%d new directories were created by now\n",dir_created);}

        /*Freeing allocated memory*/
        if(curr_path){free(curr_path);}
    }
}

/*Set root path '.' and calls layer_step for all 5 layers*/
void create_random_dirs(){
    int i;
    dir_paths[0]=(char*)malloc(2);
    strcpy(dir_paths[0],".");
    for(i=1;i<6;i++){
        layer_step(i);
    }
}

/*Go over all directories created and removes them*/
void delete_dirs(){
    int i;
    for (i=DIR_NUM;i>0;i--){
        if(rmdir(dir_paths[i])){
            printf("ERROR: Could not remove directory path %s. %s\n",dir_paths[i],strerror(errno));
            exit(1);
        }
        free(dir_paths[i]);
    }
    curr_path = NULL;
    dir_created=1;
}

/*Checks if the file already exist in the path given*/
int file_exist(int lim,char* path){
    int i;

    if(DEBUG){printf("Checking if file %s already exist.\n",path);}
    for(i=0;i<lim;i++){
        if (strcmp(path, file_paths[i]) == 0) {
            if (DEBUG) { printf("File %s does already exist\n", path); }
            return 1;
        }
    }
    return 0;
}

/*Creates FILE_NUM random files in DIR_NUM random directories*/
void create_random_files(){
    int i,length,rand_dir;
    char* file_name, *new_path;
    FILE *fp;
    struct stat file_stat;

    for(i=0;i<FILE_NUM;i++){
        /*Chooses a random directory from dir_path array*/
        rand_dir=rand()%(DIR_NUM);
        if(DEBUG){printf("I chose directory number %d\n",rand_dir);}

        length = rand()%(5)+3; /*File name length between 3 and 7 chars*/
        if(DEBUG){printf("File name length is %d\n",length);}

        /*Allocate and generate file name*/
        file_name=(char*)malloc(length+1);
        get_rand_string(file_name,(size_t)length);
        if(DEBUG){printf("File name is %s\n",file_name);}

        /*Allocate and set new file path*/
        new_path=(char*)malloc(strlen(dir_paths[rand_dir])+length+2);
        if(!new_path){
            if(DEBUG){printf("Malloc failed\n");}
            exit(1);
        }
        new_path[0]='\0';
        strcat(strcat(strcat(new_path,dir_paths[rand_dir]),"/"), file_name);
        if(DEBUG){printf("File new path is %s\n",new_path);}

        /*Checks if the file path doesnt exist already (as a file or as a directory) */
        stat(new_path,&file_stat);
        if((!file_exist(i-1,new_path)) && (!S_ISDIR(file_stat.st_mode))) {
            /*Create a new file at new_path*/
            fp = fopen(new_path, "w");
            if (!fp) {
                printf("ERROR: Could not open new file path %s. %s\n", new_path,strerror(errno));
                exit(1);
            }
            fclose(fp);

            /*Update new file i file_names and file_paths arrays*/
            file_names[i] = (char *) malloc(strlen(file_name) + 1);
            strcpy(file_names[i], file_name);
            file_paths[i] = (char *) malloc(strlen(new_path) + 1);
            strcpy(file_paths[i], new_path);
        }
        else{ i--; } /*If the path already exists - redo to loop*/

        /*Freeing allocated memory*/
        free(file_name);
        free(new_path);
    }
}

/*Go over all files created and removes them*/
void delete_files(){
    int i;
    remove("output");
    for(i=0;i<FILE_NUM;i++){
        if(remove(file_paths[i])){
            printf("ERROR: Could not remove file path %s. %s\n",file_paths[i],strerror(errno));
            exit(1);
        }
        free(file_paths[i]);
    }
}

/*Generate a random searching term T and compare program output to paths created*/
void test_prog(){
    int j,length,rc,test_found=0,prog_found=0;
    char* T, *cmd, *grep_out;
    char path[1000];
    FILE* fg_out, *fp_out, *out_buff;

    length = rand()%(3)+1; /*Searching term length between 1 and 3 chars*/
    if(DEBUG){printf("Searching term length is %d\n",length);}

    /*Allocate and generate searching term*/
    T=(char*)malloc(length+1);
    get_rand_string(T,(size_t)length);
    if(DEBUG){printf("Searching term is %s\n",T);}

    /*Allocate and set callig command to pfind.out*/
    cmd=(char*)malloc(strlen("./pfind.out . 50 ")+strlen(T)+1);
    cmd[0]='\0';
    strcat(strcat(strcat(cmd,"./pfind.out . "),T)," 50");
    if(DEBUG){printf("Command is %s\n",cmd);}
    out_buff = popen(cmd, "r");
    /*Open a file for program output names 'output'*/
    fp_out = fopen("output", "w");
    if (!fp_out) {
        printf("ERROR: Could not open file. %s\n",strerror(errno));
        exit(1);
    }

    /*Read the program output and write it to 'output' file*/
    while (fgets(path, sizeof(path), out_buff) != NULL) {
        fprintf(fp_out,"%s", path);
    }

    /*Close the program output file and free cmd*/
    pclose(out_buff);
    fclose(fp_out);
    free(cmd);

    /*Opens 'output' file for reading*/
    fp_out=fopen("output","r");

    for (j=0;j<FILE_NUM;j++){
        /*Checks if the generated searching term is in the file name*/
        if(strstr(file_names[j],T)!=NULL){
            /*Allocate and set callig command to grep on file output*/
            cmd=(char*)malloc(strlen("grep -w output ")+strlen(file_paths[j])+1);
            cmd[0]='\0';
            strcat(strcat(strcat(cmd,"grep -w "),file_paths[j])," output");
            if(DEBUG){printf("Command is %s\n",cmd);}

            /*Open a file for grep output*/
            fg_out = popen(cmd, "r");
            if (!fg_out) {
                printf("ERROR: Could not open file. %s\n",strerror(errno));
                exit(1);
            }

            /*Allocate and set output of grep function*/
            grep_out=(char*)malloc(strlen(file_paths[j])+1);
            rc=fscanf(fg_out,"%s",grep_out);

            /*Close the file for grep output*/
            pclose(fg_out);

            /*Checks if fscanf was successful and grep output match path*/
            if((rc>0) && (!strcmp(grep_out,file_paths[j]))){
                prog_found++;
            }
            else{ /*ERROR! Program didnt find a file path*/
                if(DEBUG){printf("Program didnt find file path %s\n",file_paths[j]);}
                FAILED=1;
            }
            test_found++;
            if(cmd){free(cmd);}
        }
    }

    fclose(fp_out);

    if(prog_found==test_found){
        printf("PASS: Searching term was %s.\nTest and Program both found the same %d file paths\n",T,test_found);
    }
    else{
        printf("FAILED: Searching term was %s.\n Test found %d file paths.\n Program found %d file paths",T,test_found,prog_found);
    }

    if(T){free(T);}
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int i;
    create_random_dirs();
    create_random_files();
    for(i=0;i<10;i++){
        test_prog();
    }
    delete_files();
    delete_dirs();
    if(FAILED){
        printf("YOU FAILED\n");
    }
    else{
        printf("YOU PASSED\n");
    }

    return 0;
}