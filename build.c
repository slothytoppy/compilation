#define COMPILATION_IMPLEMENTATION
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv, "tcc");
compile_dir(".", "../bin", "tcc", ".c");
if(IS_PATH_EXIST("bin")){
compile_file("recipe.c", "bin/recipe", "gcc", ".c"); 
} else{
MKDIR("bin");
}
if(ends_with("aaa/", '/')){
printf("IT DOES\n");
}
}
