#ifndef COMPILATION_IMPLEMENTATION
int exec(char* args[]);
int len(char* str1);
char* fext(char* file);
char* base(char* path);
int compile_all(DIR* dir);
#endif

#ifdef COMPILATION_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include <sys/wait.h>
#include <sys/stat.h>

int exec(char* args[]){
  pid_t id=fork();
  int child_status;
  if(id==0){
    execvp(args[0], args);
  } 
  if(id<0){
    printf("forking failed");
    return -1;
  }
  wait(&child_status);
  return 0;
}

int len(char* str1){
if(str1==NULL) return -1;
int i=0;
  while(*str1++){
  i++;
  }
  return i;
}

char* ext(char* filename){
int i;
char* ext={0};
int sz=len(filename);
if(sz<0) return NULL;
  for(i=0; i<sz; i++){
    if(filename[i]=='.'){ 
    ext=strdup(filename);
    ext=strrchr(filename, '.');
    return ext;
    }
  } 
  return filename;
}

char* base(char* file){
  if (file==NULL) return NULL;
  char *retStr;
  char *lastExt;
  if ((retStr = malloc (strlen (file) + 1)) == NULL) return NULL;
  strcpy (retStr, file);
  lastExt = strrchr (retStr, '.');
  if (lastExt != NULL)
    *lastExt = '\0';
  return retStr;
}
/*int compile_all(DIR* dir){
  struct dirent dirent;
  while(dir){
  if()
  }
  } */
#endif
