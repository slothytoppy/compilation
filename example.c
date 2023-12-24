#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char** argv){
// MKDIR("hello");
// compile_all(".", "gcc", ".c", "bin");
GO_REBUILD(__FILE__, argv);
compile_all(".", "tcc", ".c", "tests");
compile_all("tests", "tcc",".c", "tests");
compile_all(".", "tcc", ".", "bin");
printf("hello sailor\n");
return 0;
}
