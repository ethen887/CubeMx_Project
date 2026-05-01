#ifndef __PID_H__
#define __PID_H__ 
/*==== PID初始化结构体,定义好Kp,Ki,Kd参数等重要参数 ====*/
typedef struct 
{
    float Kp, Ki, Kd; // PID参数
    float Integral; // 积分值
    float Integral_Max; // 积分限幅
    float Output_Max; // 输出限幅
    float Error; // 当前误差
    float Last_Error; // 上次误差, 用于计算微分项
    float Output; // PID输出
}PID_T;
void PID_Init(PID_T *pid, float Kp, float Ki, float Kd, float Integral_Max, float Output_Max);
float PID_Calculate(PID_T* pid, float Set_Point, float Actual_Value);
void PID_Reset(PID_T* pid);

#endif
