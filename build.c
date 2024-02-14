#define NOM_IMPLEMENTATION
#include "beta.h"

int main(int argc, char* argv[]) {
  rebuild(__FILE__, "gcc");
  Nom_cmd run = {0};
  nom_cmd_append(&run, "nom");
  // returns 1 on failure i guess
  if(!nom_run_path(run, NULL)) {
    exit(1);
  }
  //  if (IS_FILE_MODIFIED("nom.c"))
  // printf("hello\n");
  // printf("%d\n", x);
  // printf("hello");
  /* if(IS_LIBRARY_MODIFIED("nom.h", __FILE__, "gcc")) printf("hello world\n");
  Nom_cmd cmd={0};
  nom_cmd_append_many(&cmd, 4, "gcc", "-o", base(__FILE__), __FILE__);
  nom_run_sync(cmd);
  */
  /*
  unsigned fd=inot_setup(NULL);
  unsigned wd=nom_add_watch(fd, ".", IN_MODIFY | IN_CREATE | IN_MOVE);
  char* bin_path="./build";
  char* args[]={NULL};
  nom_read_inot(fd, bin_path, NULL);
  // printf("%d\n", x);
  */
}
