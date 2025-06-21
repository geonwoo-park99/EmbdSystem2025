#ifndef _ACCELMAGGYRO_H_
#define _ACCELMAGGYRO_H_

int accelMagGyroInit(void);
int accelMagGyroData (void);
int accelMagGyroExit(void);

#define ACCELPATH "/sys/class/misc/FreescaleAccelerometer/"
#define MAGNEPATH "/sys/class/misc/FreescaleMagnetometer/"
#define GYROPATH "/sys/class/misc/FreescaleGyroscope/"

#endif _ACCELMAGGYRO_H_
