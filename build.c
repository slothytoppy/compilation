#define COMPILATION_IMPLEMENTATION
#define DEBUG
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv, "tcc");
	/*
	char* str2=calloc(1, PATH_MAX);
	upcwd("../bin", str2);
	run("bin/mkdir_test");
	printf("%s\n", str2);
printf("upper cwd:%s\n", upcwd("bin", NULL));
free(str2);
*/
char* flags[]={"-Wpedantic"};
compile("gcc", flags, ".", "bin", ".c", 1);
compile("gcc", flags, "tests", "bin", ".c", 1);
compile("gcc", flags, "examples", "../bin", ".c", 1);
}
