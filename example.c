#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char** argv){
GO_REBUILD(__FILE__, argv);
compile_all(".", "tcc", ".c", "bin");
compile_all("tests", "tcc",".c", "tests");
printf("hello world\n");
return 0;
}
