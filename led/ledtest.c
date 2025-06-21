#include "led.h"
#include <unistd.h>  // sleep, usleep 함수 사용을 위해 필요

int main(void) {
    ledLibInit();
    ledOnOff(0, 1); // 0번 LED 켜기
    sleep(1); 
    ledOnOff(0, 0); // 0번 LED 끄기
    ledLibExit();
    return 0;
}
