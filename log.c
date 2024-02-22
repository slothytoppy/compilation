#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

enum log_level {
  NOM_INFO,
  NOM_WARN,
  NOM_PANIC,
  NOM_DEBUG,
  NOM_NONE,
};

#define ON 0
#define OFF 1

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
  nom_logger.colors.debug_color = "\033[38;5;241m[DEBUG]\033[0m";
  nom_logger.colors.info_color = "\033[38;5;208m[INFO]\033[0m";
  nom_logger.colors.warn_color = "\033[38;5;1m[WARN]\033[0m";
  nom_logger.colors.panic_color = "\033[38;5;196m[PANIC]\033[0m";
}

void nom_logger_reset(void) {
  nom_logger.lines = ON;
  nom_logger.show_mode = ON;
  nom_logger.show_debug = ON;
}

void nom_logger_reset_all() {
  nom_logger.colors.debug_color = "\033[38;5;241m[DEBUG]\033[0m";
  nom_logger.colors.info_color = "\033[38;5;208m[INFO]\033[0m";
  nom_logger.colors.warn_color = "\033[38;5;1m[WARN]\033[0m";
  nom_logger.colors.panic_color = "\033[38;5;196m[PANIC]\033[0m";
  nom_logger.lines = ON;
  nom_logger.show_mode = ON;
  nom_logger.show_debug = ON;
}
void nom_log(enum log_level level, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if(nom_logger.colors.debug_color == NULL) {
    nom_logger_reset_colors();
  } else if(nom_logger.colors.info_color == NULL) {
    nom_logger_reset_colors();
  } else if(nom_logger.colors.warn_color == NULL) {
    nom_logger_reset_colors();
  } else if(nom_logger.colors.panic_color == NULL) {
    nom_logger_reset_colors();
  }
  if(nom_logger.show_mode == OFF) {
    level = NOM_NONE;
  }
  if(level == NOM_DEBUG && nom_logger.show_debug == ON) {
    fprintf(stderr, "%s ", nom_logger.colors.debug_color);
  } else if(level == NOM_INFO) {
    fprintf(stderr, "%s ", nom_logger.colors.info_color);
  } else if(level == NOM_WARN) {
    fprintf(stderr, "%s ", nom_logger.colors.warn_color);
  } else if(level == NOM_PANIC) {
    fprintf(stderr, "%s ", nom_logger.colors.panic_color);
  }
  vfprintf(stderr, fmt, args);
  va_end(args);
  if(nom_logger.lines == ON) {
    printf("\n");
  }
  return;
}

void nom_file_log(enum log_level level, char* fin, const char* fmt, ...) {
  nom_logger.finfo.fout = fin;
  FILE* fout = fopen(fin, "a+");
  time_t rawtime;
  struct tm* timeinfo;
  char buffer[20];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d-%M-%S", timeinfo);
  fprintf(fout, "%s ", buffer);
  va_list args;
  va_start(args, fmt);

  if(nom_logger.colors.debug_color == NULL) {
    nom_logger_reset_colors();
  } else if(nom_logger.colors.info_color == NULL) {
    nom_logger_reset_colors();
  } else if(nom_logger.colors.warn_color == NULL) {
    nom_logger_reset_colors();
  } else if(nom_logger.colors.panic_color == NULL) {
    nom_logger_reset_colors();
  }
  if(nom_logger.show_mode == OFF) {
    level = NOM_NONE;
  }
  if(level == NOM_DEBUG && nom_logger.show_debug == ON) {
    fprintf(fout, "%s ", nom_logger.colors.debug_color);
  } else if(level == NOM_INFO) {
    fprintf(fout, "%s ", nom_logger.colors.info_color);
  } else if(level == NOM_WARN) {
    fprintf(fout, "%s ", nom_logger.colors.warn_color);
  } else if(level == NOM_PANIC) {
    fprintf(fout, "%s ", nom_logger.colors.panic_color);
  }
  vfprintf(fout, fmt, args);
  va_end(args);
  if(nom_logger.lines == ON) {
    fprintf(fout, "\n");
  }
  fclose(fout);
  return;
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

int main() {
  nom_log(NOM_WARN, "hello");
  nom_file_log(NOM_INFO, "logger", "hello asdkjsadskl %s", "HELLO");
}
