#define NOMAKE_IMPLEMENTATION 
#include "nomake.h"

int main(int argc, char* argv[]){
rebuild(__FILE__, "gcc");
if(IS_LIBRARY_MODIFIED("nomake.h", __FILE__, "gcc")) printf("hello world\n");

}
