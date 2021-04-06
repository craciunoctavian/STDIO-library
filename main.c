//
// Created by octi on 31.03.2021.
//

#include <stdio.h>
#include "util/so_stdio.h"


int main() {
    char buffer[256] = "aaSDADSAFASDDFSAADSFzzzzzzzzz";
    char buffer1[256];
    SO_FILE *f = so_fopen("octi.txt", "r+");
//    printf("%zu", so_fwrite("abcde", 5, 1, f));
//    printf(" Pos:%ld\n", so_ftell(f));
//    printf(" Pos:%d\n", so_fseek(f, 2, SEEK_CUR));
//    printf(" Pos:%ld\n", so_ftell(f));
//    printf("%s\n", buffer);
    printf("%zu\n", so_fwrite(buffer, 1, 25, f));
    printf("%zu\n", so_fread(buffer1, 1, 25, f));
    //printf("%d", so_fflush(f));
    printf(" Pos:%ld\n", so_ftell(f));
    printf("BUFFER 1 %s\n", buffer1);
    so_fclose(f);


    FILE* fl = fopen("octi.txt", "r+");
//    unsigned long r;// = fread(buffer, 2 ,50, fl);
//    //fwrite(buffer, 1 ,2, fl);
//    //fwrite(buffer, 1 ,1, fl);
//    fread(buffer, 1 ,1, fl);
//    fwrite(buffer, 1 ,25, fl);
//    int d = fflush(stdout);
//    fread(buffer + 1, 1 ,1, fl);
//    fread(buffer + 2, 1 ,1, fl);
//    printf("%d\n", d);
//    //printf("%zu", r);
//    printf("%s", buffer);
//    int d;
//    d = fwrite(buffer, 1 ,1, fl);
//    printf("%d\n", d);
//    printf("%s\n", buffer);
//    char buffer1[256];
//    d = fread(buffer1, 1 ,1, fl);
//    printf("%s\n", buffer1);
//    printf("%d\n", d);
//    buffer[0] = 'z';
//    d = ftell(fl);
//    printf("%d\n", d);
    return 0;
}
