#define COMPILATION_IMPLEMENTATION
#define DEBUG
#include "../nomake.h"

int main(void){
MKDIR("hello");
if(IS_PATH_EXIST("hello")){
	printf("it does exist\n");
} else{
printf("it doesnt exist\n");
}
MKFILE("bye.c");
write_basic_c_file("bye.c");
char* command[]={"gcc", "bye.c", "-o", "bye", NULL}; 
exec(command);
// compile_file("bye.c", "bye", "gcc", ".c"); // this is an alternative to using exec for compiling files, there are other versions of this like compile_targets for an array of files and compile_dir 
return 0;
}
