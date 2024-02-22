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
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
// TODO: overhaul logging

enum log_level {
  NOM_INFO,
  NOM_WARN,
  NOM_PANIC,
  NOM_DEBUG,
  NOM_NONE,
};

#define ON 0
#define OFF 1

typedef struct
{
  char** items;
  unsigned count;
  unsigned capacity;
} Nom_cmd;

typedef struct
{
  void** items;
  unsigned count;
  unsigned capacity;
  int type;
} Dyn_arr;

#define DEFAULT_CAP 256

typedef struct {
  char* fin;
  char* fout;
} finfo;
typedef struct {
  char* debug_color;
  char* info_color;
  char* warn_color;
  char* panic_color;
} colors;
typedef struct {
  int lines;
  int show_mode;
  int show_debug;
  colors colors;
  finfo finfo;
} nom_debug_logger;

// everything on by default, options are OPT OUT, there may be some exceptions to this
nom_debug_logger nom_logger = {0};

void nom_logger_reset_colors() {
  if(nom_logger.colors.debug_color == NULL) {
    nom_logger.colors.debug_color = "\033[38;5;241m[DEBUG]\033[0m";
  }
  if(nom_logger.colors.info_color == NULL) {
    nom_logger.colors.info_color = "\033[38;5;208m[INFO]\033[0m";
  }
  if(nom_logger.colors.warn_color == NULL) {
    nom_logger.colors.warn_color = "\033[38;5;1m[WARN]\033[0m";
  }
  if(nom_logger.colors.panic_color == NULL) {
    nom_logger.colors.panic_color = "\033[38;5;196m[PANIC]\033[0m";
  }
  return;
}

void nom_logger_reset(void) {
  nom_logger.lines = ON;
  nom_logger.show_mode = ON;
  nom_logger.show_debug = ON;
  return;
}

void nom_logger_reset_all() {
  nom_logger_reset_colors();
  nom_logger.lines = ON;
  nom_logger.show_mode = ON;
  nom_logger.show_debug = ON;
  return;
}

void nom_gen_log(enum log_level level, const char* fmt, va_list args) {
  if(!fmt) {
    return;
  }
  nom_logger_reset_colors();
  if(nom_logger.show_mode == OFF) {
    level = NOM_NONE;
  }
  if(level == NOM_DEBUG && nom_logger.show_debug == ON) {
    fprintf(stderr, "%s ", nom_logger.colors.debug_color);
    vfprintf(stderr, fmt, args);
    if(nom_logger.lines == ON) {
      fprintf(stderr, "\n");
    }
    return;
  } else if(level == NOM_INFO) {
    fprintf(stderr, "%s ", nom_logger.colors.info_color);
    vfprintf(stderr, fmt, args);
    if(nom_logger.lines == ON) {
      fprintf(stderr, "\n");
    }
  } else if(level == NOM_WARN) {
    fprintf(stderr, "%s ", nom_logger.colors.warn_color);
    vfprintf(stderr, fmt, args);
    if(nom_logger.lines == ON) {
      fprintf(stderr, "\n");
    }
  } else if(level == NOM_PANIC) {
    fprintf(stderr, "%s ", nom_logger.colors.panic_color);
    vfprintf(stderr, fmt, args);
    if(nom_logger.lines == ON) {
      fprintf(stderr, "\n");
    }
  }
  return;
}

void nom_log(enum log_level level, const char* fmt, ...) {
  if(!fmt) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  nom_gen_log(level, fmt, args);
  va_end(args);
  return;
}

void nom_debug_mode(enum log_level level, char* msg) {
  if(!msg)
    return;
  if(level == NOM_DEBUG && nom_logger.show_debug == ON) {
    fprintf(stderr, "%s ", nom_logger.colors.debug_color);
    fprintf(stderr, "%s", msg);
    if(nom_logger.lines == ON) {
      fprintf(stderr, "\n");
    }
  }
  return;
}

int nom_debug_mode_cmd(enum log_level level, char* msg, Nom_cmd cmd) {
  if(!msg || cmd.items[0] == NULL || level != NOM_DEBUG)
    return 0;
  if(level == NOM_DEBUG && nom_logger.show_debug == ON) {
    fprintf(stderr, "%s ", nom_logger.colors.debug_color);
    fprintf(stderr, "%s", msg);
    for(int i = 0; i < cmd.count; i++) {
      fprintf(stderr, "%s ", cmd.items[i]);
    }
    if(nom_logger.lines == ON) {
      fprintf(stderr, "\n");
    }
    return 1;
  }
}

void nom_log_cmd(enum log_level level, char* msg, Nom_cmd cmd) {
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    return;
  }
  int debug = nom_debug_mode_cmd(level, msg, cmd);
  if(level == NOM_DEBUG && debug) {
    return;
  }
  if(level == NOM_INFO) {
    fprintf(stderr, "%s ", nom_logger.colors.info_color);
  } else if(level == NOM_WARN) {
    fprintf(stderr, "%s ", nom_logger.colors.warn_color);
  } else if(level == NOM_PANIC) {
    fprintf(stderr, "%s ", nom_logger.colors.panic_color);
  }
  fprintf(stderr, "%s ", msg);
  for(int i = 0; i < cmd.count; i++) {
    if(i > 0 && cmd.items[i] == NULL) {
      nom_log(NOM_WARN, "cmd->%d is null", i);
    }
    fprintf(stderr, "%s ", cmd.items[i]);
  }
  if(!debug) {
    fprintf(stderr, "\n");
  }
  return;
}

void logger(enum log_level level, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  nom_gen_log(level, fmt, args);
}

void nom_file_reset(char* file) {
  struct stat fi;
  if(stat(file, &fi) < 0) {
    perror("stat");
  }
  if(truncate(file, 0) < 0) {
    perror("truncate");
  }
}

void nom_cmd_append(Nom_cmd* cmd, char* item) {
  if(cmd->count == 0) {
    cmd->capacity = DEFAULT_CAP;
    cmd->items = (char**)calloc(1, sizeof(cmd->items));
    cmd->items[0] = item;
    cmd->count++;
    return;
  }

  cmd->count += 1;
  cmd->items = (char**)realloc(cmd->items, (cmd->count + 1) * sizeof(cmd->items));
  cmd->items[cmd->count - 1] = item;
  cmd->items[cmd->count] = NULL;

  if(cmd->count + 1 >= cmd->capacity) {
    cmd->capacity *= 2;
    cmd->items = (char**)realloc(cmd->items, cmd->capacity * sizeof(char*));
    if(cmd->items == NULL) {
      nom_log(NOM_PANIC, "could not allocate enough memory for cmd; buy more ram smh");
      return;
    }
  }
}

void nom_cmd_append_many(Nom_cmd* cmd, unsigned count, ...) {
  va_list args;
  va_start(args, count);
  int i = 0;
  while(i < count) {
    char* item = va_arg(args, char*);
    nom_cmd_append(cmd, item);
    i++;
  }
  va_end(args);
}

void nom_cmd_shrink(Nom_cmd* cmd, size_t count, int arr[]) {
  if(count > cmd->count)
    return;
  Nom_cmd shrunk = {0};
  for(int i = 0; i < count; i++) {
    if(arr[i] < cmd->count) {
      cmd->items[arr[i]] = NULL;
      nom_log(NOM_DEBUG, "arr[%d]=%d", i, arr[i]);
    }
  }
  for(int i = 0; i <= cmd->count; i++) {
    if(cmd->items[i] == NULL) {
      continue;
    }
    nom_cmd_append(&shrunk, cmd->items[i]);
    nom_log(NOM_DEBUG, "%s", cmd->items[i]);
  }
  return;
}

void nom_cmd_reset(Nom_cmd* cmd) {
  for(int i = 0; i < cmd->count; i++) {
    cmd->items[i] = NULL;
  }
  cmd->count = 0;
  return;
}

void* nom_shift_args(int* argc, char*** argv) {
  if(*argc < 0)
    return NULL;
  char* result = **argv;
  (*argv) += 1;
  (*argc) -= 1;
  return result;
}

unsigned int exec(char* args[]) {
  if(!args)
    return 0;
  pid_t pid = fork();
  int child_status;
  if(pid == 0) {
    if(!execvp(args[0], args)) {
      nom_log(NOM_WARN, "exec failed, invalid path or command");
    }
  }
  if(pid < 0) {
    nom_log(NOM_WARN, "forking failed");
    return 0;
  }
  if(waitpid(pid, &child_status, 0) < 0)
    return 0;
  return 1;
}

unsigned int nom_run_path(Nom_cmd cmd, char* args[]) {
  nom_log(NOM_DEBUG, "starting nom_run_path");
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    nom_log(NOM_PANIC, "can not run nom_run_path, cmd struct is empty");
    return 0;
  }
  int child_status;
  pid_t pid = fork();
  if(pid == 0) {
    execv(cmd.items[0], args);
    return 0;
  }
  if(pid > 0) {
    if(waitpid(pid, &child_status, 0) < 0) {
      nom_log(NOM_WARN, "could not wait on child");
      return 0;
    }
    if(WEXITSTATUS(child_status) != 0) {
      nom_log(NOM_WARN, "failed to run %s", cmd.items[0]);
      return 0;
    }
    if(WEXITSTATUS(child_status) == 0) {
      nom_log_cmd(NOM_DEBUG, "nom_run_path ran:", cmd);
      return 1;
    }
    if(WIFSIGNALED(child_status)) {
      nom_log(NOM_WARN, "command process was terminated by %s", strsignal(WTERMSIG(child_status)));
      return 0;
    }
  }
  return 0;
}

unsigned int nom_run_async(Nom_cmd cmd) {
  nom_log(NOM_DEBUG, "starting async");
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    nom_log(NOM_PANIC, "can not run nom_run_async, cmd struct is empty");
    return 0;
  }
  nom_log(NOM_DEBUG, "starting to run: %s", cmd.items[0]);
  pid_t pid = fork();
  if(pid == -1) {
    nom_log(NOM_WARN, "fork failed in nom_run_async");
    return 0;
  }
  if(pid == 0) {
    if(execvp(cmd.items[0], cmd.items) == -1) {
      nom_log(NOM_WARN, "could not execute child process");
      return 0;
    }
  }
  nom_log_cmd(NOM_DEBUG, "async ran:", cmd);
  return pid;
}

unsigned int nom_run_sync(Nom_cmd cmd) {
  nom_log(NOM_DEBUG, "starting sync");
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    nom_log(NOM_PANIC, "can not run nom_run_sync, cmd struct is empty");
    return 0;
  }
  nom_log(NOM_DEBUG, "starting to run: %s", cmd.items[0]);
  pid_t pid = fork();
  int child_status;
  if(pid == -1) {
    nom_log(NOM_WARN, "fork failed in nom_run_async");
    return 0;
  }
  if(pid == 0) {
    execvp(cmd.items[0], cmd.items);
  }
  if(pid > 0) {
    if(waitpid(pid, &child_status, 0) < 0) {
      return 0;
    }
    if(WEXITSTATUS(child_status) != 0) {
      nom_log(NOM_WARN, "failed to run %s", cmd.items[0]);
      return 0;
    }
    if(WEXITSTATUS(child_status) == 0) {
      nom_log_cmd(NOM_DEBUG, "sync ran:", cmd);
      if(WIFSIGNALED(child_status)) {
        nom_log(NOM_WARN, "command process was terminated by %s", strsignal(WTERMSIG(child_status)));
        return 0;
      }
      return 1;
    }
  }
  return 0;
}

unsigned int run_args(char* pathname[]) {
  if(!pathname)
    return 0;
  if(exec(pathname))
    return 1;
  return 0;
}

unsigned int run(char* pathname) {
  if(!pathname)
    return 0;
  char* command[] = {pathname, NULL};
  if(run_args(command)) {
    return 1;
  }
  return 0;
}

char* base(const char* file) {
  if(file == NULL)
    return NULL;
  char* retStr;
  char* lastExt;
  if((retStr = (char*)malloc(strlen(file) + 1)) == NULL)
    return NULL;
  strcpy(retStr, file);
  lastExt = strrchr(retStr, '.');
  if(lastExt != NULL)
    *lastExt = '\0';
  return retStr;
}

int ends_substr(char* str1, char* str2) {
  if(!str1 | !str2)
    return 0;
  int ned = strlen(str2);
  int j = 0;
  if(ned > strlen(str1))
    return 0;
  for(int i = strlen(str1) - ned; i < ned; i++) {
    if(str1[i] == str2[j]) {
      j++;
      continue;
    }
    return 0;
  }
  return 1;
}

int has_substr(const char* const str1, const char* const str2) {
  if(!str1 | !str2)
    return 0;
  int needle = strlen(str2);
  int j = 0;
  for(int i = strlen(str1) - needle; i < needle; i++) {
    if(str1[i] == str2[j]) {
      j++;
      continue;
    }
    return 0;
  }
  return 1;
}

unsigned int IS_PATH_DIR(char* path) {
  if(!path)
    return 0;
  struct stat fi;
  if(stat(path, &fi) < 0) {
    if(errno == ENOENT)
      nom_log(NOM_WARN, "could not open %s", path);
    perror("errno");
    return 0;
  }
  if(S_ISDIR(fi.st_mode)) {
    return 1;
  }
  return 0;
}

unsigned int IS_PATH_FILE(char* path) {
  if(!path)
    return 0;
  struct stat fi;
  if(stat(path, &fi) < 0) {
    if(errno == ENOENT) {
      nom_log(NOM_WARN, "%s doesnt exist", path);
    }
    return 0;
  }
  if(!S_ISREG(fi.st_mode)) {
    return 0;
  }
  nom_log(NOM_DEBUG, "IS FILE %s", path);
  return 1;
}

unsigned int IS_PATH_EXIST(char* path) {
  if(!path)
    return 0;
  struct stat fi;
  if(stat(path, &fi) == -1) {
    return 0;
  }
  return 1;
}

// it doesnt work because source time is always less than now
unsigned int IS_PATH_MODIFIED(char* path) {
  if(!path)
    return 0;
  struct stat fi;
  if(stat(path, &fi) < 0) {
    nom_log(NOM_WARN, "%s doesnt exist", path);
  }
  unsigned int source_time = fi.st_mtime;
  time_t now = time(NULL);
  struct tm* curtime = localtime(&now);
  return source_time >= now;
}

unsigned int mkfile_if_not_exist(char* file) {
  if(!file)
    return 0;
  struct stat fi;
  if(stat(file, &fi) < 0) {
    if(errno == EEXIST) {
      nom_log(NOM_DEBUG, "dir: %s already exists");
      return 1;
    }
    if(creat(file, 0644) < 0) {
      nom_log(NOM_WARN, "mkfile error:%s %d", file, errno);
      return 0;
    }
  }
  return 1;
}

unsigned int mkdir_if_not_exist(char* path) {
  if(!path)
    return 0;
  struct stat fi;
  if(stat(path, &fi) != 0) {
    mode_t perms = S_IRWXU | S_IRWXG | S_IRWXO;
    if(mkdir(path, perms) < 0) {
      nom_log(NOM_WARN, "mkdir error:%s %d", path, errno);
      return 0;
    }
  }
  if(IS_PATH_EXIST(path)) {
    return 1;
  }
  return 0;
}

time_t set_mtime(char* file) {
  struct utimbuf ntime;
  struct stat fi;
  ntime.actime = ntime.modtime = time(NULL);
  if(utime(file, &ntime) < 0) {
    nom_log(NOM_WARN, "could not update %s's timestamp", file);
    return 0;
  }
  if(stat(file, &fi) < 0) {
    nom_log(NOM_WARN, "could not stat %s", file);
    return 0;
  }
  if(fi.st_mtime > 0)
    return fi.st_mtime;
  else
    return 0;
}

unsigned int needs_rebuild(char* str1, char* str2) {
  if(strlen(str1) <= 0 || strlen(str2) <= 0)
    return 0;
  struct stat fi;
  if(stat(str1, &fi) < 0) {
    nom_log(NOM_WARN, "%s doesnt exist", str1);
  }
  unsigned int source_time = fi.st_mtime;
  if(stat(str2, &fi) < 0) {
    nom_log(NOM_WARN, "%s doesnt exist", str2);
  }
  unsigned int binary_time = fi.st_mtime;
  return source_time > binary_time;
}

Nom_cmd needs_rebuild1(char* input_path, char** output_paths, unsigned int count) {
  Nom_cmd null = {0};
  if(!input_path || !output_paths)
    return null;
  struct stat fi = {0};
  if(stat(input_path, &fi) < 0) {
    nom_log(NOM_WARN, "could not stat %s %s", input_path, strerror(errno));
    return null;
  }
  unsigned int input_time = fi.st_mtime;
  Nom_cmd cmd = {0};
  for(int i = 0; i < count; i++) {
    if(stat(output_paths[i], &fi) < 0) {
      nom_log(NOM_WARN, "could not stat %s %s", output_paths, strerror(errno));
      return null;
    }
    unsigned int output_time = fi.st_mtime;
    if(output_time > input_time) {
      nom_cmd_append(&cmd, output_paths[i]);
    }
  }
  return cmd;
}

unsigned int rebuild(char* file, char* compiler) {
  if(file == NULL || compiler == NULL)
    return 0;
  char* bin = base(file);
  if(!needs_rebuild(file, bin))
    return 0;
  nom_log(NOM_DEBUG, "starting rebuild");
  char* old_path = strcat(base(file), ".old");
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 5, compiler, "-ggdb", file, "-o", base(file));
  rename(bin, old_path);
  if(!IS_PATH_EXIST(old_path)) {
    nom_log(NOM_WARN, "%s does not exist, no previous rollback, exiting", old_path);
  }
  nom_log(NOM_INFO, "renamed %s to %s", bin, old_path);
  if(!nom_run_sync(cmd)) {
    return 0;
  }
  if(!IS_PATH_EXIST(bin)) {
    nom_log(NOM_INFO, "%s did not exist, renaming %s to %s", bin, old_path, bin);
    rename(old_path, bin);
  }
  Nom_cmd run = {0};
  nom_cmd_append(&run, base(file));
  if(nom_run_path(run, NULL)) {
    nom_log(NOM_DEBUG, "ending rebuild");
    exit(0);
  }
  nom_log(NOM_WARN, "rebuild failed");
  exit(1);
}

int update_path_time(char* path1, char* path2) {
  if(!path1 || !path2)
    return 0;
  struct stat fi;
  struct utimbuf ntime;
  ntime.actime = ntime.modtime = time(NULL);
  if(stat(path1, &fi) < 0) {
    fprintf(stderr, "%s doesnt exist\n", path1);
    return 0;
  }
  unsigned int path1_time = fi.st_mtime;
  if(stat(path2, &fi) < 0) {
    fprintf(stderr, "%s doesnt exist\n", path2);
    return 0;
  }
  unsigned int path2_time = fi.st_mtime;
  if(utime(path1, &ntime) < 0) {
    fprintf(stderr, "could not update %s's timestamp\n", path1);
    return 0;
  }
  if(utime(path2, &ntime) < 0) {
    fprintf(stderr, "could not update %s's timestamp\n", path2);
    return 0;
  }
  return path1_time == path2_time;
}

int IS_LIBRARY_MODIFIED(char* lib, char* file, char* compiler) {
  if(!lib || !file)
    return 0;
  struct stat fi;
  if(stat(lib, &fi) < 0) {
    nom_log(NOM_WARN, "%s doesnt exist", lib);
  }
  unsigned int lib_time = fi.st_mtime;
  if(stat(file, &fi) < 0) {
    nom_log(NOM_WARN, "%s doesnt exist", file);
  }
  unsigned int file_time = fi.st_mtime;
  if(lib_time < file_time) {
    return 0;
  }
  struct utimbuf ntime;
  ntime.actime = ntime.modtime = time(NULL);
  nom_log(NOM_INFO, "beginning IS_LIBRARY_MODIFIED");
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 5, compiler, "-ggdb", file, "-o", base(file));
  if(nom_run_sync(cmd)) {
    if(utime(file, &ntime) < 0) {
      nom_log(NOM_WARN, "could not update %s's timestamp", file);
      return 0;
    }
  }
  Nom_cmd run = {0};
  nom_cmd_append(&run, base(file));
  if(nom_run_path(run, NULL)) {
    nom_log(NOM_DEBUG, "ending IS_LIBRARY_MODIFIED");
    exit(0);
  }
  nom_log(NOM_WARN, "IS_LIBRARY_MODIFIED failed");
  exit(1);
}

void dyn_init(Dyn_arr* dyn, unsigned type) {
  dyn->type = sizeof(type);
  dyn->count = 0;
  dyn->capacity = DEFAULT_CAP;
}

void dyn_arr_append(Dyn_arr* dyn, void* item) {
  if(dyn->count == 0) {
    dyn->items = (void**)malloc(1 * dyn->type * sizeof(item));
    dyn->items[0] = item;
    dyn->count++;
    return;
  }
  dyn->count += 1;
  dyn->items = (void**)realloc(dyn->items, dyn->count * dyn->type * sizeof(item));
  dyn->items[dyn->count - 1] = item;
  if(dyn->count >= dyn->capacity) {
    dyn->capacity *= 2;
    dyn->items = (void**)realloc(dyn->items, dyn->capacity * dyn->type);
    if(dyn->items == NULL) {
      nom_log(NOM_PANIC, "could not alloc enough memory for dyn; buy more ram smh");
      return;
    }
  }
}

// #endif
