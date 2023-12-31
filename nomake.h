#ifndef COMPILATION_IMPLEMENTATION

unsigned int debug_print(char* status, ...);
unsigned int debug_print_array(char* status, char** msg);
unsigned int exec(char* args[]);
unsigned int run(char* pathname)
unsigned int run_args(char* pathname[]);
unsigned int len(const char* str1);
char* ext(const char* file);
char* base(const char* path);
unsigned int IS_PATH_DIR(const char* path);
unsigned int IS_PATH_FILE(const char* path);
unsigned int IS_PATH_EXIST(const char* path);
unsigned int MKFILE(const char* file);
unsigned int RMFILE(const char* file);
unsigned int MKDIR(const char* path);
unsigned int RMDIR(const char *path);
unsigned int is_path1_modified_after_path2(const char* source_path, const char* binary_path);
unsigned int print_exec(char* args[]);
unsigned int compile_file(char* file, char* destination, char* compiler, const char* extension);
unsigned int compile_targets(char* files[], char* destination, char* compiler, const char* extension);
unsigned int compile_dir(char* origin, char* destination, char* compiler, const char* extension);

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
#include <stdarg.h>
#include <pthread.h>

#ifdef DEBUG
#define debug(status, ...) debug_print(status, __VA_ARGS__);
#define print_files(file1, file2) printf("%s && %s\n", file1, file2)
#define print_source() printf("[SOURCE]", __FILE__)
#else 
// is neccesary so that if DEBUG isnt defined it does nothing
#define debug(status, ...)
#define print_files(file1, file2)
#define print_source(){            \
  char* file=__FILE__;             \
  printf("[SOURCE] %s\n", file);   \
}
#endif // DEBUG
typedef const char* Cstr;

unsigned int debug_print(char* status, ...){
  if(!status) return 0;
	va_list args;
  va_start(args, status);
  printf("[%s] ", status);
  vprintf("%s " , args);
  va_end(args);
  printf("\n");
  return 1;
}

unsigned int exec(char* args[]){
  if(!args) return 0;
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

unsigned int run_args(char* pathname[]){
  if(!pathname) return 0;
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
  return 1;
}

unsigned int run(char* pathname){
if(!pathname) return 0;
char* command[]={pathname, NULL};
if(run_args(command)){
return 1;
}
return 0;
}

unsigned int len(Cstr str1){
  if(str1==NULL) return 0;
  unsigned int i=0;
  while(*str1++){
    i++;
  }
  return i;
}

unsigned int ends_with(char* str1, char with){
if(str1==NULL || with==0) return 0;
unsigned int sz=len(str1)-1; // index starts at zero so i have to get rid of 1 so it doesnt access the \0
debug("NEEDLE IS", str1[sz]);
if(str1[sz]==with){
return 1;
}
return 0;
}

const char* ext(Cstr filename){
if(!filename) return NULL;
unsigned int i;
unsigned int sz=len(filename);
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

unsigned int IS_PATH_DIR(char* path){
  if(!path) return 0;
	struct stat fi;
  if(stat(path, &fi)<0){
    if(errno==ENOENT) 
      fprintf(stderr, "could not open %s\n", path);
    perror("errno");
    return 0;
  }
  if(!S_ISDIR(fi.st_mode)){
  return 0;
  }
  debug("IS DIR", path); 
  return 1;
}

unsigned int IS_PATH_FILE(char* path){
  if(!path) return 0;
	struct stat fi;
  if(stat(path, &fi)<0){
    if(errno==ENOENT){
      fprintf(stderr, "%s doesnt exist\n", path);
    }
    return 0;
  } 
  if(!S_ISREG(fi.st_mode)){
  return 0;
  }
  debug("IS FILE", path); 
  return 1;
}

unsigned int IS_PATH_EXIST(char* path){
  if(!path) return 0;
  struct stat fi;
  if(stat(path, &fi)==-1){
		debug("DOESNT EXIST", path);
    return 0;
  } 
  debug("EXISTS", path);
  return 1;
}

unsigned int MKFILE(char* file){
  if(!file) return 0;
  struct stat fi;
  if(stat(file, &fi)!=0){
    if(creat(file, 0644)<0){
      fprintf(stderr, "mkfile error:%s %d\n", file, errno);
      return 0;
    }
  }
  if(IS_PATH_EXIST(file)){
  debug("CREATED", file);
  }
  return 1;
}

unsigned int RMFILE(char* file){
  if(!file) return 0;
  struct stat fi;
  if(stat(file, &fi)==0){
    if(unlink(file)<0){
      fprintf(stderr, "rmfile error:%s %d\n", file, errno);
      return 0;
    }
  }
  if(!IS_PATH_EXIST(file)){
  debug("REMOVED", file);
  }
  return 1;
}

unsigned int CLEAN(char* directory, char* extension){
if(directory==NULL || extension==NULL){
    fprintf(stderr, "directory or compiler was null\n");
    return 0;
  }
  struct dirent *dirent;
  DIR* source_dir;
  if(strcmp(directory, ".")==0){
  char buff[PATH_MAX];
  char* cwd=getcwd(buff, sizeof(buff));
  source_dir=opendir(cwd);
  } else{
  source_dir=opendir(directory);
  }
  if(source_dir){
    while((dirent=readdir(source_dir))!=NULL){
      if(strcmp(dirent->d_name, ".")!=0 && strcmp(dirent->d_name, "..")!=0){
	if(strcmp(ext(dirent->d_name), extension)==0){
	RMFILE(base(dirent->d_name));	
  	}
      }
    }
  }
  return 1;
}

unsigned int MKDIR(char* path){
  if(!path) return 0;
  struct stat fi;
  if(stat(path, &fi)!=0){
    mode_t perms = S_IRWXU | S_IRWXG | S_IRWXO;
    if(mkdir(path, perms)<0){ 
      fprintf(stderr, "mkdir error:%s %d\n", path, errno);
      return 0;
    }
  }
  if(IS_PATH_EXIST(path)){
  return 1;
	}
  debug("CREATED DIR", path);
  return 1;
}

unsigned int RMDIR(char *path){
	if(!path) return 0; 
	struct stat fi;
  if(stat(path, &fi)==0 && IS_PATH_DIR(path)){
    if(rmdir(path)<0){
      fprintf(stderr, "rmdir error:%s %d\n", path, errno);
      return 0;
    }
  }
  debug("REMOVE DIR", path);
  return 1;
}

unsigned int is_path1_modified_after_path2(Cstr source_path, Cstr binary_path){
  if(!source_path || !binary_path) return 0;
	struct stat fi;
  if(stat(source_path, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", source_path); 
  }
  unsigned int source_time=fi.st_mtime;
  if(stat(binary_path, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", binary_path);
  }
  unsigned int binary_time=fi.st_mtime;
  return source_time>binary_time;
}

unsigned int print_exec(char* args[]){
  printf("executed:%s source:%s binary:%s\n", args[0], args[1], args[3]);
  return 1;
}

unsigned int compile_file(char* file, char* destination, char* compiler, const char* extension){
	if(file==NULL || destination==NULL || compiler==NULL || extension==NULL){
    fprintf(stderr, "origin, destination, compiler or extension was null\n");
    return 0;
	}
if(strcmp(ext(file), extension)==0){
char* command[]={compiler, file, "-o", destination, NULL};
exec(command);
print_exec(command);
	if(IS_PATH_EXIST(destination)){
debug_print("COMPILED", file);
	} else{
debug_print("COULDNT COMPILE", file);
	}
}
return 1;
}

unsigned int compile_targets(unsigned int sz, char* files[], char* destination, char* compiler, Cstr extension){
if(files==NULL || destination==NULL || compiler==NULL || extension==NULL){
    fprintf(stderr, "file, destination, compiler or extension was null\n");
    return 0;
  }
  if(strcmp(*files, ".")==0){
  fprintf(stderr, "file can not be only a dot\n");
  return 0;
  }
  unsigned int i;
  unsigned int j;
  struct stat fi;
  printf("DEBUG ELEMS:%d\n", sz); 
  for(i=0; i<sz; i++){
    if(strcmp(destination, ".")==0){
      if(strcmp(ext(files[i]), extension)==0){
  char* command[]={compiler, files[i], "-o", base(files[i]), NULL};
  printf("FILE:%s BASE:%s\n", command[1], command[3]);
      if(exec(command)){
      print_exec(command);
			}
    } 
    }
		if(strcmp(destination, ".")!=0){
			if(strcmp(ext(files[i]), extension)==0){
  char* twd=calloc(1, PATH_MAX);
  strcat(twd, destination); 
  strcat(twd, "/");
  strcat(twd, base(files[i]));
  debug("TWD", twd);
  char* command[]={compiler, files[i], "-o", twd, NULL};
			if(exec(command)){
			print_exec(command);
			}
			}
	  }
  }
  if(stat(base(*files), &fi)!=0){
  fprintf(stderr, "%s wasnt compiled correctly\n", base(*files));
  return 0;
  }
  return 1;
}

unsigned int compile_dir(char* origin, char* destination, char* compiler, Cstr extension){
  if(origin==NULL || destination==NULL || compiler==NULL || extension==NULL){
    fprintf(stderr, "origin, destination, compiler or extension was null\n");
    return 0;
  }
  unsigned int i=0;
  struct dirent *dirent;
  DIR* source_dir;
  source_dir=opendir(origin);
  if(source_dir){
    while((dirent=readdir(source_dir))!=NULL){
      if(strcmp(dirent->d_name, ".")!=0 && strcmp(dirent->d_name, "..")!=0){
	if(strcmp(ext(dirent->d_name), extension)==0){
	i++;
	  char* dest_path=calloc(1, PATH_MAX);
	  char* origin_path=calloc(1, PATH_MAX);
	  if(strcmp(origin, destination)==0){
	    char* command[]={compiler, "-o", base(dirent->d_name), dirent->d_name, NULL};
	    exec(command);
	    debug("COMPILED", command[3]);
	    debug("BINARY", command[2]);
	    debug("finished compiling", command[3], command[2]);
	  }
	  else{
	    if(strcmp(origin, ".")==0){
	    strcat(origin_path, dirent->d_name);	
	    debug("ORIGIN:dot", origin_path);
	    } else{
	    if(strcmp(origin, ".")!=0){
	    strcat(origin_path, origin);
	    if(!ends_with(origin_path, '/')){
	    strcat(origin_path, "/");
	    }
	    strcat(origin_path, dirent->d_name);
	    debug("ORIGIN:path", origin_path);
	    }
	    }
	    if(strcmp(destination, ".")==0){
	    /* 
	    strcat(dest_path, origin);
	    strcat(dest_path, "/");
	    */
	    strcat(dest_path, base(dirent->d_name));
	    debug("DEST:path", dest_path);
	    }
	    if(strcmp(destination, ".")!=0){
	    strcat(dest_path, destination);
	    if(!ends_with(dest_path, '/')){
	    strcat(dest_path, "/");
	    }
	    strcat(dest_path, base(dirent->d_name));
	    debug("DEST:path", dest_path);
	    }
	    char* command[]={compiler, "-o", dest_path, origin_path, NULL};
	    debug("COMMAND", command[0]);
	    debug("BINARY", command[2]);
	    debug("SOURCE", command[3]);
	    printf("[source]:%s [binary]:%s\n", origin_path, dest_path);
	  }
	}
      }
    }
  }
	return 1;
}

int renameold(char* file){
char* old=calloc(1, PATH_MAX);
strcat(old, file);
strcat(old, ".old");
rename(file, old);
debug("RENAMED TO", old);
return 1;
}

#define GO_REBUILD(argc, argv, compiler){																							\
	char* file=__FILE__;																																\
	printf("FILE %s\n", file);																													\
  assert(file!=NULL && argc>=0);																											\
  if(is_path1_modified_after_path2(file, argv[0])){																		\
	  renameold(argv[0]);																																\
    char* command[]={compiler, "-o", base(file), file, NULL};													\
    if(exec(command)){                                                                \
    debug("COMPILING", command[3]);                                                   \
    debug("COMPILED", command[2]);                                                    \
    debug("RUNNING", argv[0]);                                                        \
		run(base(file));																																	\
		exit(0);																																					\
		}																																									\
	}																																										\
}																																											

#endif // COMPILATION_IMPLEMENTATION
