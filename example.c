#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char** argv){
// MKDIR("hello");
// compile_all(".", "gcc", ".c", "bin");
char* flags[]={"-Wextra", "-Wall", NULL};
if(is_path1_modified_after_path2(__FILE__, argv[0])){
compile_all(".", "tcc", NULL, ".c", "tests");
compile_all("tests", "tcc", NULL,".c", "bin");
}
printf("hella\n");
return 0;
}
