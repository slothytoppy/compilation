#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char** argv){
// GO_REBUILD(__FILE__, argv);
compile_dir(".", "bin","tcc", ".c");
compile_dir("tests", ".", "tcc",".c");
printf("hello world!\n");
return 0;
}
