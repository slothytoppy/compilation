#define COMPILATION_IMPLEMENTATION

#include "../nomake.h"

int write_file(char* file){
MKFILE(file);
int fd=open(file, O_WRONLY);
write(fd, "#include <stdio.h>\n", strlen("#include <stdio.h>\n"));
write(fd,"\n", 1);
write(fd, "int main(){\n", strlen("int main(){\n"));
write(fd, "  printf(\"hello world\\n\");\n", strlen("  printf(\"hello world\\n\");\n"));
write(fd, "}", strlen("}"));
close(fd);
return 0;
}

int main(){
char* files[]={"compile_test1.c", "compile_test2.c", "compile_test3.c", "compile_test4.c", NULL};
write_file("compile_test1.c");
write_file("compile_test2.c");
write_file("compile_test3.c");
write_file("compile_test4.c");
compile_targets(files, base(*files), "cc", ".c"); 
int i;
int sz=sizeof(files)/sizeof(files[0]);
for(i=0; i<sz; i++){
RMFILE(files[i]);
RMFILE(base(files[i]));
}
return 0;
}
