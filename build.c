#define COMPILATION_IMPLEMENTATION
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv);
if(IS_PATH_EXIST("bin")){
compile_file("examples/recipe.c", "bin/recipe", "tcc", ".c"); 
} else{
MKDIR("bin");
}
/*
if(IS_PATH_EXIST("examples")){
char* command[]={"cc", "examples/test.c", "-o", "examples/test", NULL};
exec(command);
print_exec(command);
} else{
MKDIR("examples");
MKFILE("test.c");
char* command[]={"cc", "examples/test.c", "-o", "examples/test", NULL};
exec(command);
print_exec(command);
}
*/
/*
if(!IS_PATH_EXIST("exec")){
MKDIR("exec");
} else{
printf("exec already exists\n");
}
*/
printf("hello sailor\n");
}

