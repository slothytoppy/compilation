#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char** argv){
IS_FILE_FILE("example.c");
GO_REBUILD(__FILE__, argv);
compile_all("tests", "tcc",".c", "tests");
printf("hello hello\n");
return 0;
}
