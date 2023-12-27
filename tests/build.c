#define COMPILATION_IMPLEMENTATION
#include "../nomake.h"

int main(int argc, char* argv[]){
print_source();
GO_REBUILD(argc, argv);
compile_dir(".", ".", "tcc", ".c");
return 0;
}
