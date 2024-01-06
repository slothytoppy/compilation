#define COMPILATION_IMPLEMENTATION
#define DEBUG
#include "../nomake.h"

int main(){
  if(MKDIR("test")<=0){
  fprintf(stderr, "error creating directory:%s\n", "test");
  return 1;
  }
  if(RMDIR("test")<=0){
  fprintf(stderr, "error removing directory:%s\n", "test");
  return 1;
  }
  return 0;
}
