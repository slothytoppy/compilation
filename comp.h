#ifndef COMPILATION_IMPLEMENTATION
int exec(char* args[]);
int len(char* str1);
char* ext(char* file);
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
#include <errno.h>

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
  int sz=len(filename);
  if(sz<0) return NULL;
  for(i=0; i<sz; i++){
    if(filename[i]=='.'){ 
      char* ext=strdup(filename);
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

/* TODO: implement recursion for compile all, maybe i should check if the current d_type is a directory then open that, then run compile_all from within that directory
 to compile all files in that directory that match extension, but then i would have to have a "root" directory which would be "directory" for the compile_all function. */

int IS_FILE_DIR(const char* path){
struct stat fi;
if(stat(path, &fi)<0){
  if(errno==ENOENT) 
  fprintf(stderr, "could not open %s %s\n", path, strerror(errno));
  return ENOENT;
} else return S_ISDIR(fi.st_mode);
}

int compile_targets(char* files[],char* compiler, char* extension){
if(files==NULL || compiler==NULL || extension==NULL){
  fprintf(stderr, "list of files, compiler or extension was null\n");
  return -1;
}
struct stat fi;
int i;
int sz=sizeof(*files);
for(i=0; i<sz; i++){
if(stat(files[i], &fi)==0 && strcmp(ext(files[i]), extension)==0){
  printf("size:%d file[i]:{%s}\n", sz, files[i]);
  char* command[]={compiler, files[i], "-o", base(files[i]), NULL};
  exec(command);
}
}
return 0;
}

int compile_all(char* directory, char* compiler, char* extension){
  struct stat fi;
  struct dirent *dirent;
  DIR* Dir;
  if(directory==NULL || compiler==NULL || extension==NULL){
    fprintf(stderr, "directory, compiler, extension or executable path was null\n"); 
    return -1;
  }
  Dir=opendir(directory);
  if(Dir){
    while((dirent=readdir(Dir))!=NULL){
      if(strlen(dirent->d_name)>1 && strcmp(dirent->d_name, ".")!=0 && strlen(dirent->d_name)>2 && strcmp(dirent->d_name, "..")!=0){
	if(strcmp(ext(dirent->d_name), extension)==0){
	  char* command[]={compiler, dirent->d_name, "-o", base(dirent->d_name), NULL};
	  exec(command);
	  if(stat(command[1], &fi)==0 && stat(command[3], &fi)==0){
	  printf("executed:{%s} source:{%s} output:{%s}\n", command[0], command[1], command[3]);
	  } else{
	    printf("file:{%s} or {%s} doesnt exist\n", command[1], command[3]);
	}
	}
      }
    }
  }
  else if(ENOENT==errno){
  closedir(Dir);
  return -1;
  }
  closedir(Dir);
  return 0;
}

/* TODO add optional recursion to compile_all */

#endif
