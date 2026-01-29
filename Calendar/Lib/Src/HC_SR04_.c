#include "HC_SR04.h"
#define HC_SR04_INIT_TRIGGER 100 //Trig引脚准备时间
#define COUNTER_US htim4
#define VOICE_SPEED 340 //声音在空中传播的速度, 单位米/秒
/*微妙级别的延时函数, 用的是TIM4的定时器*/
uint32_t justnow = 0;
void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&COUNTER_US, 0);
    HAL_TIM_Base_Start(&COUNTER_US);
    while (__HAL_TIM_GET_COUNTER(&COUNTER_US) < us)
    {
    }
    HAL_TIM_Base_Stop(&COUNTER_US);
    __HAL_TIM_SET_COUNTER(&COUNTER_US, 0);

}
/*给Trig引脚发送一段时间的高电平信号, 用以触发声波发送*/
void HC_SR04_Init(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
}

/*判断超声波Echo引脚高电平时间*/
uint32_t HC_SR04_Hight_Time(void)
{
    justnow = HAL_GetTick();
    while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET)
    {
        if(HAL_GetTick() - justnow >= 2000)
        {
            justnow = 0;
            return 0;
        }
    }
    HAL_TIM_Base_Start(&COUNTER_US);
    __HAL_TIM_SET_COUNTER(&COUNTER_US, 0);
    justnow = HAL_GetTick();
    while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET)
    {
        if(HAL_GetTick() - justnow >= 2000){
            justnow = 0;
            return 0;
        }
    }
    HAL_TIM_Base_Stop(&COUNTER_US);
    return __HAL_TIM_GET_COUNTER(&COUNTER_US);
}

/*计算距离*/
float HC_SR04_Get_Distance(void)
{
    HC_SR04_Init();
    uint32_t time = HC_SR04_Hight_Time();
    float distance = (float)(time * 1e-6 * VOICE_SPEED) / 2.0f;
    return distance;
}

