#define NOM_IMPLEMENTATION
#include "nom.h"

int main(){
Nom_cmd cmd={0};
nom_cmd_append(&cmd, "gcc");
nom_cmd_append(&cmd, "nom.c");
nom_cmd_append(&cmd, "-o");
nom_cmd_append(&cmd, "nom");
nom_cmd_append_null(&cmd);
nom_run_async(cmd);
}
