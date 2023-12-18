#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/stat.h>

int exec(char* args[]){
    pid_t id=fork();
    int child_status;
    if(id==0){
    execvp(args[0], args);
    } 
    if(id<0){
        printf("forking failed");
    return -1;
    }
    wait(&child_status);
    return 0;
}

char* fext(char* path){
    char *retStr;
    char *lastExt;
    if (path== NULL) return NULL;
    if ((retStr = malloc (strlen (path) + 1)) == NULL) return NULL;
    strcpy (retStr, path);
    lastExt = strrchr (retStr, '.');
    if (lastExt != NULL)
        *lastExt = '\0';
    return retStr;
}

int main(int argc, char* argv[]){
  if(argc==1){ 
printf("not enough args\n"); 
return -1;
  }
char* output=fext(argv[1]);
/* mkdir("/bin", 0644); */
// strcat("/bin", output);
char* command[]={"gcc", "-Wall", argv[1], "-o" , fext(argv[1]), NULL};
printf("compiled:%s output is:%s\n", argv[1], fext(argv[1]));
exec(command);
}
