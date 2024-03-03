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

enum log_level {
  NOM_INFO,
  NOM_DEBUG,
  NOM_WARN,
  NOM_PANIC,
  NOM_NONE,
};

typedef struct {
  int lines;
  int show_mode;
  int show_debug;
} nom_debug_logger;

#define ON 0
#define OFF 1

#define DEFAULT_CAP 256

// everything on by default, options are OPT OUT, there may be some exceptions to this
nom_debug_logger nom_logger = {0};

const char* const color[] = {
    "\033[38;5;208m[INFO]\033[0m",
    "\033[38;5;241m[DEBUG]\033[0m",
    "\033[38;5;1m[WARN]\033[0m",
    "\033[38;5;196m[PANIC]\033[0m",
};

void nom_logger_reset(void) {
  nom_logger.lines = ON;
  nom_logger.show_mode = ON;
  nom_logger.show_debug = ON;
  return;
}

void nom_gen_log(enum log_level level, char* fmt, va_list args) {
  if(!fmt || nom_logger.show_debug == OFF && level == NOM_DEBUG) {
    return;
  }
  if(level == NOM_NONE) {
    nom_logger.show_mode = OFF;
  }
  if(nom_logger.show_mode == OFF) {
    vfprintf(stderr, fmt, args);
    if(nom_logger.lines == ON) {
      fprintf(stderr, "\n");
    }
    return;
  }
  char* nfmt = (char*)calloc(1, strlen(fmt) + 1);
  nfmt[0] = ' ';
  strncat(nfmt + 1, fmt, strlen(fmt));
  if(nom_logger.show_mode == ON) {
    fprintf(stderr, "%s", color[level]);
    vfprintf(stderr, nfmt, args);
    if(nom_logger.lines == ON) {
      fprintf(stderr, "\n");
    }
  }
  return;
}

void nom_log(enum log_level level, char* fmt, ...) {
  if(!fmt) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  nom_gen_log(level, fmt, args);
  va_end(args);
  return;
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

unsigned int mkdir_if_not_exist(char* path) {
  if(!path)
    return 0;
  struct stat fi;
  int len = strnlen(path, 4096);
  char* dir = (char*)calloc(1, len);
  for(int i = 0; i <= len; i++) {
    if(path[i] == '/') {
      dir = strncpy(dir, path, i);
      if(mkdir(dir, 0755) < 0) {
        if(errno == 17) {
          nom_log(NOM_INFO, "%s already exists", dir);
        } else {
          nom_log(NOM_WARN, "error in %s:%s", __FUNCTION__, strerror(errno));
        }
      }
      nom_log(NOM_INFO, "%c:%d", path[i], i);
      nom_log(NOM_INFO, "len:%d file:%s", len, dir);
    }
  }
  return 1;
}

unsigned int mkfile_if_not_exist(char* path) {
  if(!path)
    return 0;
  struct stat fi;
  unsigned int len = strnlen(path, 255);
  if(stat(path, &fi) < 0) {
    if(errno == EEXIST) {
      nom_log(NOM_DEBUG, "file: %s already exists");
      return 1;
    }
  }
  char* file = (char*)calloc(1, len);
  for(int i = 0; i <= len; i++) {
    strncpy(file, path, i);
    if(file[i] == '/') {
      if(mkdir(file, 0755) < 0) {
        if(errno == 17) {
          nom_log(NOM_INFO, "%s already exists", file);
        } else {
          nom_log(NOM_WARN, "error in %s:%s", __FUNCTION__, strerror(errno));
        }
      }
    }
  }
  if(creat(file, 0644) < 0) {
    nom_log(NOM_WARN, "mkfile error:%s %s", file, strerror(errno));
    return 0;
  }
  return 1;
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

// gets rid of program name by increment argv and decrementing argc
void* nom_shift_args(int* argc, char*** argv) {
  if(*argc < 0)
    return NULL;
  char* result = **argv;
  (*argv) += 1;
  (*argc) -= 1;
  return result;
}

typedef struct
{
  char** items;
  unsigned count;
  unsigned capacity;
} Nom_cmd;

void nom_log_cmd(enum log_level level, char* msg, Nom_cmd cmd) {
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    return;
  }
  nom_logger.lines = OFF;
  nom_log(level, msg);
  nom_logger.show_mode = OFF;
  nom_log(NOM_NONE, " ");
  for(int i = 0; i < cmd.count; i++) { // no null checks because cmd.items[0] is empty or the cmd.items[cmd.count] is null
    nom_log(NOM_NONE, "%s ", cmd.items[i]);
  }
  nom_log(NOM_NONE, "\n");
  nom_logger_reset();
  return;
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
  return;
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
  if(cmd->count == 0)
    return;
  for(int i = 0; i < cmd->count; i++) {
    cmd->items[i] = NULL;
  }
  cmd->count = 0;
  return;
}

unsigned int nom_run_path(Nom_cmd cmd, char* args[]) {
  nom_log(NOM_INFO, "starting nom_run_path");
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    nom_log(NOM_WARN, "can not run nom_run_path, cmd struct is empty");
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
      nom_log(NOM_WARN, "could not wait on child:%s", strerror(errno));
      return 0;
    }
    if(WEXITSTATUS(child_status) != 0) {
      nom_log(NOM_WARN, "failed to run %s", cmd.items[0]);
      return 0;
    }
    if(WEXITSTATUS(child_status) == 0) {
      nom_log_cmd(NOM_INFO, "nom_run_path ran:", cmd);
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
    nom_log(NOM_WARN, "can not run nom_run_async, cmd struct is empty");
    return 0;
  }
  nom_log(NOM_INFO, "starting to run: %s", cmd.items[0]);
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
  nom_log_cmd(NOM_INFO, "async ran:", cmd);
  return pid;
}

unsigned int nom_run_sync(Nom_cmd cmd) {
  nom_log(NOM_INFO, "starting sync");
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    nom_log(NOM_WARN, "can not run nom_run_sync, cmd struct is empty");
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
      nom_log_cmd(NOM_INFO, "sync ran:", cmd);
      if(WIFSIGNALED(child_status)) {
        nom_log(NOM_WARN, "command process was terminated by %s", strsignal(WTERMSIG(child_status)));
        return 0;
      }
      return 1;
    }
  }
  return 0;
}

unsigned int rebuild(int argc, char* argv[], char* file, char* compiler) {
  char* old_path = strcat(base(file), ".old");
  char* bin = base(file);
  if(file == NULL || compiler == NULL || argc < 1 || !compiler) {
    nom_logger.lines = OFF;
    nom_log(NOM_PANIC, "failed to rebuild because:");
    nom_logger.show_mode = OFF;
    if(file == NULL) {
      nom_log(NOM_NONE, "file was null");
    }
    if(compiler == NULL) {
      nom_log(NOM_NONE, "compiler was null");
    }
    if(argc < 1) { // apparently argc can be less than 0 if you run it using certain exec function
      nom_log(NOM_NONE, "argc was %d instead of >= 1", argc);
    }
    nom_logger_reset();
    return 0;
  }
  if(!IS_PATH_EXIST(file)) {
    nom_log(NOM_WARN, "rebuild failed: %s does not exist", file);
    return 0;
  }
  if(!IS_PATH_EXIST(old_path)) {
    nom_log(NOM_WARN, "%s does not exist, no previous rollback, exiting", old_path);
    exit(1);
  }
  nom_logger.show_mode = ON;
  if(!needs_rebuild(file, base(file)))
    return 0;
  nom_log(NOM_INFO, "starting rebuild");
  rename(bin, old_path);
  nom_log(NOM_DEBUG, "renamed %s to %s", bin, old_path);
  nom_logger.lines = OFF;
  nom_log(NOM_DEBUG, "argv: ");
  if(nom_logger.show_debug == ON) {
    for(int i = 0; i < argc; i++) {
      nom_log(NOM_NONE, "%s", argv[i]);
    }
    nom_log(NOM_NONE, "\n");
  }
  nom_logger.lines = ON;
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 5, compiler, "-ggdb", file, "-o", bin);
  if(!nom_run_sync(cmd)) {
    nom_log_cmd(NOM_WARN, "failed to run:", cmd);
  }
  nom_log_cmd(NOM_INFO, "rebuild ran:", cmd);
  if(!IS_PATH_EXIST(bin)) { // checks if bin was removed by the compiler, this happens on some compiler errors
    nom_log(NOM_WARN, "%s did not exist, renaming %s to %s", bin, old_path, bin);
    rename(old_path, bin);
  }
  Nom_cmd run = {0};
  nom_cmd_append(&run, base(file));
  if(nom_run_path(run, argv)) {
    nom_log(NOM_INFO, "ending rebuild");
    exit(0);
  }
  nom_logger.lines = OFF;
  return 1;
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
  struct stat fi;
  if(!lib || !file || !compiler || !IS_PATH_EXIST(lib) || !IS_PATH_EXIST(file)) {
    nom_logger.lines = OFF;
    nom_log(NOM_PANIC, "failed to rebuild in IS_LIBRARY_MODIFIED because:");
    nom_logger.show_mode = OFF;
    if(!lib) {
      nom_log(NOM_NONE, "lib was null");
    }
    if(!file) {
      nom_log(NOM_NONE, "file was null");
    }
    if(!compiler) {
      nom_log(NOM_NONE, "compiler was null");
    }
    if(!IS_PATH_EXIST(lib)) {
      nom_log(NOM_NONE, "lib:%s does not exist", lib);
    }
    if(!IS_PATH_EXIST(file)) {
      nom_log(NOM_NONE, "file:%s does not exist", file);
    }
    nom_log(NOM_NONE, "\n");
    nom_logger_reset();
    return 0;
  }
  if(stat(lib, &fi) < 0) {
    nom_log(NOM_PANIC, "statting lib errored with:%s", strerror(errno));
    return 0;
  }
  unsigned int lib_time = fi.st_mtime;
  if(stat(file, &fi) < 0) {
    nom_log(NOM_PANIC, "statting lib errored with:%s", strerror(errno));
    return 0;
  }
  unsigned int file_time = fi.st_mtime;
  if(lib_time < file_time) {
    return 0;
  }
  struct utimbuf ntime;
  ntime.actime = ntime.modtime = time(NULL);
  nom_log(NOM_DEBUG, "beginning IS_LIBRARY_MODIFIED");
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 5, compiler, "-ggdb", file, "-o", base(file));
  if(nom_run_sync(cmd)) {
    set_mtime(file);
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

/*
typedef struct
{
  void** items;
  unsigned count;
  unsigned capacity;
  int type;
} Dyn_arr;

void dyn_init(Dyn_arr* dyn, unsigned size_of_type) {
  dyn->type = sizeof(size_of_type);
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
*/

// #endif
