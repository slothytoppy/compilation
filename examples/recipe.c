#define COMPILATION_IMPLEMENTATION
#include "../nomake.h"

int main(int argc, char** argv){
compile_dir(".", "bin","tcc", ".c");
compile_dir("examples", ".", "tcc", ".c");
compile_dir(".", ".", "tcc", ".c");
compile_dir("tests", "bin", "tcc",".c");
return 0;
}
