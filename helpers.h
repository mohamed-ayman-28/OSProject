#include <stdlib.h>
#include "headers.h"

int convertIntToStr(int i, int str_len, char* str){
    if(str_len <= 0 || str == NULL){
        return -1;
    }

    int index = str_len - 1;
    int digit;
    while(i > 0 && index > 0){
        digit = i % 10;
        str[index] = digit - '0';
        index--;
        i = i / 10;
    }
    str[str_len-1] = '\0';

    return 0; 
}