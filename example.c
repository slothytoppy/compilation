#define COMPILATION_IMPLEMENTATION
#include "comp.h"
#include <fcntl.h>

int isDirectory(const char *path) {
struct stat statbuf;
  if(stat(path, &statbuf) != 0)
    return 0;
return S_ISDIR(statbuf.st_mode);
  }

int main(int argc, char* argv[]){
compile_all(".", "gcc", ".c");
return 0;
}
