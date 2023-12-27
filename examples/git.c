#define COMPILATION_IMPLEMENTATION
#include "../nomake.h"

int main(){
char* command[]={"git", "commit", "-a", "--dry-run", NULL};
exec(command);
}
