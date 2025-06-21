#ifdef _COLORLED_H_
#define _COLORLED_H_


void doHelp(void);
void pwmActivate(int bActivate, int pwmIndex);
void pwmEnable(int bEnable, int pwmIndex);
void writePWMPeriod(int frequency, int pwmIndex);
void writePWMDuty(int DutyCycle, int pwmIndex);

#define COLORLED_DRIVER_NAME "/dev/periled"

#endif _COLORLED_H_
