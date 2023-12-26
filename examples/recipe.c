#define COMPILATION_IMPLEMENTATION
// #define DEBUG //if DEBUG is defined then debug info will be printed from compile commands, more functions will have DEBUG support soon
#include "comp.h"

int main(int argc, char** argv){
GO_REBUILD(argc, argv);
if(IS_PATH_DIR("bin")){
printf("yes\n");
}
compile_dir(".", "bin","tcc", ".c");
compile_dir("tests", ".", "tcc",".c");
printf("hello yes!\n");
char* command[]={"test", "build", NULL};
run_args(command);
return 0;
}
