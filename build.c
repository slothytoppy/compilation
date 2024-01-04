#define COMPILATION_IMPLEMENTATION
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv, "tcc");
char* cwd=calloc(1, PATH_MAX);
parse("../build"); 
// parse_path_dots("..", "build/", cwd);
if(IS_PATH_EXIST("bin")){
compile_file("recipe.c", "bin/recipe", "gcc", ".c"); 
} else{
MKDIR("bin");
}
if(ends_with("aaa/", '/')){
printf("IT DOES\n");
}
}
