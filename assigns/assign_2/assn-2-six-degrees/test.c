#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int cmpFn(const void *key, const void *member){
    char * str_key = (char *) key;
    char * str_member = *(char **) member;
    return strcmp(str_key, str_member);
}

int main(){
    char *names[] = {"ann", "bob", "cindy", "david", "gill", "howard", "ivy", "jorah"};
    char *name = names[1]; 
    int *pos = (int *)bsearch(&name, names, 8, sizeof(char *), cmpFn);
    printf("%d\n", *pos);
    return 0; 
}