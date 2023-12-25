#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char** argv){
GO_REBUILD(__FILE__, argv);
if(IS_PATH_FILE("example.c")){
printf("example.c is a file\n");
} 
if(IS_PATH_EXIST("example.c")){
printf("YES\n");
} 
compile_all(".", "tcc", ".c", "bin");
compile_all("tests", "tcc",".c", "tests");
printf("hello world\n");
return 0;
}
