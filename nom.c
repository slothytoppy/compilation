#define NOM_IMPLEMENTATION
#include "nom.h"

int main(){
// rebuild(__FILE__, "gcc");

Nom_cmd cmd={0};
nom_cmd_append_many(&cmd, 6, "gcc", "-g", "nom.c", "-o", "nom");
nom_run_async(cmd);

Dyn_arr dyn={0};
dyn_init(&dyn, sizeof(int*));
int item=5;
dyn_arr_append(&dyn, (int*)item);
dyn_arr_append(&dyn, (int*)item);
item=3;
dyn_arr_append(&dyn, (int*)item);
printf("%d %d %d\n", dyn.items[0], dyn.items[1], dyn.items[2]);
}
