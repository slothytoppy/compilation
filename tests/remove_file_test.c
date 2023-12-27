#define COMPILATION_IMPLEMENTATION
#define DEBUG

#include "../nomake.h"

int main(){
char* file="file_to_be_removed";
  if(!IS_PATH_EXIST(file)){
MKFILE(file);
RMFILE(file);
  }
  if(IS_PATH_EXIST(file)){
RMFILE(file);
  }
  return 0;
}
