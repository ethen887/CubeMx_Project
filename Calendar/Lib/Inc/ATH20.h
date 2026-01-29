#ifndef __AHT20_H
#define __AHT20_H
#include "main.h"
#include "i2c.h"
#include "math.h"
#include "usart.h"
void AHT20_Init(void);
void AHT20_Read_Date();
void AHT20_Get_Data(float *temperature, float *humidity);
#endif

