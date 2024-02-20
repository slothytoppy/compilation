#define DEBUG
#define NOM_IMPLEMENTATION
#include "../nom.h"

int main() {
  // rebuild works by comparing the last modified time of __FILE__ to base(file), if __FILE__ was modified after base(file) it recompiles it
  rebuild(__FILE__, "gcc");
  // IS_LIBRARY_MODIFIED works like rebuild except it compares the last modified time of lib(the first argument) to file, i made it because i would modify nom.h and got tired of having to rebuild
  // it myself
  //  IS_LIBRARY_MODIFIED("../nom.h", __FILE__, "gcc");
  // returns 1 on success, if(IS_PATH_DIR) would execute what is in the if block if true
  IS_PATH_DIR(".");
  // like IS_PATH_DIR except for files
  IS_PATH_FILE("ex1.c");
  // like the above functions except it returns true if the first argument exists in the filesystem
  IS_PATH_EXIST("ex1.c");
  char* info = "hello";
  // my logger by default appends a new line after it finishes printing
  nom_log(NOM_INFO, "%s", info);
  nom_log(NOM_WARN, "%s", info);
  nom_log(NOM_PANIC, "%s", info);
  nom_log(NOM_DEBUG, "%s", info);
  // NOM_NO_NEWLINE_DEBUG prints out [DEBUG] without creating a new line
  nom_log(NOM_NO_NEWLINE_DEBUG, "%s\n", info);
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 2, "echo", "--colors=true");
  if(nom_run_sync(cmd)) {
    nom_log(NOM_INFO, "successfully executed %s %s", cmd.items[0], cmd.items[1]);
  }
}
