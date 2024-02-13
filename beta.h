/*
#ifndef NOM_IMPLEMENTATION
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
unsigned int is_path1_modified_after_path2(const char* source_path, const char* binary_path);
#endif

#ifdef NOM_IMPLEMENTATION
*/
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
#include <sys/inotify.h>

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

typedef struct{
  void** items;
  unsigned count;
  unsigned capacity;
  int type;
} Dyn_arr;

#define DEFAULT_CAP 256

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

void nom_cmd_append(Nom_cmd *cmd, char *item) {
  if (cmd->count == 0) {
    cmd->capacity = DEFAULT_CAP;
    cmd->items = (char**)calloc(1, sizeof(cmd->items));
    cmd->items[0] = item;
    cmd->count++;
    return;
  }

  cmd->count += 1;
  cmd->items = (char**)realloc(cmd->items, cmd->count * sizeof(cmd->items));
  cmd->items[cmd->count - 1] = item;

  if (cmd->count+1 >= cmd->capacity) {
    cmd->capacity *= 2;
    cmd->items = (char**)realloc(cmd->items, cmd->capacity * sizeof(char *));
    if (cmd->items == NULL) {
      nom_log(NOM_PANIC, "could not allocate enough memory for cmd");
      exit(1);
    }
  }
}

void dyn_init(Dyn_arr* dyn, unsigned type){
  dyn->type=sizeof(type);
}

void dyn_arr_append(Dyn_arr* dyn, void* item){
  if(dyn->count==0){
  dyn->capacity=DEFAULT_CAP;
  dyn->items=(void**)malloc(1*dyn->type*sizeof(item));
  dyn->items[0]=item;
  dyn->count++;
  return;
  }
  dyn->count+=1;
  dyn->items=(void**)realloc(dyn->items, dyn->count*dyn->type*sizeof(item));
  dyn->items[dyn->count-1]=item;
  if(dyn->count>=dyn->capacity){
    dyn->capacity*=2;
    dyn->items=(void**)realloc(dyn->items, dyn->capacity*dyn->type);
    if(dyn->items==NULL){
      nom_log(NOM_PANIC, "could not alloc enough memory for dyn");
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
  while(i<=count){
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

unsigned int run_path(char* path, char* args[]){
  if(!path) return 0;
  pid_t pid=fork();
  if(pid<0) return 0;
  if(pid==0){
  execv(path, NULL);
  int child_status;
  if(waitpid(pid, &child_status, 0)<0) return 0;
  if(!WIFEXITED(child_status)) return 0;
  if(!WEXITSTATUS(child_status)){
    nom_log(NOM_INFO, "%s failed to run", path);
    return child_status;
  }
  if(WEXITSTATUS(child_status)==0) nom_log(NOM_INFO, "%s was ran successfully", path);
}
return 0;
}

unsigned int nom_run_async(Nom_cmd cmd) {
  if (cmd.count <= 0) {
    return 0;
  }
  cmd.items[cmd.count-1]=NULL;
  if (cmd.items[cmd.count-1] != NULL) {
  nom_log(NOM_PANIC, "cmd.items at %d is not null terminated", cmd.count);
  exit(1);
  }
  pid_t pid = fork();
  if(pid==-1){
    nom_log(NOM_PANIC, "fork failed in nom_run_async");
    return 0;
  }
  if(pid==0){
    for (int i = 0; i<cmd.count-1; i++) {
    unsigned before_null=2;
      printf("%s ", cmd.items[i]);
      if (i == cmd.count-before_null) {
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
  printf("%s ", cmd.items[i]);
  if(i==cmd.count-1) printf("\n");
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
return 0;
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

char* base(const char* file){
  if (file==NULL) return NULL;
  char *retStr;
  char *lastExt;
  if ((retStr = (char*)malloc (strlen (file) + 1)) == NULL) return NULL;
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

time_t set_mtime(char* file){
  struct utimbuf ntime;
  struct stat fi;
  ntime.actime=ntime.modtime=time(NULL);
  if(utime(file, &ntime)<0){
          fprintf(stderr, "could not update %s's timestamp\n", file);
          return 0;
  }
  if(stat(file, &fi)<0){
    nom_log(NOM_PANIC, "could not stat %s", file);
    return 0;
  }
  if(fi.st_mtime>0) return fi.st_mtime;
  else return 0;
}

unsigned int is_path1_modified_after_path2(const char* source_path, const char* binary_path){
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
Nom_cmd cmd={0};
nom_cmd_append_many(&cmd, 6, compiler, "-ggdb", file, "-o", base(file));
if(needs_rebuild(file, bin)){
  rename(bin, old_path);
  nom_log(NOM_INFO, "renamed %s to %s", bin, old_path);
  if(nom_run_sync(cmd)){
    if(!IS_PATH_EXIST(bin)){ 
      if(!IS_PATH_EXIST(old_path)){
      nom_log(NOM_WARN, "%s does not exist, no previous rollback, exiting", old_path);
      return 0;
      }
      nom_log(NOM_INFO, "renaming %s to %s", old_path, bin);
      rename(old_path, bin);
      run(bin);
      exit(0);
    }
    //nom_log(NOM_INFO, "compiled %s %s %s", command[0], command[2], command[4]);
    run(bin);
    exit(0);
  } 
}
  return 1;
}

int IS_LIBRARY_MODIFIED(char* lib, char* file, char* compiler){
  if(!lib || !file) return 0;
  struct stat fi;
  struct utimbuf ntime;
  ntime.actime=ntime.modtime=time(NULL);
  if(stat(lib, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", lib); 
  }
  unsigned int lib_time=fi.st_mtime;
  if(stat(file, &fi)<0){
    fprintf(stderr, "%s doesnt exist\n", file);
  }
  Nom_cmd cmd={0};
  nom_cmd_append(&cmd, compiler);
  nom_cmd_append(&cmd, "-ggdb");
  nom_cmd_append(&cmd, file);
  nom_cmd_append(&cmd, "-o");
  nom_cmd_append(&cmd, base(file));
  unsigned int file_time=fi.st_mtime;
  if(lib_time>file_time){
    if(nom_run_sync(cmd)){
     if(utime(file, &ntime)<0){
          fprintf(stderr, "could not update %s's timestamp\n", file);
          return 0;
        }
    return 1;
    }
  }
  return 0;
} 

unsigned int inot_setup(int(*fp)(void)){
  unsigned int res=inotify_init();
  if(fp!=NULL) fp();
  return res;
}

unsigned int inot_setup1(int flags, int(*fp)(void)){
  unsigned int res=inotify_init1(flags);
  if(fp!=NULL) fp();
  return res;
}

unsigned int nom_add_watch(unsigned fd, char* path, uint32_t mask){
  return inotify_add_watch(fd, path, mask)? 0: 1;
}

unsigned int nom_remove_watch(unsigned fd, unsigned wd){
  return inotify_rm_watch(fd, wd)? 0: 1;
}

unsigned int nom_read_inot(unsigned fd, char* bin_path, char* args[]){
  if(fd<0 || !IS_PATH_EXIST(bin_path)) return 0;
  struct inotify_event* event;
  Nom_cmd cmd={0};
  nom_cmd_append(&cmd, bin_path);
  char buff[4096];
  ssize_t len=read(fd, buff, sizeof(buff));
  for(char* in_buff=buff; in_buff<buff+len; in_buff+=sizeof(struct inotify_event)+event->len){
    event = (struct inotify_event *)in_buff;
    if(event->mask & IN_MODIFY){
    run_path(bin_path, args);
  printf("hello\n");
    } else if(event->mask & IN_CREATE){
    run_path(bin_path, args);
  printf("world\n");
    } else if(event->mask & IN_MOVE){
    run_path(bin_path, args);
  printf("bye\n");
    }
  usleep(350);
}
return 0;
}
//#endif
