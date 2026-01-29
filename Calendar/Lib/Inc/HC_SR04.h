#ifndef __HC_SR04_H__
#define __HC_SR04_H__ 
#include "main.h"
void delay_us(uint16_t us);
void HC_SR04_Init(void);
float HC_SR04_Get_Distance(void);
uint32_t HC_SR04_Hight_Time(void);
#endif
