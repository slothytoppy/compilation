#define COMPILATION_IMPLEMENTATION
#include "../nomake.h"
#include <fcntl.h>

int main(){
MKFILE("comp.c");
int fd=open("comp.c", O_WRONLY);
write(fd, "#include <stdio.h>\n", strlen("#include <stdio.h>\n"));
write(fd,"\n", 1);
write(fd, "int main(){\n", strlen("int main(){\n"));
write(fd, "  printf(\"hello world\\n\");\n", strlen("  printf(\"hello world\\n\");\n"));
write(fd, "}", strlen("}"));
close(fd);
char* command[]={"cc", "comp.c", "-o", "comp", NULL};
exec(command);
return 0;
}
