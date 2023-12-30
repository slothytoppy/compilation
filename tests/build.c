#define COMPILATION_IMPLEMENTATION
#define DEBUG
#include "../nomake.h"

int main(int argc, char* argv[]){
  if(argc>=2 && strcmp(argv[1], "clean")==0){
CLEAN(".", ".c");
  }
  else{
  print_source();
GO_REBUILD(argc, argv, "tcc");
compile_dir(".", ".", "cc", ".c"); 
  }
return 0;
}
