#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char** argv){
if(IS_PATH_FILE("img")){
printf("example.c is a file\n");
}
GO_REBUILD(__FILE__, argv);
compile_all(".", "tcc", ".c", "bin");
compile_all("tests", "tcc",".c", "tests");
printf("hello goodbye\n");
return 0;
}
