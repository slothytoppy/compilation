#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(void){
MKDIR("hello");
compile_all(".", "gcc", ".c");
compile_all("examples", "gcc", ".c");
return 0;
}
