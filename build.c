#define COMPILATION_IMPLEMENTATION
// #define DEBUG
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv, "tcc");
print_source();
char* str2=calloc(1, PATH_MAX);
upcwd("../bin", str2);
printf("upper cwd:%s\n", upcwd("bin", NULL));
printf("%s\n", str2);
printf("%d\n", PATH_MAX);
free(str2);
char* flags[]={"-Wpedantic"};
unsigned int sz=sizeof(flags)/sizeof(flags[0]);
compile("gcc", flags, ".", "bin", ".c", sz);
compile("gcc", flags, "tests", "bin", ".c", sz);
compile("gcc", flags, "examples", "bin", ".c", sz);
}
