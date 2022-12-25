#include<stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
    int x = atoi(argv[0]);
    x = x * 2;
    printf("%d\n", x);
    return 0;
}