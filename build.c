#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv);
if(IS_PATH_EXIST("bin")){
char* command[]={"cc", "example.c", "-o", "bin/example", NULL};
exec(command);
print_exec(command);
} else{
MKDIR("bin");
}
if(IS_PATH_EXIST("examples")){
char* command[]={"cc", "test.c", "-o", "examples/test", NULL};
exec(command);
print_exec(command);
} else{
MKDIR("examples");
MKFILE("test.c");
char* command[]={"cc", "test.c", "-o", "examples/test", NULL};
exec(command);
print_exec(command);
}
if(!IS_PATH_EXIST("bin")){
printf("bin doesnt exist\n");
}
if(!IS_PATH_EXIST("exec")){
MKDIR("exec");
} else{
printf("exec already exists\n");
}
}
