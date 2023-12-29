#define COMPILATION_IMPLEMENTATION
#define DEBUG
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv);
if(IS_PATH_EXIST("bin")){
compile_file("examples/recipe.c", "bin/recipe", "cc", ".c"); 
} else{
MKDIR("bin");
}
printf("hello sailor\n");
}

