#define NOM_IMPLEMENTATION 
#include "beta.h"

int main(int argc, char* argv[]){
rebuild(__FILE__, "gcc");
// if(IS_LIBRARY_MODIFIED("nom.h", __FILE__, "gcc")) printf("hello world\n");
Nom_cmd cmd={0};
nom_cmd_append_many(&cmd, 4, "gcc", "-o", base(__FILE__), __FILE__);
nom_run_sync(cmd);
}
