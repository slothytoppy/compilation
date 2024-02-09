#ifndef NOM_IMPLEMENTATION 
char* gcwd(void); // gets the cwd
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
unsigned int IS_PATH_MODIFIED(char* path);
unsigned int MKFILE(char* file);
unsigned int RMFILE(char* file);
unsigned int CLEAN(char* directory, char* extension);
unsigned int MKDIR(char* path);
unsigned int RMDIR(char *path);
unsigned int is_path1_modified_after_path2(const char* source_path, const char* binary_path);
unsigned int print_exec(char* args[]);
unsigned compile_simple(char* file, char* compiler);
unsigned int compile_file(char* file, char* flags[], char* destination, char* compiler, const char* extension, ...);
unsigned int compile_targets(unsigned int sz, char* files[], char* destination, char* compiler, const char* extension);
unsigned int compile_dir(char* origin, char* destination, char* compiler, const char* extension);
unsigned int renameold(char* file);
#endif

#ifdef NOM_IMPLEMENTATION 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <utime.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

typedef const char* Cstr;

enum log_level{
	NOM_INFO,
	NOM_WARN,
	NOM_PANIC,
	NOM_DEBUG,
};

void nom_log(enum log_level level, const char* fmt, ...){
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

typedef struct{
  char** items;
  unsigned count;
  unsigned capacity;
} Nom_cmd;

#define DEFAULT_CAP 256

void nom_cmd_append(Nom_cmd *cmd, char *item) {
  if (cmd->count == 0) {
    cmd->capacity = DEFAULT_CAP;
    cmd->items = malloc(sizeof(cmd->items));
    cmd->items[0] = item;
    cmd->count++;
    return;
  }

  cmd->count += 1;
  cmd->items = realloc(cmd->items, cmd->count * sizeof(cmd->items));
  cmd->items[cmd->count - 1] = item;

  if (cmd->count >= cmd->capacity) {
    cmd->capacity *= 2;
    cmd->items = realloc(cmd->items, cmd->capacity * sizeof(char *));
    if (cmd->items == NULL) {
      nom_log(NOM_PANIC, "could not allocate enough memory for cmd");
      exit(1);
    }
  }
}

void nom_cmd_append_null(Nom_cmd* cmd){
  nom_cmd_append(cmd, NULL);
}

void nom_cmd_append_many(Nom_cmd* cmd, unsigned count, ...){
  va_list args;
  va_start(args, count);
  int i=0;
  while(i<count){
  char* item=va_arg(args, char*);
  nom_cmd_append(cmd, item);
  i++;
  }
  va_end(args);
}

void* nom_shift_args(int *argc, char*** argv){
  if(*argc<0) return NULL;
  char* result=**argv;
  (*argv)+=1;
  (*argc)-=1;
  return result;
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
nom_log(NOM_INFO, "%s", str2);
return str2;
}
strcat(cwd, "/");
strcat(cwd, str1);
nom_log(NOM_INFO, "%s", str1);
return str1;
}

unsigned int exec(char* args[]){
	if(!args) return 0;
	pid_t id=fork();
  int child_status;
  if(id==0){
    if(!execvp(args[0], args)){
		nom_log(NOM_WARN, "exec failed, invalid path or command");
		}
  } 
  if(id<0){
    printf("forking failed\n");
  return 0;
  }
  if(waitpid(id, &child_status, 0)<0) return 0;
  return 1;
}

unsigned int nom_run_async(Nom_cmd cmd) {
  if (cmd.count <= 0) {
    return 0;
  }
if (cmd.items[cmd.count] != NULL) {
    nom_cmd_append(&cmd, NULL);
  }
  if (cmd.items[cmd.count] != NULL) {
    nom_log(NOM_PANIC, "could not null terminate cmd");
    exit(1);
  }
  pid_t pid = fork();
  if (pid < 0) {
    nom_log(NOM_PANIC, "fork failed in nom_run_async");
    return 0;
  }

  switch (pid) {
  case -1:
    nom_log(NOM_PANIC, "fork failed in nom_run_async");
    return 0;
  case 0:
    for (int i = 0; i < cmd.count; i++) {
      printf("%s ", cmd.items[i]);
      if (i == cmd.count - 1) {
        printf("\n");
      }
    }
  }

  if (execvp(cmd.items[0], cmd.items) == -1) {
    nom_log(NOM_PANIC, "could not execute child process");
    exit(1);
  }

  return pid;
}

unsigned int nom_run_async1(Nom_cmd cmd){
  if(cmd.count<=0) return 0;
  if(cmd.items[cmd.count]!=NULL){
    nom_cmd_append(&cmd, NULL);
  }
  if(cmd.items[cmd.count]!=NULL){
  nom_log(NOM_PANIC, "could not null terminate cmd");
  exit(1);
  }
  pid_t pid=fork();
  if(pid<0){
    nom_log(NOM_PANIC, "fork failed in nom_run_async");
    return 0;
  }
  if(pid==0){
    if(execvp(cmd.items[0], cmd.items)<0){
      nom_log(NOM_PANIC, "could not execute child process");
      exit(1);
    }
  }
  if(pid>0){
  printf("executed: ");
  int i;
  for(i=0; i<cmd.count-1; i++){
  // printf("%d\n", cmd.count);
  printf("%s ", cmd.items[i]);
  }
  }
  return pid;
}

unsigned int nom_run_sync(Nom_cmd cmd){
if(cmd.count<=0) return 0;
int child_status;
pid_t id=nom_run_async(cmd);
if(waitpid(id, &child_status, 0)){
if(WIFEXITED(child_status)){
  if(WEXITSTATUS(child_status)==0){
return WEXITSTATUS(child_status);
  } else{
  return 1;
  }
}
}
}

unsigned int run_args(char* pathname[]){
  if(!pathname) return 0;
  if(exec(pathname)) return 1;
  return 0;
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
nom_log(NOM_INFO, "NEEDLE IS:%c", with);
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
  nom_log(NOM_DEBUG, "IS FILE %s", path); 
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

unsigned int IS_PATH_MODIFIED(char* path, char* path2){
  if(!path || !path2) return 0;
	struct stat fi;
  if(stat(path, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", path); 
  }
  unsigned int source_time=fi.st_mtime;
  if(stat(path2, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", path2);
  }
  unsigned int binary_time=fi.st_mtime;
  return source_time>binary_time;

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
  nom_log(NOM_DEBUG, "CREATED %s", file);
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

unsigned int renameold(char* file){
if(!file) return 0;
char* old=calloc(1, PATH_MAX);
strcat(old, file);
strcat(old, ".old");
rename(file, old);
nom_log(NOM_INFO, "RENAMED %s TO %s", file, old);
return 1;
}

unsigned int needs_rebuild(char* str1, char* str2){
  if(!str1 || !str2) return 0;
  if(*str1==' ' || *str2==' ') return 0;
	struct stat fi;
  if(stat(str1, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", str1); 
  }
  unsigned int source_time=fi.st_mtime;
  if(stat(str2, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", str2);
  }
  unsigned int binary_time=fi.st_mtime;
  return source_time>binary_time;
}

Nom_cmd needs_rebuild1(char* input_path, char** output_paths, unsigned int count){
  Nom_cmd null={0};
  if(!input_path || !output_paths) return null;

  struct stat fi = {0};
  if(stat(input_path, &fi)<0){
        nom_log(NOM_WARN, "could not stat %s %s", input_path, strerror(errno));
        return null;
  } 
  unsigned int input_time=fi.st_mtime;
  Nom_cmd cmd={0};
  for(int i=0; i<count; i++){
      if(stat(output_paths[i], &fi)<0){
        nom_log(NOM_WARN, "could not stat %s %s", output_paths, strerror(errno));
        return null;
      }
    unsigned int output_time=fi.st_mtime;
    printf("%d ", i); 
    if(output_time>input_time){
    nom_cmd_append(&cmd, output_paths[i]);
    }
  }
  return cmd;
}

unsigned int rebuild(char* file, char* compiler){
if(file==NULL || compiler==NULL) return 0;
char* bin=base(file);
char* old_path=strcat(base(file), ".old");
char* command[]={compiler, "-ggdb", file, "-o", base(file), NULL};
if(needs_rebuild(file, bin)){
  rename(bin, old_path);
  nom_log(NOM_INFO, "renamed %s to %s", bin, old_path);
  if(exec(command)){
    if(!IS_PATH_EXIST(bin)){ 
      if(!IS_PATH_EXIST(old_path)){
      nom_log(NOM_WARN, "%s does not exist, no previous rollback, exiting", old_path);
      exit(1);
      }
      nom_log(NOM_INFO, "renaming %s to %s", old_path, bin);
      rename(old_path, bin);
      run(bin);
      exit(0);
    }
    nom_log(NOM_INFO, "compiled %s %s %s", command[0], command[2], command[4]);
    run(bin);
    exit(0);
  } 
}
  return 1;
}

int update_path_time(char* path1, char* path2){
  if(!path1 || !path2) return 0;
  struct stat fi;
  struct utimbuf ntime;
  ntime.actime=ntime.modtime=time(null);
  if(stat(path1, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", path1);
    return 0;
  }
  unsigned int path1_time=fi.st_mtime;
  if(stat(path2, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", path2);
    return 0;
  }
  unsigned int path2_time=fi.st_mtime;
  if(utime(path1, &ntime)<0){
    fprintf(stderr, "could not update %s's timestamp\n", path1);
    return 0;
  }
  if(utime(path2, &ntime)<0){
    fprintf(stderr, "could not update %s's timestamp\n", path2);
    return 0;
  }
  return path1_time==path2_time;
}

int is_library_modified(char* lib, char* file, char* compiler){
  if(!lib || !file) return 0;
  struct stat fi;
  struct utimbuf ntime;
  ntime.actime=ntime.modtime=time(null);
  if(stat(lib, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", lib); 
  }
  unsigned int lib_time=fi.st_mtime;
  if(stat(file, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", file);
  }
  unsigned int file_time=fi.st_mtime;
  if(lib_time>file_time){
  char* command[]={compiler, "-ggdb", file, "-o", base(file), null};
    if(exec(command)){
      nom_log(nom_info, "compiled %s %s -o %s", command[0], command[2], command[4]);
        if(utime(file, &ntime)<0){
          fprintf(stderr, "could not update %s's timestamp\n", file);
          return 0;
        }
      return 1;
  nom_cmd cmd={0};
  nom_cmd_append(&cmd, compiler);
  nom_cmd_append(&cmd, "-ggdb");
  nom_cmd_append(&cmd, file);
  nom_cmd_append(&cmd, "-o");
  nom_cmd_append(&cmd, base(file));
  unsigned int file_time=fi.st_mtime;
  if(lib_time>file_time){
    if(nom_run_sync(cmd)){ 
    return 1;
    }
  }
  return 0;
} 
}
}

// simple rebuild implementation but should always work
#define GO_REBUILD(argc, argv, compiler){																							\
	char* file=__FILE__;																																\
  if(file==NULL || argc==0)	return 0;																									\
  if(needs_rebuild(file, argv[0])){																		                \
	  renameold(argv[0]);																																\
    char* command[]={compiler, "-ggdb", "-o", base(file), file, NULL};								\
    if(exec(command)){                                                                \
		nom_log(NOM_INFO, "compiled %s running %s", file, argv[0]);                       \
    run(argv[0]);																																	    \
		exit(0);																																				  \
		}                                                                                 \
	}																																										\
}                                                                                     

#endif // NOM_IMPLEMENTATION
