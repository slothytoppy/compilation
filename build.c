#define COMPILATION_IMPLEMENTATION
#define DEBUG
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv, "tcc");
char* str2=calloc(1, PATH_MAX);
printf("upper cwd:%s\n", uppcwd("../bin", str2));
free(str2);
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
