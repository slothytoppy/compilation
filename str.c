#include <stdio.h>

int main(){
char hello[][10]={{
	72+32, 69+32, 76+32, 76+32, 79+32}
};
char* str=(void*)hello;
int i;
for(i=0; i<21; i++){
printf("%s", *str++);
}
}
