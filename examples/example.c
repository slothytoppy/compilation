#define COMPILATION_IMPLEMENTATION
#include "nomake.h"

int main(void){
MKDIR("hello");
compile_all(".", "gcc", ".c");
compile_all("examples", "gcc", ".c");
return 0;
}
