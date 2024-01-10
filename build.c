#define COMPILATION_IMPLEMENTATION
// #define DEBUG
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
printf("%d\n", PATH_MAX);
char* flags[]={"-Wpedantic"};
unsigned int sz=sizeof(flags)/sizeof(flags[0]);
// compile("gcc", flags, ".", ".", ".c", sz);
// compile("gcc", flags, "tests", "tests", ".c", sz);
// compile("gcc", flags, "examples", "examples", ".c", sz);
int i;
char* ex[]={".c", ".rs", ".f99",".c", 0};
int asz=sizeof(ex)/sizeof(ex[0]);
for(i=0; ex[i]!=NULL; i++){
if(strcmp(ext(ex[i]), ".c")==0) printf("%d\n", i);
}
}
