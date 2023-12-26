#define COMPILATION_IMPLEMENTATION
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
run(command);
return 0;
}
