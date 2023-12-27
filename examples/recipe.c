#define COMPILATION_IMPLEMENTATION
// #define DEBUG //if DEBUG is defined then debug info will be printed from compile commands, more functions will have DEBUG support soon
#include "../nomake.h"

int main(int argc, char** argv){
compile_dir(".", "bin","tcc", ".c");
compile_dir("examples", ".", "tcc", ".c");
compile_dir("tests", "bin", "tcc",".c");
return 0;
}
