#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(void){
// MKDIR("hello");
// compile_all(".", "gcc", ".c", "bin");
if(is_path1_modified_after_path2("example.c", "bin/example")){
  printf("YES\n");
}

compile_all("tests", "tcc", ".c", "tests");
return 0;
}
