#ifndef __ENCODER_H
#define __ENCODER_H
#include "main.h"
#include "tim.h"
void EncoderInit(void);
uint32_t GetEncoderCounter(TIM_HandleTypeDef *htim);
void EncoderJudge(void);
typedef void (*EncoderCallback)(void);
void EncoderSetOnBackwardCallback(EncoderCallback callback);
void EncoderSetOnForwardCallback(EncoderCallback callback);
void EncoderSetOnButtonPressedCallback(EncoderCallback callback);
uint32_t GetTick(void);
#endif // !__ENCODER_H
