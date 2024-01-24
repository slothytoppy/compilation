#define NOMAKE_IMPLEMENTATION // needed for getting the implementation of the functions
// #define DEBUG // old feature, no longer used in favor of NOM_LOG
#include "nomake.h"

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv, "tcc"); // this makes it such that build.c will rebuild itself if it is changed, see GO_REBUILD at the bottom of nomake.h for more
print_source(); // this prints out __FILE__, kinda useless but its there
char* str2=calloc(1, PATH_MAX); // upcwd can return a malloc'd string for later use, put null there if you're not going to use it
upcwd("../bin", str2);
printf("upper cwd:%s\n", upcwd("bin", NULL));
printf("%s\n", str2);
printf("%d\n", PATH_MAX);
free(str2);
char* flags[]={"-Wpedantic"}; // where flags is can be null, if it is then it wont compile with flags, you also have to give it the amount of flags for it to use all of them
unsigned int sz=sizeof(flags)/sizeof(flags[0]);
compile("gcc", flags, ".", "bin", ".c", sz); 
/* all of my compile functions should go something like this, compiler, origin(directory/file) directory if its compile or compile_dir else its a file, destination, 
same as origin but if its compile or compile_dir it will put that in the destination directory else its a file, you could probably do something like ../filename inside of a function 
thats not compile or compile_dir and it should work, 5th argument is the extension ie what file extension it checks for and in most cases it will get the 
base(filename with no ext) of the file and make that the executable name, the last one is the amount of flags, if you dont use them it wont error this is because its a variadic argument(optional argument)
*/
compile("gcc", flags, "tests", "bin", ".c", sz);
compile("gcc", flags, "examples", "bin", ".c", sz);
}
