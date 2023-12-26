#ifndef COMPILATION_IMPLEMENTATION
int exec(char* args[]);
int run(char* pathname)
int run_args(char* pathname[]);
int len(const char* str1);
char* ext(const char* file);
char* base(const char* path);
int IS_PATH_DIR(const char* path);
int IS_PATH_FILE(const char* path);
int IS_PATH_EXIST(const char* path);
int MKFILE(const char* file);
int RMFILE(const char* file);
int MKDIR(const char* path);
int RMDIR(const char *path);
int is_path1_modified_after_path2(Cstr source_path, Cstr binary_path);
int print_exec(char* args[]);
int debug_print(char* info, char* msg);
int compile_targets(const char* files[], const char* compiler, const char* extension);
int compile_dir(const char* source_file, const char* directory, char* compiler, char* flags[], const char* extension, char* target_directory);
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

#ifdef DEBUG
#define debug(status, msg) debug_print(status, msg)
#else 
#define debug(status, msg) // is neccesary so that if DEBUG isnt defined it does nothing
#endif // DEBUG

typedef const char* Cstr;

int exec(char* args[]){
  pid_t id=fork();
  int child_status;
  if(id==0){
    execvp(args[0], args);
  } 
  if(id<0){
    printf("forking failed\n");
    return 0;
  }
  wait(&child_status);
  return 1;
}

int run(char* pathname){
  char* command[]={pathname, NULL};
  pid_t id=fork();
  int child_status;
  if(id==0) execv(command[0], command);
  if(id<0){
    printf("forking failed\n");
    return 0;
  }
  wait(&child_status);
  return 1;
}

int run_args(char* pathname[]){
  pid_t id=fork();
  int child_status;
  if(id==0){
    execv(pathname[0], pathname);
  } 
  if(id<0){
    printf("forking failed\n");
    return 0;
  }
  wait(&child_status);
  printf("executed:%s\n", pathname[0]);
  return 1;
}

int len(Cstr str1){
  if(str1==NULL) return 0;
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
      fprintf(stderr, "could not open %s\n", path);
    perror("errno");
    return 0;
  }
  return 1;
}

int IS_PATH_FILE(Cstr path){
  struct stat fi;
  if(stat(path, &fi)<0){
    if(errno==ENOENT){
      fprintf(stderr, "%s doesnt exist\n", path);
    }
    return 0;
  } 
  return 1;
}

int IS_PATH_EXIST(Cstr path){
  struct stat fi;
  if(stat(path, &fi)<0){
    if(errno==ENOENT){
      fprintf(stderr, "%s doesnt exist\n", path);
    }  
    return 0;
  } 
  return 1;
}

int MKFILE(const char* file){
  struct stat fi;
  if(stat(file, &fi)!=0){
    if(creat(file, 0644)<0){
      fprintf(stderr, "mkfile error:%s %d\n", file, errno);
      return 0;
    }
  }
  return 1;
}

int RMFILE(const char* file){
  struct stat fi;
  if(stat(file, &fi)!=0){
    if(unlink(file)<0){
      fprintf(stderr, "rmfile error:%s %d\n", file, errno);
      return 0;
    }
  }
  return 1;
}

int MKDIR(const char* path){
  struct stat fi;
  if(stat(path, &fi)!=0){
    mode_t perms = S_IRWXU | S_IRWXG | S_IRWXO;
    if(mkdir(path, perms)<0){ 
      fprintf(stderr, "mkdir error:%s %d\n", path, errno);
      return 0;
    }
  }
  return 1;
}

int RMDIR(const char *path){
  struct stat fi;
  if(stat(path, &fi)!=0){
    if(rmdir(path)<0){
      fprintf(stderr, "rmdir error:%s %d\n", path, errno);
      return 0;
    }
  }
  return 1;
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

int print_exec(char* args[]){
printf("executed:%s source:%s binary:%s\n", args[0], args[1], args[3]);
return 1;
}

int debug_print(char* status, char* msg){ 
  printf("[%s] %s\n", status, msg); 
  return 1;
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
  return 1;
}

int compile_dir(char* origin, char* destination, char* compiler, Cstr extension){
  if(origin==NULL || destination==NULL || compiler==NULL || ext==NULL){
    fprintf(stderr, "origin, destination, compiler, or extension was null\n");
  }
  struct dirent *dirent;
  DIR* source_dir;
  DIR* binary_dir;
  source_dir=opendir(origin);
  if(source_dir){
    while((dirent=readdir(source_dir))!=NULL){
      if(strcmp(dirent->d_name, ".")!=0 && strcmp(dirent->d_name, "..")!=0){
	if(strcmp(ext(dirent->d_name), extension)==0){
	  char* dest_path=calloc(1, PATH_MAX);
	  char* origin_path=calloc(1, PATH_MAX);
	  if(strcmp(origin, ".")==0 && strcmp(destination, ".")==0){
	  char* command[]={compiler, dirent->d_name, "-o", base(dirent->d_name), NULL};
	  print_exec(command);
	  debug("INFO", dirent->d_name);
	  exec(command);
	  }
	  if(strcmp(origin, ".")==0 && strcmp(destination, ".")!=0){
	  strcat(origin_path, destination);
	  strcat(dest_path, destination);
	  strcat(dest_path, "/");
	  strcat(dest_path, base(dirent->d_name));
	  debug("INFO", dest_path);
	  char* command[]={compiler, dirent->d_name, "-o", dest_path, NULL};
	  print_exec(command);
	  exec(command);
	  }
	  if(strcmp(origin, ".")!=0 && strcmp(destination, ".")==0){
	  strcat(origin_path, origin);
	  strcat(dest_path, origin);
	  strcat(dest_path, "/");
	  strcat(dest_path, base(dirent->d_name));
	  debug("INFO", dest_path);
	  strcat(origin_path, "/");
	  debug("INFO", dest_path);
	  strcat(origin_path, dirent->d_name);
	  debug("INFO", dest_path);
	  char* command[]={compiler, origin_path, "-o", dest_path, NULL};
	  print_exec(command);
	  exec(command);
	  }
	  if(strcmp(origin, ".")!=0 && strcmp(destination, ".")!=0){
	  strcat(dest_path, destination);
	  debug("INFO", dest_path);
	  strcat(dest_path, "/");
	  debug("INFO", dest_path);
	  strcat(origin_path, origin);
	  strcat(origin_path, "/");
	  debug("INFO", dest_path);
	  strcat(dest_path, base(dirent->d_name));
	  strcat(origin_path, dirent->d_name);
	  debug("INFO", dest_path);
	  char* command[]={compiler, origin_path, "-o", dest_path, NULL};
	  print_exec(command);
	  exec(command);
	  }
	}
      }
    }
  }
  return 1;
}

#define GO_REBUILD(argc, argv){    						      \
  char* file=__FILE__;      						      	      \
  printf("file:{%s}\n", file);     						      \
  assert(file!=NULL && argv!=NULL); 						      \
  if(is_path1_modified_after_path2(file, argv[0])){				      \
    char* command[]={"cc", "-o", argv[0], file, NULL};				      \
    exec(command); 								      \
    print_exec(command);                                                              \
  }										      \
}

#endif // COMPILATION_IMPLEMENTATION
