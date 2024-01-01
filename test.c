#define COMPILATION_IMPLEMENTATION
#include "nomake.h"

int main(int argc, char* argv[]){
const char const *str="hello";
if(argc==1) return -1;
// struct stat fi;
// stat(argv[1], &fi);
// printf("size: %ld\n", fi.st_size);
char* path = getenv("MAIL");
printf("%s\n", path);
return 0;
}
