#define NOM_IMPLEMENTATION
#include "beta.h"

int main() {
  rebuild(__FILE__, "gcc");
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 5, "gcc", "-g", "nom.c", "-o", "nom");
  nom_run_async(cmd);
  Dyn_arr dyn = {0};
  dyn_init(&dyn, sizeof(int));
  intptr_t item = 5;
  dyn_arr_append(&dyn, (int*)item);
  item = 3;
  dyn_arr_append(&dyn, (int*)item);
  item = 8;
  dyn_arr_append(&dyn, (int*)item);
  for(int i = 0; i < dyn.count; i++) {
    printf("%d\n", *((int*)dyn.items + sizeof(int) / 2 * i));
  }
}
