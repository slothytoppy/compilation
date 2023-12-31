#define COMPILATION_IMPLEMENTATION // this defines the implementation of the functions i've made, by default all you get is the name of the functions
#define DEBUG // this defines debug printing for most functions
#include "../nomake.h" // based on your path to nomake it may be #include "nomake.h"

int main(int argc, char* argv[]){
char filename[]="test";
GO_REBUILD(argc, argv, "cc"); // if this file is compiled then you modify this file afterward all you have to do is ./filename and it will rebuild itself
if(!IS_PATH_EXIST(filename)){ // checks if path exists, it works anything that can be used in stat
printf("%s exists", filename);
} else{
	MKDIR(filename);
}
}
