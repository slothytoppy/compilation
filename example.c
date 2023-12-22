#define COMPILATION_IMPLEMENTATION
#include "comp.h"
int main(int argc, char* argv[]){
char* targets[]={"test.c", "example.c", NULL};
compile_targets(targets, "gcc", ".c");
return 0;
}
