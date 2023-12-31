#define COMPILATION_IMPLEMENTATION
#include "../nomake.h"

int main(int argc, char** argv){
compile_dir(".", "bin/","cc", ".c");
compile_dir("examples/", ".", "cc", ".c");
compile_dir(".", ".", "cc", ".c");
compile_dir("tests", "bin", "cc",".c");
return 0;
}
