#define COMPILATION_IMPLEMENTATION
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv, "tcc");
if(IS_PATH_EXIST("bin")){
compile_file("examples/recipe.c", "bin/recipe", "cc", ".c"); 
} else{
MKDIR("bin");
}
printf("cdaba\n");
}

