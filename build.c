#define NOM_IMPLEMENTATION 
#include "nom.h"

int main(int argc, char* argv[]){
rebuild(__FILE__, "gcc");
if(IS_LIBRARY_MODIFIED("nom.h", __FILE__, "gcc")) printf("hello world\n");

}
