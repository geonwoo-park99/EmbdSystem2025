#include <stdio.h>
#include "textlcd.h"

int main(int argc, char **argv)
{
    // 인자가 부족하면 사용법 안내
    if (argc < 3) {
        printf("Usage: %s <line(1 or 2)> <string>\n", argv[0]);
        printf("예시: %s 1 Hello\n", argv[0]);
        return 1;
    }

    // textlcdwrite 함수 호출 (textlcd.c에 구현)
    return textlcdwrite(argc, argv);
}
