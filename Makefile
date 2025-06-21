# 대상 소스/헤더 파일 목록
MODULES = led colorled textlcd fnd buzzer button temperature accelMagGyro

# 각 .o 파일 목록 만들기
OBJS = $(addsuffix .o, $(MODULES))

# 기본 타겟: libMyPeri.a 생성
all: libMyPeri.a

# 각 .o 파일 만들기 (개별 .c → .o)
%.o: %.c %.h
	arm-linux-gnueabi-gcc -c $< -o $@

# 라이브러리 만들기 (모든 .o를 묶어서 .a 생성)
libMyPeri.a: $(OBJS)
	arm-linux-gnueabi-ar rc $@ $^

# 정리(선택)
clean:
	rm -f *.o libMyPeri.a
