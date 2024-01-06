#ifndef COMPILATION_IMPLEMENTATION
void proto_print(char* status, char* fmt, va_list args);
void INFO(char* fmt, ...);
void debug_print(char* fmt, ...);
char* gcwd(char* str1);
char* upcwd(char* str1, char* str2){
unsigned int exec(char* args[]);
unsigned int run(char* pathname);
unsigned int len(Cstr str1);
unsigned int ends_with(char* str1, char with);
const char* ext(Cstr filename);
char* base(Cstr file);
unsigned int IS_PATH_DIR(char* path);
unsigned int IS_PATH_FILE(char* path);
unsigned int IS_PATH_EXIST(char* path);
unsigned int MKFILE(char* file);
unsigned int RMFILE(char* file);
unsigned int CLEAN(char* directory, char* extension);
unsigned int MKDIR(char* path);
unsigned int RMDIR(char *path);
unsigned int is_path1_modified_after_path2(Cstr source_path, Cstr binary_path);
unsigned int print_exec(char* args[]);
unsigned int compile_file(char* file, char* destination, char* compiler, const char* extension);
unsigned int compile_targets(unsigned int sz, char* files[], char* destination, char* compiler, Cstr extension);
unsigned int compile_dir(char* origin, char* destination, char* compiler, Cstr extension);
int renameold(char* file);
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
// #include <assert.h>
#include <stdarg.h>
// #include <pthread.h>

// is neccesary so that if DEBUG isnt defined it does nothing
#define print_source(){            \
  char* file=__FILE__;             \
  printf("[SOURCE] %s\n", file);   \
}
#define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
typedef const char* Cstr;

void proto_print(char* status, char* fmt, va_list args){
fprintf(stdout, "[%s] ", status);
vfprintf(stdout, fmt, args);
fprintf(stdout, "\n");
}

void INFO(char* fmt, ...) PRINTF_FORMAT(1, 2); 

void INFO(char* fmt, ...){
		va_list args;
		va_start(args, fmt);
		proto_print("INFO", fmt, args); 
		va_end(args);
}

void WARN(char* fmt, ...) PRINTF_FORMAT(1, 2);

void WARN(char* fmt, ...){
	va_list args;
	va_start(args, fmt);
	proto_print("WARNING", fmt, args);
	va_end(args);
}

void debug_print(char* fmt, ...) PRINTF_FORMAT(1,2); 

void debug_print(char* fmt, ...){
va_list args;
va_start(args, fmt);
proto_print("DEBUG", fmt, args);
}

char* gcwd(void){
char buff[PATH_MAX];
return getcwd(buff, sizeof(buff));
}

char* upcwd(char* str1, char* str2){
if(str2==NULL) str2=calloc(1, PATH_MAX);
if(str1==NULL) return 0;
char* cwd=gcwd();
if(strncmp(str1, "../", 3)==0){
char* cbuff=calloc(1, strlen(str1));
strcat(cbuff, str1);
char* buff=calloc(1, PATH_MAX);
char* sbuff=strrchr(cwd, '/');
strncpy(buff, str1+3, strlen(cwd)-strlen(sbuff));
strncpy(str2, cwd, strlen(cwd)-strlen(sbuff));
strcat(str2, "/");
strcat(str2, buff);
free(cbuff);
free(buff);
INFO("%s", str2);
return str2;
}
strcat(cwd, "/");
strcat(cwd, str1);
INFO("%s", str1);
return str1;
}

unsigned int exec(char* args[]){
  char* errormsg="exec failed, invalid path or command";
	if(!args) return 0;
	pid_t id=fork();
  int child_status;
  if(id==0){
    if(!execvp(args[0], args)){
		WARN("%s", errormsg);
		}
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
    if(!execv(pathname[0], pathname)){
		WARN("%s", "exec failed, invalid path or filename");
		}
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
unsigned int sz=len(str1);
INFO("NEEDLE IS:%c", with);
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
  if(S_ISDIR(fi.st_mode)){
  return 1;
  }
  return 0;
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
  debug_print("IS FILE %s", path); 
  return 1;
}

unsigned int IS_PATH_EXIST(char* path){
  if(!path) return 0;
  struct stat fi;
  if(stat(path, &fi)==-1){
    return 0;
  } 
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
  debug_print("CREATED %s", file);
  return 1;
	}
  return 0;
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
  return 1;
	}
  return 0;
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
  return 0;
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
	if(!IS_PATH_EXIST(path)){
  return 1;
	}
	return 0;
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
debug_print("COMPILED %s", file);
	} else{
debug_print("COULDNT COMPILE %s", file);
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
  debug_print("TWD %s", twd);
  char* command[]={compiler, files[i], "-o", twd, NULL};
			if(exec(command)){
			print_exec(command);
			free(twd);
			} else{
			free(twd);
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
	struct dirent *dirent;
  DIR* source_dir;
  source_dir=opendir(origin);
  if(source_dir){
    while((dirent=readdir(source_dir))!=NULL){
      if(strcmp(dirent->d_name, ".")!=0 && strcmp(dirent->d_name, "..")!=0){
	if(strcmp(ext(dirent->d_name), extension)==0){
	  char* dest_path=calloc(1, PATH_MAX);
	  char* origin_path=calloc(1, PATH_MAX);
		// char* buff=calloc(1, PATH_MAX);
		// char* dbuff=calloc(1, PATH_MAX);
		char* cwdbuff=calloc(1, PATH_MAX);
		if(strcmp(origin, destination)==0){
	    char* command[]={compiler, "-o", base(dirent->d_name), dirent->d_name, NULL};
	    exec(command);
	    debug_print("COMPILED %s", command[3]);
	    debug_print("BINARY %s", command[2]);
	    debug_print("finished compiling %s %s", command[3], command[2]);
	  }
	  else{
	    if(strcmp(origin, ".")==0){
	    strcat(origin_path, dirent->d_name);	
	    debug_print("ORIGIN:dot %s", origin_path);
	    } 
	    if(strcmp(origin, ".")!=0){
			strcat(origin_path, origin);
			if(!ends_with(origin_path, '/')){
	    strcat(origin_path, "/");
	    }
	    strcat(origin_path, dirent->d_name);
	    debug_print("ORIGIN:path %s", origin_path);
	    }
	    if(strcmp(destination, ".")==0){
	    strcat(dest_path, origin);
	    if(!ends_with(dest_path, '/')){
			strcat(dest_path, "/");
	    }
	    strcat(dest_path, base(dirent->d_name));
	    debug_print("DEST:path %s", dest_path);
	    }
	    if(strcmp(destination, ".")!=0){
	    strcat(dest_path, destination);
			debug_print("DEST:dot %s", dest_path);
	    if(!ends_with(dest_path, '/')){
			strcat(dest_path, "/");
	    }
	    strcat(dest_path, base(dirent->d_name));
	    debug_print("DEST:path %s", dest_path);
	    }
	    char* command[]={compiler, "-o", dest_path, origin_path, NULL};
	    exec(command);
			INFO("COMMAND %s", command[0]);
	    INFO("BINARY %s", command[2]);
	    INFO("SOURCE %s", command[3]);
	    INFO("[source]:%s [binary]:%s", origin_path, upcwd(dest_path, cwdbuff)); 
		}
			free(dest_path);
			free(origin_path);
			free(cwdbuff);
	}
      }
    }
  }
	return 1;
}

int renameold(char* file){
if(!file) return 0;
char* old=calloc(1, PATH_MAX);
strcat(old, file);
strcat(old, ".old");
rename(file, old);
debug_print("RENAMED TO %s", old);
return 1;
}

#define GO_REBUILD(argc, argv, compiler){																							\
	char* file=__FILE__;																																\
  if(file==NULL || argc==0)	return 0;																									\
  if(is_path1_modified_after_path2(file, argv[0])){																		\
	printf("FILE %s\n", file);																													\
	  renameold(argv[0]);																																\
    char* command[]={compiler, "-o", base(file), file, NULL};													\
    if(exec(command)){                                                                \
    debug_print("COMPILING %s", command[3]);                                                   \
    debug_print("COMPILED %s", command[2]);                                                    \
    debug_print("RUNNING %s", argv[0]);																												\
		run(base(file));																																	\
		exit(0);																																				  \
		}																																									\
	}																																										\
}																																											

#endif // COMPILATION_IMPLEMENTATION
