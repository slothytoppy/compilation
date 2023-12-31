#define COMPILATION_IMPLEMENTATION // this defines the implementation of the functions i've made, by default all you get is the name of the functions
// #define DEBUG // this defines debug printing for most functions
#include "../nomake.h" 

int write_file(char* file){
MKFILE(file);
int fd=open(file, O_WRONLY);
write(fd, "#include <stdio.h>\n", strlen("#include <stdio.h>\n"));
write(fd,"\n", 1);
write(fd, "int main(int argc, char* argv[]){\n", strlen("int main(int argc, char* argv[]){\n"));
write(fd, "  if(argc>1){\n", strlen("  if(argc>1){\n"));
write(fd, "  printf(\"argv1:%s\\n\", argv[1]);\n", strlen("  printf(\"argv1:%s\\n\", argv[1]);\n"));
write(fd, "  }\n", len("  }\n"));
write(fd, "  printf(\"hello world\\n\");\n", strlen("  printf(\"hello world\\n\");\n"));
write(fd, "}\n", 2);
close(fd);
return 0;
}

int main(int argc, char* argv[]){
GO_REBUILD(argc, argv, "cc"); // if this file is compiled then you modify this file afterward all you have to do is ./filename and it will rebuild itself
char filename[]="test";
if(!IS_PATH_EXIST(filename)){ // checks if path exists, it works anything that can be used in stat
printf("%s exists\n", filename);
} else{
	MKDIR(filename);
}
char* files[]={"main.c", "gend.c"};
int i;
/*
for(i=0; i<2; i++){
write_file(files[i]);
}
*/
compile_dir(".", ".", "cc", ".c"); // the arguments to this are origin directory, target directory or where the binaries go, the compiler you want to use then the extension you want the function to look for
// compile_targets(2, files, ".", "cc", ".c"); // first argument is size, second is the array, third argument is destination, 4th is compiler then the last is the extension 
// compile_file("main.c", base("main.c"), "cc", ".c"); // base gets the filename without anything past the dot so if you have base("main.c") it will turn that into "main"
	if(strcmp(ext("main.c"), ".c")==0){
	printf("main.c ends with .c\n");
	}
// char* commands[]={"main", NULL};
// exec(commands);
// char* args[]={"main", "-a", NULL};
// run_args(args);
// run("main");
printf("BYEE\n");
printf("AAAA\n");
}
