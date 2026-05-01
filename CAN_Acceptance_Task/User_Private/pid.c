/* 头文件包含区域 */
/* ====================================================================== */
#include "pid.h"
/* ====================================================================== */

/* 全局变量定义区域 */
/* ====================================================================== */
PID_T pid_t;
/* ====================================================================== */

/* 函数定义区域 */
/* ====================================================================== */
/**
 * @brief  PID控制器初始化函数
 * @param  pid: PID控制器结构体指针
 * @param  Kp: 比例增益
 * @param  Ki: 积分增益
 * @param  Kd: 微分增益
 * @param  Integral_Max: 积分限幅, 用于防止积分过大导致系统不稳定
 * @param  Output_Max: 输出限幅, 用于防止PID输出过大导致系统不稳定
 * @retval None
*/
void PID_Init(PID_T *pid, float Kp, float Ki, float Kd, float Integral_Max, float Output_Max)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->Integral_Max = Integral_Max;
    pid->Output_Max = Output_Max;
    pid->Integral = 0.0f;     
    pid->Last_Error = 0.0f;
    pid->Error = 0.0f;
    pid->Output = 0.0f;
    // 以上初始化积分值、误差值和输出值为0, 以确保PID控制器从一个干净的状态开始工作
}

/**
 * @brief  PID控制器计算函数
 * @param  pid: PID控制器结构体指针
 * @param  Set_Point: 设定值
 * @param  Actual_Value: 实际值
 * @retval PID输出值
*/
float PID_Calculate(PID_T* pid, float Set_Point, float Actual_Value)
{
    pid->Error = Set_Point - Actual_Value; // 计算当前误差
    pid->Integral += pid->Error; // 更新积分值
    // 积分限幅, 防止积分过大导致系统不稳定
    if( pid->Integral > pid->Integral_Max ) pid->Integral = pid->Integral_Max;
    if( pid->Integral < -pid->Integral_Max ) pid->Integral = -pid->Integral_Max;
    float Derivative = pid->Error - pid->Last_Error; // 计算微分项
    float pid_p_out = pid->Kp * pid->Error; // 计算比例项输出
    float pid_i_out = pid->Ki * pid->Integral; // 计算积分项输出
    float pid_d_out = pid->Kd * Derivative; // 计算微分项输出
    pid->Output = pid_p_out + pid_i_out + pid_d_out; // 计算总输出
    // 输出限幅, 防止PID输出过大导致系统不稳定
    if( pid->Output > pid->Output_Max ) pid->Output = pid->Output_Max;
    if( pid->Output < -pid->Output_Max ) pid->Output = -pid->Output_Max;
    pid->Last_Error = pid->Error; // 更新上次误差, 为下一次计算微分项做准备
    return pid->Output; // 返回PID输出值
}

/**
 * @brief  PID控制器清零函数
 * @param  pid: PID控制器结构体指针
 * @retval None
*/
void PID_Reset(PID_T* pid)
{
    pid->Integral = 0.0f;     
    pid->Last_Error = 0.0f;
    pid->Error = 0.0f;
    pid->Output = 0.0f;
}