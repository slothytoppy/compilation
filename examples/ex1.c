#define DEBUG
#define NOM_IMPLEMENTATION
#include "../nom.h"

int main() {
  // rebuild works by comparing the last modified time of __FILE__ to base(file), if __FILE__ was modified after base(file) it recompiles it
  rebuild1(__FILE__, "gcc");
  // IS_LIBRARY_MODIFIED works like rebuild except it compares the last modified time of lib(the first argument) to file, i made it because i would modify nom.h and got tired of having to rebuild
  // it myself
  // IS_LIBRARY_MODIFIED("../nom.h", __FILE__, "gcc");
  // returns 1 on success, if(IS_PATH_DIR) would execute what is in the if block if true
  //  IS_PATH_DIR(".");
  //  IS_PATH_FILE("ex1.c");
  //  IS_PATH_EXIST("ex1.c");
  //  IS_PATH_MODIFIED("ex1.c");
  //  mkfile_if_not_exist("hello.c");
  //  mkdir_if_not_exist("hello");
  // sets modified time of file
  //  set_mtime("hello.c");
  // like IS_PATH_MODIFIED but it returns true if the first argument is modified after the second
  //  needs_rebuild("hello.c", "ex1.c");
  // like needs_rebuild but for an array of files
  // prints everything inside of cmd can take an extra argument that
  // tells it whether to print [INFO] and a newline or not
  // char* info = "hello";
  // my logger by default appends a new line after it finishes printing
  /*
  nom_log(NOM_INFO, "%s", info);
  nom_log(NOM_WARN, "%s", info);
  nom_log(NOM_PANIC, "%s", info);
  nom_log(NOM_DEBUG, "%s", info);
  */
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 3, "eza", "--color=always", "--icons=always");
  if(nom_run_sync(cmd)) {
    nom_log_cmd(NOM_INFO, "successfully executed:", cmd);
  }
}
