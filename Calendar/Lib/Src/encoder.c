/**
 * @file    encoder.c
 * @brief   旋转编码器驱动模块（TIM 编码器模式 + 按键）
 *
 * 功能说明：
 *  - 使用 TIM 编码器模式检测旋转方向
 *  - 软件消抖，防止旋转和按键误触发
 *  - 通过回调函数机制上报正转 / 反转 / 按键事件
 *
 * 使用方法：
 *  1. 系统初始化时调用 EncoderInit()
 *  2. 通过 EncoderSetOnXXXCallback() 注册回调函数
 *  3. 在主循环中周期性调用 EncoderJudge()
 */

/* ===================== 编码器参数配置 ===================== */

#include "encoder.h"
/** 编码器使用的定时器（移植时只需修改这里） */
#define ENCODER_TIM htim1
/** 编码器初始计数值 */   
#define COUNTER_INIT_VALUE ( (uint16_t) 30000)
/** 编码器最大延迟时间, 消抖时间为 ENCODER_MAX_DELAY ms */
#define ENCODER_MAX_DELAY  ( (uint16_t) 100) 
/** 按键延迟时间, 消抖时间为 ENCODER_KEY_DELAY ms */
#define ENCODER_KEY_DELAY  ( (uint16_t) 10)   
/** 按键状态枚举类型, Pressed 表示按键按下, Released 表示按键未按下 */
typedef enum {Pressed, Released} ButtonState;   

/* ===================== 函数实现 ===================== */

/** 获取按键状态 */
ButtonState GetButtonState(void)
{
    // 这里应该根据实际硬件连接情况来判断按键状态
    // 例如，如果按键按下时引脚为低电平，则返回Pressed
    // 如果按键未按下时引脚为高电平，则返回Released
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_RESET ? Pressed : Released; // 假设按键连接在PB14引脚上
}
/**获取时钟当前计数值, 用于后面的软件消抖 */
uint32_t GetTick(void)
{
    return HAL_GetTick();
}

/** 设置编码器初始计数值 */
void SetInitEncoderCounter(uint16_t value)
{
    __HAL_TIM_SET_COUNTER(&ENCODER_TIM, value);
}

/** 编码器初始化 */
void EncoderInit(void)
{
    HAL_TIM_Encoder_Start(&ENCODER_TIM, TIM_CHANNEL_ALL);
    SetInitEncoderCounter(COUNTER_INIT_VALUE);
}

/** 获取编码器计数数值 */
uint32_t GetEncoderCounter(TIM_HandleTypeDef *htim)
{
    return __HAL_TIM_GET_COUNTER(htim);
}

/**自己定义回调函数指针, 后续用于指向发生特定事件时需要调用的函数 */
EncoderCallback onForwardCallback = NULL;
EncoderCallback onBackwardCallback = NULL;
EncoderCallback onButtonPressedCallback = NULL;

/** 设置编码器正转时要调的函数 */
void EncoderSetOnForwardCallback(EncoderCallback callback)
{
    onForwardCallback = callback;
}

/** 获取编码器反转时要调的函数 */
void EncoderSetOnBackwardCallback(EncoderCallback callback)
{
    onBackwardCallback = callback;
}

/** 获取编码器按键按下时要调的函数 */
void EncoderSetOnButtonPressedCallback(EncoderCallback callback)
{
    onButtonPressedCallback = callback;
}

volatile uint32_t last_read = 0;

/** 判断编码器正转、反转、按键是否按下的函数 */
void EncoderJudge(void)
{
    uint32_t now = GetTick();
    uint32_t cnt = GetEncoderCounter(&ENCODER_TIM);
    /*判断编码器是否转过了, 只有在转过时才进行判断, 避免编码器的延时对按键延时造成误差*/
    if(cnt != COUNTER_INIT_VALUE){
    
        if(now - last_read < ENCODER_MAX_DELAY ) return; // 如果距离上次读取时间小于最大延迟时间, 那么就直接返回, 不进行后续判断
        last_read = now;
        
        // 说明是顺时针旋转
        if (cnt > COUNTER_INIT_VALUE)
        {
            
            //如果指针不是空指针, 那么就调用回调函数, 执行指针所指向函数的内容
            if (onForwardCallback != NULL)
            {
                onForwardCallback();
            }
        }
        
        // 说明是逆时针旋转
        else if (cnt < COUNTER_INIT_VALUE)
        {
            //如果指针不是空指针, 那么就调用回调函数, 执行指针所指向函数的内容
            if (onBackwardCallback != NULL)
            {
                onBackwardCallback();
            }
        }
    }
    
    SetInitEncoderCounter(COUNTER_INIT_VALUE);

    ButtonState buttonstate = GetButtonState();
    
    static uint8_t callback_state = 0;  //用于记录按键回调函数是非被调用过, 0表示未调用过, 1表示调用过
    static uint32_t pressed_time = 0; //用于记录上次按键时间
    if (buttonstate == Pressed) //按键按下
    {
        if(pressed_time == 0)
        {
            pressed_time = GetTick(); //记录按键按下的时间
        }
        
        /**如果没有callback_state来判断按键是否被调用过, 那么按键只要处于按下状态, 
        * 那么就一直调用按键回调函数, 直到按键弹起为止,但我们得避免这种情况 */
        else if (callback_state ==0 && ( GetTick() - pressed_time ) > ENCODER_KEY_DELAY)
        {
            //如果指针不是空指针, 那么就调用回调函数, 执行指针所指向函数的内容
            if (onButtonPressedCallback != NULL)
            {
                onButtonPressedCallback();
                callback_state = 1; //表示按键回调函数已经被调用过
            }
            
        } 
    }
    else
    {
        pressed_time = 0; //重置按键时间
        callback_state = 0; //重置回调函数调用状态
    }
    

}


