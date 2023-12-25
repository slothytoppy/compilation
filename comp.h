#ifndef COMPILATION_IMPLEMENTATION
int exec(char* args[]);
int len(const char* str1);
char* ext(const char* file);
char* base(const char* path);
int IS_PATH_DIR(const char* path);
int IS_PATH_FILE(const char* file);
int MKFILE(const char* file);
int RMFILE(const char* file);
int MKDIR(const char* path);
int RMDIR(const char *path);
int is_path1_modified_after_path2(Cstr source_path, Cstr binary_path);
int compile_targets(const char* files[], const char* compiler, const char* extension);
int compile_all(const char* source_file, const char* directory, char* compiler, char* flags[], const char* extension, char* target_directory);
int GO_REBUILD(char* file,char** argv);
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
#include <fcntl.h>
#include <assert.h>

typedef const char* Cstr;

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

int len(Cstr str1){
  if(str1==NULL) return -1;
  int i=0;
  while(*str1++){
    i++;
  }
  return i;
}

const char* ext(Cstr filename){
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

char* base(Cstr file){
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

int IS_PATH_DIR(Cstr path){
  struct stat fi;
  if(stat(path, &fi)<0){
    if(errno==ENOENT) 
      fprintf(stderr, "could not open %s", path);
    perror("errno");
    return ENOENT;
  } else return S_ISDIR(fi.st_mode);
}

int IS_PATH_FILE(Cstr file){
 struct stat fi;
  if(stat(file, &fi)<0){
    if(errno==ENOENT) 
      fprintf(stderr, "could not open %s", file);
    perror("errno");
    return ENOENT;
  } else return S_ISREG(fi.st_mode);
}

int MKFILE(const char* file){
  struct stat fi;
  if(stat(file, &fi)!=0){
    if(creat(file, 0644)<0){
      fprintf(stderr, "mkfile error:%s %d", file, errno);
      return -1;
    }
  }
  return 0;
}

int RMFILE(const char* file){
  struct stat fi;
  if(stat(file, &fi)!=0){
    if(unlink(file)<0){
      fprintf(stderr, "rmfile error:%s %d", file, errno);
      return -1;
    }
  }
  return 0;
}

int MKDIR(const char* path){
  struct stat fi;
  if(stat(path, &fi)!=0){
    mode_t perms = S_IRWXU | S_IRWXG | S_IRWXO;
    if(mkdir(path, perms)<0){ 
      fprintf(stderr, "mkdir error:%s %d", path, errno);
      return -1;
    }
  }
  return 0;
}

int RMDIR(const char *path){
  struct stat fi;
  if(stat(path, &fi)!=0){
    if(rmdir(path)<0){
      fprintf(stderr, "rmdir error:%s %d", path, errno);
      return -1;
    }
  }
  return 0;
}

int is_path1_modified_after_path2(Cstr source_path, Cstr binary_path){
  struct stat fi;
  if(stat(source_path, &fi)!=0){
    fprintf(stderr, "%s doesnt exist\n", source_path); 
  }
  int source_time=fi.st_mtime;
  if(stat(binary_path, &fi)!=0){
    fprintf(stderr, "%s doesnt exist\n", binary_path);
  }
  int binary_time=fi.st_mtime;
  return source_time>binary_time;
}

int compile_targets(char* files[], char* compiler, Cstr extension){
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

int compile_all(Cstr directory, char* compiler, Cstr extension, char* target_directory){
  struct stat fi;
  struct dirent *dirent;
  DIR* Dir;
  if(directory==NULL || compiler==NULL || extension==NULL || target_directory==NULL){
    fprintf(stderr, "directory, compiler, extension or executable path was null\n"); 
    return -1;
  }
  Dir=opendir(directory);
  if(Dir){
    while((dirent=readdir(Dir))!=NULL){
      if(strlen(dirent->d_name)>1 && strcmp(dirent->d_name, ".")!=0 && strlen(dirent->d_name)>2 && strcmp(dirent->d_name, "..")!=0){
	if(strcmp(ext(dirent->d_name), extension)==0){
	  if(strcmp(directory, ".")!=0){
	    if(IS_PATH_FILE(directory) && IS_PATH_FILE(target_directory)){
	    fprintf(stderr, "%s or %s isnt a directory\n", directory, target_directory);
	    }
	    char* cwd=malloc(sizeof(cwd) * PATH_MAX);;
	    char* dname=malloc(sizeof(dname) * PATH_MAX);;
	    char buff[PATH_MAX];
	    strcat(cwd, target_directory);
	    // printf("cwd:%s\n", cwd);
	    strcat(cwd, "/");
	    // printf("cwd:%s\n", cwd);
	    strcat(cwd, base(dirent->d_name));
	    // printf("cwd:%s\n", cwd);
	      // printf("name:%s cwd:%s\n", dirent->d_name, cwd);
	      strcat(dname, getcwd(buff, sizeof(buff)));
	      strcat(dname, "/");
	      strcat(dname, directory);
	      strcat(dname, "/");
	      strcat(dname, dirent->d_name);
	      char* command[]={compiler, dname, "-o", cwd, NULL};
	      // printf("dname:{%s} cwd:{%s}\n", dname, cwd);
	      exec(command);  
	      if(stat(command[1], &fi)==0 && stat(command[3], &fi)==0){
	      printf("executed:{%s} source:{%s} output:{%s}\n", command[0], command[1], command[3]); 
	      }
	      free(cwd);
	  }
	  if(strcmp(directory, ".")==0){
	    char* cwd=base(dirent->d_name);
	    char* command[]={compiler, dirent->d_name, "-o", cwd, NULL};
	      // printf("d_name:{%s} cwd:{%s}\n", dirent->d_name, cwd);
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
  }
  else if(ENOENT==errno){
    closedir(Dir);
    return -1;
  }
  closedir(Dir);
  return 0;
}

int GO_REBUILD(char* file,char** argv){
  assert(file!=NULL && argv!=NULL);
  if(is_path1_modified_after_path2(file, argv[0])){
  char* command[]={"cc", "-o", argv[0], file, NULL};
  exec(command); 
  printf("executed:%s source:%s binary:%s\n", command[0], command[3], command[2]); 
  }
  return 0;
}

#endif
