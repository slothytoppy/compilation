#ifndef NOMAKE_IMPLEMENTATION
void LOG_INFO(enum log_level level, const char* fmt, ...);
void nom_cmd_append(Nom_cmd* cmd, char* item);
void nom_cmd_append_many(Nom_cmd* cmd, char* item[]);
void nom_cmd_append_null(Nom_cmd* cmd);
void* nom_shift_args(int* argc, char*** argv);
char* gcwd(char* str1); // gets the cwd
char* upcwd(char* str1, char* str2); // if str1 is ../dir, it will move what is after the ../ into str2, if str2 is null it returns str1 
unsigned int exec(char* args[]); // is a wrapper for execvp 
unsigned int run(char* pathname);
unsigned int len(const char* str1); // my own strlen(not used very much but it is used)
unsigned int ends_with(char* str1, char with);
const char* ext(const char* filename);
char* base(const char* file); // only works for files that have slashes, i havent tested if it works for something like ../../file 
unsigned int IS_PATH_DIR(char* path);
unsigned int IS_PATH_FILE(char* path);
unsigned int IS_PATH_EXIST(char* path);
unsigned int MKFILE(char* file);
unsigned int RMFILE(char* file);
unsigned int CLEAN(char* directory, char* extension);
unsigned int MKDIR(char* path);
unsigned int RMDIR(char *path);
unsigned int is_path1_modified_after_path2(const char* source_path, const char* binary_path);
unsigned int print_exec(char* args[]);
unsigned int compile_file(char* file, char* destination, char* compiler, const char* extension);
unsigned int compile_targets(unsigned int sz, char* files[], char* destination, char* compiler, const char* extension);
unsigned int compile_dir(char* origin, char* destination, char* compiler, const char* extension);
unsigned int renameold(char* file);
#endif

#ifdef NOMAKE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

#define print_source(){            \
  char* file=__FILE__;             \
  LOG_INFO(NOM_INFO, "SOURCE %s", file);   \
}

typedef const char* Cstr;

enum log_level{
	NOM_INFO,
	NOM_WARN,
	NOM_PANIC,
	NOM_DEBUG,
};

typedef struct{
  char** items;
  unsigned count;
  unsigned capacity;
} Nom_cmd;

#define DEFAULT_CAP 256

void LOG_INFO(enum log_level level, const char* fmt, ...){
#ifdef DEBUG 
	return;
#endif
	switch(level){
	case NOM_DEBUG:
		fprintf(stderr, "[DEBUG] ");
	break;
	case NOM_INFO:
		fprintf(stderr, "[INFO] ");
	break;
	case NOM_WARN:
		fprintf(stderr, "[WARNING] ");
	break;
	case NOM_PANIC:
		fprintf(stderr, "[PANIC] ");
	break;
	}
va_list args;
va_start(args, fmt);
vfprintf(stderr, fmt, args);
va_end(args);
fprintf(stderr, "\n");
}

void nom_cmd_append(Nom_cmd* cmd, char* item){ 
  if(cmd->count==0){
  cmd->count+=1;
  cmd->capacity=DEFAULT_CAP;
  cmd->items=malloc(cmd->count*sizeof(cmd->items));
  cmd->items[cmd->count-1]=item; 
  }
  else{
  cmd->count+=1;
  cmd->items=realloc(cmd->items, cmd->count*sizeof(cmd->items));
  cmd->items[cmd->count-1]=item; 
  }
  if(cmd->count>=cmd->capacity){
  cmd->capacity*=2;
  cmd->items=realloc(cmd->items, cmd->capacity*sizeof(char*));
  }
}

void nom_cmd_append_many(Nom_cmd* cmd, char* item[]){
  while(*item!=NULL){
  nom_cmd_append(cmd, *item);
  *item++;
  }
}

void nom_cmd_append_null(Nom_cmd* cmd){
  if(cmd->count<=0){
  cmd->items[0]==NULL;
  } else{
  cmd->items=realloc(cmd->items, cmd->count*sizeof(char*));
  cmd->items[cmd->count]=NULL;
  }
}

void* nom_shift_args(int *argc, char*** argv){
  if(*argc<0) return;
  char* result=**argv;
  (*argv)+=1;
  (*argc)-=1;
  return result;
}

char* gcwd(void){
char buff[PATH_MAX];
return getcwd(buff, sizeof(buff));
}

unsigned int exec(char* args[]){
  char* errormsg="exec failed, invalid path or command";
	if(!args) return 0;
	pid_t id=fork();
  int child_status;
  if(id==0){
    if(!execvp(args[0], args)){
		LOG_INFO(NOM_WARN, "%s", errormsg);
		}
  } 
  if(id<0){
    printf("forking failed\n");
  return 0;
  }
  wait(&child_status);
  return 1;
}

void nom_cmd_compile(Nom_cmd* cmd){
if(cmd->count<=0) return;
pid_t id=fork();
int child_status;
if(id==0){
  if(!execvp(cmd->items[0], cmd->items)){
  LOG_INFO(NOM_WARN, "exec failed, invalid path or command");
  return;
  }
  if(id<0){
  LOG_INFO(NOM_WARN, "forking failed");
  return;
  }
  wait(&child_status);
}
printf("compile:{%s}\n", cmd->items[0]);
return;
}

unsigned int run_args(char* pathname[]){
  if(!pathname) return 0;
	pid_t id=fork();
  int child_status;
  if(id==0){
    if(!execv(pathname[0], pathname)){
		LOG_INFO(NOM_WARN, "%s", "exec failed, invalid path or filename");
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
LOG_INFO(NOM_INFO, "NEEDLE IS:%c", with);
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
  LOG_INFO(NOM_DEBUG, "IS FILE %s", path); 
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
  LOG_INFO(NOM_DEBUG, "CREATED %s", file);
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
  LOG_INFO(NOM_DEBUG, "TWD %s", twd);
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

unsigned int compile(char* compiler, char* flags[], char* origin, char* destination, char* extension, ...){
if(origin==NULL || destination==NULL || compiler==NULL || extension==NULL){
    fprintf(stderr, "origin, destination, compiler or extension was null\n");
    return 0;
}
if(strcmp(origin, ".")==0 || strcmp(destination, ".")==0 || strcmp(origin, "..")==0 || strcmp(destination, "..")==0){
LOG_INFO(NOM_WARN, "arguments to compile should not have a '.' or '..'\n");
return 0;
}
va_list args;
va_start(args, extension);
unsigned int flagc=0;
if(flags!=NULL) flagc=va_arg(args, int);
(void) flagc;
struct dirent *dirent;
DIR* source_dir;
source_dir=opendir(origin);
  if(source_dir){
	while((dirent=readdir(source_dir))!=NULL){
        	
    }
  }
	return 0;
}

unsigned int renameold(char* file){
if(!file) return 0;
char* old=calloc(1, PATH_MAX);
strcat(old, file);
strcat(old, ".old");
rename(file, old);
LOG_INFO(NOM_INFO, "RENAMED TO %s", old);
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
    LOG_INFO(NOM_DEBUG, "COMPILING %s", command[3]);                                  \
    LOG_INFO(NOM_DEBUG, "COMPILED %s", command[2]);                                   \
    LOG_INFO(NOM_DEBUG, "RUNNING %s", argv[0]);																	      \
		run(base(file));																																	\
		exit(0);																																				  \
		}																																									\
	}																																										\
}																																											
#endif
