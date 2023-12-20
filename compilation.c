#define COMPILATION_IMPLEMENTATION
#include "comp.h"
int main(int argc, char* argv[]){
struct stat fi;
if(argc==1){ 
printf("not enough args\n"); 
return -1;
  }
 if(stat(argv[1], &fi)!=0){
  printf("file: %s doesnt exist\n", argv[1]);
  return -1;
}
char* output=base(argv[1]);
// char* command[]={"gcc", "-Wall", dirent->d_name, "-o" , output, NULL};
// printf("compiled:%s output is:%s\n", argv[1], output);
// exec(command);
stat(output, &fi);
printf("output size:%ld\n", fi.st_size);
printf("mode:%o\n", fi.st_mode);
//printf("%s\n", base("compilation.c"));
// printf("%s\n", ext("compilation.c"));
compile_all("gend.c");
//char* command[]={"nvim", "compilation.c", NULL};
//if(strcmp(ext("gend.c"), ".c")==0){
//  printf("ext:%s\n", ext("gend.c"));
// }
//exec(command);
compile_all(".");
return 0;
}
