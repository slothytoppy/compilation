#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(void){
// MKDIR("hello");
// compile_all(".", "gcc", ".c", "bin");
compile_all("examples", "gcc", ".c", "examples");
return 0;
}
