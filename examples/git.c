#define COMPILATION_IMPLEMENTATION
#include "comp.h"

int main(){
char* command[]={"git", "commit", "-a", "--dry-run", NULL};
exec(command);
}
