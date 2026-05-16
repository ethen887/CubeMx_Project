#ifndef __HEART_RATE_H__ // define to prevent recursive inclusion

/* ======================================================= */
#define __HEART_RATE_H__
#define Alpha 0.95f // this is a feedback coefficient of the high-pass filter
#define Beta 0.33f  // this is a feedback coefficient of the low-pass filter
#define Peak_Threshold 0.05 // the threshold of the peak detection
#define Peak_Interval 300 // the minimum interval between two peaks in milliseconds, which can be used to avoid false peak detection due to noise or motion artifacts
/* ======================================================= */

/* ======================================================= */
#include "max30102.h"
#include "stdio.h"
/* ======================================================= */

extern float buf_red;
extern float buf_ir;
float High_Pass_Filter(float buf_red);
float Low_Pass_Filter(float input_ac_ir);
int Hear_Rate_Calculate(void);
#endif /* __HEART_RATE_H__ */