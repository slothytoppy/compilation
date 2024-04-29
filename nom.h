/*
the structure of the functions in the header: Logger, Nom_cmd, build system functionality with some helper functions, and some memory functions
*/

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

typedef struct nom_debug_logger {
  int new_line;
  int show_mode;
  int show_debug;
  int show_msg;
} nom_debug_logger;

#define ON 0
#define OFF 1

#define DEFAULT_CAP 256

// everything on by default, options are OPT OUT, there may be some exceptions to this
nom_debug_logger nom_logger = {0};

enum log_level {
  NOM_INFO,
  NOM_DEBUG,
  NOM_WARN,
  NOM_PANIC,
  NOM_NONE,
};

const char* const color[] = {
    "\033[38;5;208m[INFO]\033[0m",
    "\033[38;5;241m[DEBUG]\033[0m",
    "\033[38;5;1m[WARN]\033[0m",
    "\033[38;5;196m[PANIC]\033[0m",
};

void nom_logger_toggle_new_line(int toggle) {
  nom_logger.new_line = toggle;
}

void nom_logger_toggle_show_mode(int toggle) {
  nom_logger.show_mode = toggle;
}

void nom_logger_toggle_show_debug(int toggle) {
  nom_logger.show_debug = toggle;
}

void nom_logger_toggle_msg(int toggle) {
  nom_logger.show_msg = toggle;
}

void nom_logger_reset(void) {
  nom_logger.new_line = ON;
  nom_logger.show_mode = ON;
  nom_logger.show_debug = ON;
  nom_logger.show_msg = ON;
  return;
}

void nom_gen_log(enum log_level level, char* fmt, va_list args) {
  if(!fmt || nom_logger.show_debug == OFF && level == NOM_DEBUG) {
    return;
  }
  if(nom_logger.show_msg == OFF) {
    return;
  }
  if(level == NOM_NONE) {
    nom_logger.show_mode = OFF;
  }
  if(nom_logger.show_mode == OFF) {
    vfprintf(stderr, fmt, args);
    if(nom_logger.new_line == ON) {
      fprintf(stderr, "\n");
    }
    // nom_logger_toggle_show_mode(ON);
    return;
  }
  char* nfmt = (char*)calloc(1, strlen(fmt) + 1);
  nfmt[0] = ' '; // puts a space between level and fmt
  strncat(nfmt + 1, fmt, strlen(fmt));
  fprintf(stderr, "%s", color[level]);
  vfprintf(stderr, nfmt, args);
  if(nom_logger.new_line == ON) {
    fprintf(stderr, "\n");
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

typedef struct {
  void** items;
  unsigned count;
  unsigned capacity;
} Nom_cmd;

bool nom_cmd_append(Nom_cmd* cmd, void* item) {
  if(cmd->count == 0) {
    cmd->capacity = DEFAULT_CAP;
    cmd->items = (void**)calloc(1, sizeof(cmd->items));
    cmd->items[0] = item;
    cmd->count++;
    return true;
  }

  cmd->count += 1;
  cmd->items = (void**)realloc(cmd->items, (cmd->count + 1) * sizeof(cmd->items));
  cmd->items[cmd->count - 1] = item;
  cmd->items[cmd->count] = NULL;

  if(cmd->count + 1 >= cmd->capacity) {
    cmd->capacity *= 2;
    cmd->items = (void**)realloc(cmd->items, cmd->capacity * sizeof(char*));
    if(cmd->items == NULL) {
      nom_log(NOM_PANIC, "could not allocate enough memory for cmd; buy more ram smh");
      return false;
    }
  }
  return true;
}

void nom_cmd_append_many(Nom_cmd* cmd, unsigned count, ...) {
  va_list args;
  va_start(args, count);
  for(int i = 0; i < count; i++) {
    void* arg = va_arg(args, void*);
    nom_cmd_append(cmd, arg);
  }
}

// TODO: actually implement nom_cmd_shrink
Nom_cmd nom_cmd_shrink(Nom_cmd* cmd, long int count, long int ind[]) {
  Nom_cmd new_cmd = {0};
  for(int i = 0; i < cmd->count; i++) {
    for(int i = 0; i < count; i++) {
      if(ind[i] > cmd->count) {
        return new_cmd; // empty cmd
      }
      long int index = ind[i];
      cmd->items[index] = NULL;
      cmd->count -= 1;
    }
    if(cmd->items[i] != NULL) {
      nom_cmd_append(&new_cmd, cmd->items[i]);
    }
  }
  return new_cmd;
}

void nom_cmd_reset(Nom_cmd* cmd) { // for outside use, there isnt much of a use to use it here
  for(int i = 0; cmd->items[i]; i++)
    cmd->items[i] = NULL;
  cmd->count = 0;
}

void nom_log_cmd(enum log_level level, char* msg, Nom_cmd cmd) {
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    nom_log(NOM_WARN, "cmd was empty, could not log it");
    return;
  }
  nom_logger.new_line = OFF;
  nom_log(level, msg);
  nom_logger.show_mode = OFF;
  nom_log(NOM_NONE, " ");
  for(int i = 0; cmd.items[i]; i++) { // no null checks because cmd.items[0] is empty or the cmd.items[cmd.count] is null
    nom_log(NOM_NONE, "%s ", cmd.items[i]);
  }
  nom_log(NOM_NONE, "\n");
  nom_logger_reset();
  return;
}

bool nom_run_path(Nom_cmd cmd) {
  nom_log_cmd(NOM_INFO, "starting", cmd);
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    nom_log(NOM_WARN, "can not run nom_run_path, cmd struct is empty");
    return false;
  }
  int child_status;
  pid_t pid = fork();
  if(pid == 0) {
    execv((char*)cmd.items[0], (char**)cmd.items);
    return false;
  }
  if(pid > 0) {
    if(waitpid(pid, &child_status, 0) < 0) {
      nom_log(NOM_WARN, "could not wait on child:%s", strerror(errno));
      return false;
    }
    if(WEXITSTATUS(child_status) != 0) {
      nom_log_cmd(NOM_WARN, "failed to run", cmd);
      return false;
    }
    if(WEXITSTATUS(child_status) == 0) {
      return true;
    }
    if(WIFSIGNALED(child_status)) {
      nom_log(NOM_WARN, "command process was terminated by %s", strsignal(WTERMSIG(child_status)));
      return false;
    }
  }
  return false;
}

pid_t start_process(Nom_cmd cmd) {
  if(cmd.count == 0 || cmd.items[0] == NULL) {
    nom_log(NOM_WARN, "can not run start_process, cmd struct is empty");
    return 0;
  }
  pid_t pid = fork();
  if(pid == -1) {
    nom_log(NOM_WARN, "fork failed in nom_run_async");
    return 0;
  }
  if(pid == 0) {
    if(execvp((const char*)cmd.items[0], (char* const*)cmd.items) != 0) {
      nom_log(NOM_WARN, "could not execute child process");
      return 0;
    }
  }
  return pid;
}

unsigned int nom_run_async(Nom_cmd cmd) {
  nom_log(NOM_DEBUG, "starting async");
  pid_t pid = start_process(cmd);
  nom_log_cmd(NOM_INFO, "async ran:", cmd);
  return pid;
}

bool nom_run_sync(Nom_cmd cmd) {
  nom_log_cmd(NOM_INFO, "starting", cmd);
  pid_t pid = start_process(cmd);
  int child_status;
  if(pid > 0) {
    if(waitpid(pid, &child_status, 0) < 0) {
      return false;
    }
    if(WEXITSTATUS(child_status) != 0) {
      nom_log(NOM_WARN, "failed to run %s", cmd.items[0]);
      return false;
    }
    if(WEXITSTATUS(child_status) == 0) {
      return true;
    }
    if(WIFSIGNALED(child_status)) {
      nom_log(NOM_WARN, "command process was terminated by %s", strsignal(WTERMSIG(child_status)));
      return false;
    }
  }
  return false;
}

int _strnlen(const char* const str, unsigned int len) { // changing the standard for compilation means strnlen and strsignal arent apart of the standard so i made my own strnlen
  int i;
  int len_null;
  if(len == 0) {
    len += 1;
    len_null = 1;
  }
  for(i = 0; i < len && str[i]; i++) {
    if(len_null == 1) {
      len += i + 1;
    }
    if(i > len)
      return -1;
  }
  return i;
}

char* nom_strrchr(const char* str, const char delim) {
  char* copy = NULL;
  for(int i = 0; *str != '\0'; i++) {
    if(*str == delim) {
      copy = (char*)str;
    }
    str++;
  }
  return copy;
}

char* base(const char* file) {
  if(file == NULL)
    return NULL;
  char* retStr = strdup(file);
  char* lastExt;
  if(retStr[0] == '.' && retStr[1] == '/') {
    retStr += 1;
  }
  lastExt = strrchr(retStr, '.');
  if(lastExt != NULL)
    *lastExt = '\0';
  return retStr;
}

char* base_file(const char* const file, const char delim) {
  if(file == NULL) {
    return NULL;
  }
  char* retstr = strdup(file);
  retstr = strrchr(retstr, delim);
  retstr += 1; // strrchr goes up to last 'delim', increment by one to skip that
  return retstr;
}

bool IS_PATH_EXIST(char* path) {
  if(!path) {
    return false;
  }
  struct stat fi;
  if(stat(path, &fi) < 0) {
    if(errno != ENOENT) {
      nom_log(NOM_WARN, "error: %s %s", path, strerror(errno));
    }
    return false;
  }
  return true;
}

unsigned int IS_PATH_DIR(char* path) {
  if(!path) {
    return false;
  }
  struct stat fi;
  if(stat(path, &fi) < 0) {
    return false;
  }
  if(S_ISDIR(fi.st_mode)) {
    return true;
  }
  return false;
}

unsigned int IS_PATH_FILE(char* path) {
  if(!path) {
    return false;
  }
  struct stat fi;
  if(stat(path, &fi) < 0) {
    return false;
  }
  if(S_ISREG(fi.st_mode)) {
    return true;
  }
  return false;
}

unsigned int mkdir_loop(char* path) {
  if(!path) {
    return false;
  }
  int len = _strnlen(path, 4096);
  if(len <= 0) {
    nom_log(NOM_WARN, "_strnlen returned %d in %s", len, __FUNCTION__);
    return false;
  }
  if(len > 4096) {
    nom_log(NOM_PANIC, "path is longer than %d, exiting", 4096);
    return false;
  }
  if(IS_PATH_EXIST(path)) {
    return true;
  }
  int i = 0;
  if(path[0] == '.' && path[1] == '/') {
    len += 2;
    i += 2;
  }
  char* dir = (char*)calloc(1, len);
  for(; i <= len; i++) {
    dir = strncpy(dir, path, i);
    if(path[i] == '/') {
      if(IS_PATH_EXIST(dir))
        continue;
      if(mkdir(dir, 0755) != 0) {
        nom_log(NOM_WARN, "dir: %s %s", dir, strerror(errno));
        return false;
      }
    }
  }
  if(mkdir(path, 0755) != 0) {
    nom_log(NOM_WARN, "path: %s %s", path, strerror(errno));
    return false;
  }
  return true;
}

bool mkdir_if_not_exist(char* path) {
  if(!path) {
    return false;
  }
  return mkdir_loop(path);
}

bool mkfile_loop(char* path) {
  if(!path)
    return 0;
  struct stat fi;
  unsigned int len = _strnlen(path, 255);
  if(len <= 0) {
    nom_log(NOM_WARN, "_strnlen returned %d in %s", len, __FUNCTION__);
    return false;
  }
  if(IS_PATH_EXIST(path)) {
    return true;
  }
  char* file = (char*)calloc(1, len);
  for(int i = 0; i <= len; i++) {
    if(path[i] == '.' && path[i + 1] == '/') {
      len += 2;
      i += 2;
      file = (char*)realloc(file, len);
    }
    strncpy(file, path, i);
    if(file[i] == '/') {
      if(mkdir(file, 0755) < 0) {
        if(errno != 17) {
          nom_log(NOM_WARN, "error %s:%s", path, strerror(errno));
          return false;
        }
      }
    }
  }
  if(creat(file, 0644) < 0) {
    nom_log(NOM_WARN, "mkfile error:%s %s", file, strerror(errno));
    return false;
  }
  return true;
}

bool mkfile_if_not_exist(char* path) {
  if(!path) {
    return false;
  }
  return mkfile_loop(path);
}

long int nom_get_mtime(char* file) {
  struct stat fi;
  if(stat(file, &fi) < 0) {
    nom_log(NOM_WARN, "%s", strerror(errno));
  }
  return fi.st_mtime;
}

bool nom_set_mtime(char* file) {
  struct utimbuf ntime;
  struct stat fi;
  if(stat(file, &fi) < 0) {
    nom_log(NOM_WARN, "could not stat %s", file);
    return 0;
  }
  ntime.modtime = time(NULL);
  if(utime(file, &ntime) < 0) {
    nom_log(NOM_WARN, "could not update %s's timestamp", file);
    return false;
  }
  return true;
}

long int nom_get_fsize(char* file) {
  struct stat fi;
  if(stat(file, &fi) < 0) {
    nom_log(NOM_WARN, "%s", strerror(errno));
  }
  return fi.st_size;
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

bool needs_rebuild(char* source_file, char* old_file) {
  if(source_file == NULL || old_file == NULL) {
    return false;
  }
  if(!IS_PATH_EXIST(source_file) || !IS_PATH_EXIST(old_file)) {
    return false;
  }
  long int source_time = nom_get_mtime(source_file);
  long int old_time = nom_get_mtime(old_file);
  return source_time > old_time;
}

unsigned int rebuild(int argc, char* argv[], char* file, char* compiler) {
  if(!needs_rebuild(file, base(file))) {
    return 0;
  }
  char* old_path = strcat(base(file), ".old");
  char* bin = base(file);
  bool newf = IS_PATH_EXIST(file);
  Nom_cmd cmd = {0};
  if(file == NULL || compiler == NULL || argc < 1 || newf == false) {
    nom_logger.new_line = OFF;
    nom_log(NOM_PANIC, "failed to rebuild because: ");
    nom_logger.show_mode = OFF;
    nom_logger.new_line = ON;
    if(file == NULL) {
      nom_log(NOM_NONE, "file was null");
      return 0;
    }
    if(compiler == NULL) {
      nom_log(NOM_NONE, "compiler was null");
      return 0;
    }
    if(argc < 1) { // apparently argc can be less than 0 if you run it using certain exec function
      nom_log(NOM_NONE, "argc was %d instead of >= 1", argc);
      return 0;
    }
    if(!newf) {
      nom_log(NOM_WARN, "%s does not exist", file);
      return 0;
    }
    nom_logger_reset();
  }
  int oldf = IS_PATH_EXIST(old_path);
  if(!oldf) {
    nom_log(NOM_WARN, "no %s, no roll back in case of failure", old_path);
  }
  nom_log(NOM_INFO, "starting rebuild");
  rename(bin, old_path);
  nom_log(NOM_DEBUG, "renamed %s to %s", bin, old_path);
  nom_logger.new_line = OFF;
  nom_log(NOM_DEBUG, "argv: ");
  if(nom_logger.show_debug == ON) {
    for(int i = 0; i < argc; i++) {
      nom_log(NOM_NONE, "%s", argv[i]);
    }
    nom_log(NOM_NONE, "\n");
  }
  nom_logger_reset();
  nom_cmd_append_many(&cmd, 5, compiler, "-ggdb", file, "-o", bin);
  if(!nom_run_sync(cmd)) {
    nom_log_cmd(NOM_WARN, "failed to run:", cmd);
  }
  nom_log_cmd(NOM_INFO, "rebuild ran:", cmd);
  int binf = IS_PATH_EXIST(bin);
  if(!binf) { // checks if bin was removed by the compiler, this happens on some compiler errors
    nom_log(NOM_WARN, "%s did not exist, renaming %s to %s", bin, old_path, bin);
    rename(old_path, bin);
  }
  Nom_cmd run = {0};
  nom_cmd_append(&run, base(file));
  for(int i = 0; i < argc; i++) {
    nom_cmd_append(&run, argv[i]);
  }
  if(nom_run_path(run)) {
    nom_log(NOM_INFO, "ending rebuild");
    _exit(0);
  }
  nom_logger.new_line = OFF;
  nom_logger_reset();
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

unsigned int rebuild1(char* file, char* compiler) {
  if(!file || !compiler)
    return 0;
  char* argv[] = {file, NULL};
  unsigned ret = rebuild(1, argv, file, compiler);
  return ret;
}

int IS_LIBRARY_MODIFIED(char* lib, char* file, char* compiler) {
  struct stat fi;
  int libf, filef;
  libf = IS_PATH_EXIST(lib);
  filef = IS_PATH_EXIST(file);
  if(lib == NULL || file == NULL || compiler == NULL || libf == false || filef == false) {
    nom_logger.new_line = OFF;
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
    if(!libf) {
      nom_log(NOM_NONE, "lib:%s does not exist", lib);
    }
    if(!filef) {
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
    nom_set_mtime(file);
  }
  Nom_cmd run = {0};
  nom_cmd_append(&run, base(file));
  if(nom_run_path(run)) {
    nom_log(NOM_DEBUG, "ending IS_LIBRARY_MODIFIED");
    _exit(0);
  }
  nom_log(NOM_WARN, "IS_LIBRARY_MODIFIED failed");
  _exit(1);
}

void* map_file_into_memory(char* file) {
  if(!IS_PATH_EXIST(file)) {
    return NULL;
  }
  int fd = open(file, O_RDWR);
  if(fd < 0) {
    return NULL;
  }
  void* mem = NULL;
  struct stat fi;
  if(stat(file, &fi) < 0) {
    return NULL;
  }
  return mmap(mem, fi.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
}

void unmap_file_from_memory(void* ptr, long int size) {
  munmap(ptr, size);
}
