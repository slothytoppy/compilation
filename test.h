#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

void* wmap(void* ptr, void* type, size_t size){
  if(!type) return 0;
  ptr=mmap(ptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
  if(!ptr){ printf("mapping failed\n");
  return 0;
  }
  if(sizeof(type)==sizeof(int*)) return (int*)ptr;
  if(sizeof(type)==sizeof(char*)) return (char*)ptr;
  if(sizeof(type)==sizeof(long*)) return (long*)ptr;
  // return ptr;
}

       // void *mremap(void old_address[.old_size], size_t old_size, \
                    size_t new_size, int flags, ... /* void *new_address */);
  
       // void *mmap(void *addr, size_t len, int prot, int flags, \
           int fildes, off_t off);

void* rmap(void* new_ptr, void* type, size_t new_size){
if(type==(void*)0) wmap(new_ptr, type, new_size);
if(sizeof(type)==sizeof(int*)){
int* buff=new_ptr;
buff=wmap(new_ptr, type, new_size);
if(!buff) return 0;
return (int*)buff;
} else if(sizeof(type)==sizeof(char*)){
if(new_ptr==(void*)0) wmap(new_ptr, type, new_size);
char* buff=new_ptr;
int i=0;
while(buff[i]){
  i++;
}
if(i==new_size) return 0;
new_ptr=mremap(new_ptr, i, new_size, 0);
if(!buff) return 0;
return buff;
}
else{
return 0;
}
}

// TODO: implement reallocarray
