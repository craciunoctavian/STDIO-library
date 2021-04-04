//
// Created by octi on 31.03.2021.
//

#include <stdio.h>
#include "util/so_stdio.h"


int main(){
    SO_FILE *f = so_fopen("octi.txt", "r+");
    printf("%c", so_fputc('c' ,f));
    printf("%c", so_fputc('c' ,f));
    printf("%c", so_fgetc(f));
    printf("%c", so_fgetc(f));
    printf("%c", so_fgetc(f));
    printf("%c", so_fgetc(f));
    printf("%c", so_fgetc(f));
    printf("%c", so_fgetc(f));
    printf("%d", so_fgetc(f));
    so_fclose(f);
    return 0;
}
