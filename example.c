#define COMPILATION_IMPLEMENTATION
#include "comp.h"
int main(int argc, char* argv[]){
compile_all(".", "gcc", ".c");
return 0;
}
