#define COMPILATION_IMPLEMENTATION
#define DEBUG

#include "../nomake.h"

int main(void){
char* file="nomake.h";
if(strcmp(base(file), "nomake")==0){
printf("%s matches nomake\n", file);
} else{
printf("%s doesnt match nomake\n", file);
return 1;
  }
if(strcmp(ext(file), ".c")==0){
printf("%s matches .c\n", file);
} else{
printf("%s doesnt match .c\n", file);
  }
}
