/* Including Area */
#include "Heart_Rate.h"
#include "stm32f1xx_hal.h" // 需要用到 HAL_GetTick() 获取系统时间
/*=======================================================================================*/

/* global variables */
float buf_red;  // red light
float buf_ir; // infrared
int Peak_Count = 0;
float buf_data[3] = {0}; // store the last three values of the red data after low-pass filtering
#define sum_totle 10
/* ======================================================================================*/

/**
 *@brief: DC component removal using a high-pass filter
 *@param buf_red: the value of the newest red data buffer, store the current red data
 *@return: the filtered red data
 */
float High_Pass_Filter(float buf_ir)
{
    static float buf_ir_pre = 0; // store the previous value of ir data
    float buf_ir_cur = 0; // store the current value of ir data after high-pass filtering
    buf_ir_cur = buf_ir + Alpha * buf_ir_pre; // DC component removal using a high-pass filter, which is an integral 
    float buf_ir_out = buf_ir_cur - buf_ir_pre;  // the output of the filter is the difference between the current value and the previous value, which is the AC component
    buf_ir_pre = buf_ir_cur; // update the previous value for the next iteration
    return buf_ir_out;
}

/*
 *@brief: low-pass filter
 *@param input_ac_ir: the value of the newest ir data buffer shifted after DC removal using a high-pass filter, store the current ir data
 *@return: the filtered ir data after low-pass filtering
 * @note: the low-pass filter is a weighted average of the current input and the previous output, which can smooth the signal and reduce noise
 */
float Low_Pass_Filter(float after_high_pass_filter_buf_ir)
{
    static float buf_lpf_pre = 0;
    float buf_lpf_cur = 0;
    buf_lpf_cur = Beta * after_high_pass_filter_buf_ir + (1 - Beta) * buf_lpf_pre; // low-pass filter, which is a weighted average of the current input and the previous output
    buf_lpf_pre = buf_lpf_cur;
    return buf_lpf_cur;
}


int Find_Heart_Peak(float input_lpf_ir_1, float input_lpf_ir_2, float input_lpf_ir_3)
{
    uint32_t current_time = HAL_GetTick(); // obtain the current system time in milliseconds
    static uint32_t last_time = 0; // store the last time when a peak was detected
    if( input_lpf_ir_1 < input_lpf_ir_2 && input_lpf_ir_2 > input_lpf_ir_3 && input_lpf_ir_2 > Peak_Threshold && (current_time - last_time > Peak_Interval))
    {
        last_time = current_time; // update the last time when a peak was detected
        Peak_Count++;
        return 1; // a peak is detected
        
    }
    else
    {
        return 0;
    }
}

void Buf_Data_Updata()
{    
    buf_data[0] = buf_data[1];
    buf_data[1] = buf_data[2];
    buf_data[2] = Low_Pass_Filter( High_Pass_Filter(buf_ir) ); // update the buffer with the new value after filtering
}

int Hear_Rate_Calculate(void)
{
    uint32_t current_time = HAL_GetTick(); // obtain the current system time in milliseconds
    static uint32_t last_time = 0;
    static uint32_t heart_rate = 0; // store the calculated heart rate
    static uint8_t index = 0;
    static int bpm_bufp[sum_totle] = {0};
    Buf_Data_Updata(); 
    if (Find_Heart_Peak(buf_data[0], buf_data[1], buf_data[2]) )
    {
        int new_bpm = 60 * 1000 / (current_time - last_time); // calculate the heart rate based on the time interval between two peaks, which is 60 seconds divided by the time interval in milliseconds
        last_time = current_time; // update the last time when a peak was detected
        if(  new_bpm > 40 && new_bpm < 150 )
        {
            bpm_bufp[index % sum_totle] = new_bpm;
            index++;
            int sum = 0;
            for( int i = 0; i < sum_totle; i++ ) sum += bpm_bufp[i];
            heart_rate = sum / sum_totle;
        }
    }
    return heart_rate;
}
