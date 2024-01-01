#define COMPILATION_IMPLEMENTATION
#define DEBUG
#include "nomake.h"

int main(){
compile_dir(".", "bin/", "gcc", ".c");
return 0;
}
