#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char** argv){
// MKDIR("hello");
// compile_all(".", "gcc", ".c", "bin");
char* flags[]={"-Wextra", "-Wall", NULL};
compile_all(__FILE__, ".", "tcc", NULL, ".c", "tests");
compile_all(__FILE__, "tests", "tcc", flags,".c", "tests");
return 0;
}
