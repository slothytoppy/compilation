#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(int argc, char* argv[]){
if(argc==1) return -1;
struct stat fi;
stat(argv[1], &fi);
printf("size: %ld", fi.st_size);
return 0;
}
