#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "accelMagGyro.h"

int main (void)
{
  accelMagGyroInit();
  accelMagGyroData ();
  accelMagGyroExit();
}
