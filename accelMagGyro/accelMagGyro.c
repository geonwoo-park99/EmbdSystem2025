#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "accelMagGyro.h"

#define ACCELPATH "/sys/class/misc/FreescaleAccelerometer/"
#define MAGNEPATH "/sys/class/misc/FreescaleMagnetometer/"
#define GYROPATH "/sys/class/misc/FreescaleGyroscope/"

    int fd_1, fd_2, fd_3 = 0;
    FILE *fp1, *fp2, *fp3 = NULL;
int accelMagGyroInit(void)
{ 

    fd_1 = open (ACCELPATH "enable",O_WRONLY);
    dprintf (fd_1,"1");

    fd_2 = open (MAGNEPATH "enable",O_WRONLY);
    dprintf (fd_2,"1");

    fd_3 = open (GYROPATH "enable",O_WRONLY);
    dprintf (fd_3,"1");

}

int accelMagGyroData (void)
{
    fp1 = fopen (ACCELPATH "data", "rt");
    int accel[3];
    fscanf(fp1,"%d, %d, %d",&accel[0],&accel[1],&accel[2]);
    printf ("I read Accel %d, %d, %d\r\n",accel[0],accel[1],accel[2]);

    fp2 = fopen (MAGNEPATH "data", "rt");
    int magne[3];
    fscanf(fp2,"%d, %d, %d",&magne[0],&magne[1],&magne[2]);
    printf ("I read Magneto %d, %d, %d\r\n",magne[0],magne[1],magne[2]);

    fp3 = fopen (GYROPATH "data", "rt");
    int gyro[3];
    fscanf(fp3,"%d, %d, %d",&gyro[0],&gyro[1],&gyro[2]);
    printf ("I read Gyroscope %d, %d, %d\r\n",gyro[0],gyro[1],gyro[2]);
}

int accelMagGyroExit(void)
{
    close(fd_1);
    fclose(fp1);
    close(fd_2);
    fclose(fp2);
    close(fd_3);
    fclose(fp3);
}
