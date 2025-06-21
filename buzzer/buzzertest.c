#include "buzzer.h"
#include <unistd.h>   // sleep 함수 사용

int main(void) {
    // 버저 초기화
    if (!buzzerInit()) {
        printf("buzzerInit failed!\n");
        return -1;
    }

    // 첫 번째 음계 재생
    buzzerPlaySong(1);

    sleep(1); // 1초 동안 소리 재생

    // 버저 끄기
    buzzerStopSong();

    // 버저 종료
    buzzerExit();

    return 0;
}
