#include "max30102.h"
#include "math.h"


uint8_t g_max30102_int_flag = 0;            /* 中断标志 */

/**
 * @brief   写一个字节到max30102的寄存器
 * @param   reg: 寄存器地址
 * @param   data: 寄存器数据
 * @retval  写入结果
 * @arg     0: 成功
 * @arg     1: 失败
 */
uint8_t atk_max30102_write_byte(uint8_t reg, uint8_t data)
{
    iic_start();
    iic_send_byte(MAX30102_I2C_ADDR | 0x00);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(data);        /* 发送一字节 */
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_stop();                 /* 产生一个停止条件 */
    
    return 0;
}

/**
 * @brief   在max30102指定寄存器地址读出一个数据
 * @param   reg:        寄存器地址
 * @retval  读到的数据
 */
uint8_t atk_max30102_read_byte(uint8_t reg)
{
    uint8_t temp = 0;
    
    iic_start();
    iic_send_byte(MAX30102_I2C_ADDR | 0x00);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    
    iic_start();
    iic_send_byte(MAX30102_I2C_ADDR | 0x01);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    temp = iic_read_byte(0);
    iic_stop();
    
    return temp;
}

/**
 * @brief   从max30102读取N字节数据
 * @param   reg:        寄存器地址
 * @param   date:       数据存储buf
 * @param   len:        数据长度
 * @retval  读出结果
 * @retval  0, 操作成功
 *          其他, 操作失败
 */
int atk_max30102_read_nbytes(uint8_t reg, uint8_t *date, uint8_t len)
{
    uint8_t i;
    
    iic_start();
    iic_send_byte(MAX30102_I2C_ADDR | 0x00);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    
    iic_start();
    iic_send_byte(MAX30102_I2C_ADDR | 0x01);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    for (i = 0; i < len; i++)
    {
        date[i] = iic_read_byte((i == (len - 1)) ? 0 : 1);
    }
    iic_stop();
    
    return 0;
}

/**
 * @brief   max30102写入N字节数据
 * @param   reg:        寄存器地址
 * @param   data:       写入数据
 * @param   len:        数据长度
 * @retval  写入结果
 * @arg     0:          成功
 * @arg     1:          失败
 */
uint8_t atk_max30102_write_nbytes(uint8_t reg, uint8_t* data, uint8_t len)
{
    uint8_t i;
    
    iic_start();
    iic_send_byte(MAX30102_I2C_ADDR | 0x00);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg);
    if (iic_wait_ack() != 0)
    {
        iic_stop();
        return 1;
    }
    for (i = 0; i < len; i++)
    {
        iic_send_byte(data[i]);
        if (iic_wait_ack() != 0)
        {
            iic_stop();
            return 1;
        }
    }
    iic_stop();
    
    return 0;
}

/**
 * @brief   复位max30102
 * @param   无   
 * @retval  无
 */
void atk_max30102_reset(void)
{
    atk_max30102_write_byte(MAX30102_MODE_CONFIG, 0x40);
    HAL_Delay(10);
}

/**
 * @brief   读取FIFO数据
 * @param   red_data :    存储RED数据   
 * @param   ir_data  :    存储IR数据    
 * @retval  无
 */
void atk_max30102_fifo_read(float *red_data, float *ir_data)
{
    uint8_t receive_data[6];
    /* 通过读取将中断状态寄存器，将中断标志位清空 */
    atk_max30102_read_byte(MAX30102_INTR_STATUS_1);
    atk_max30102_read_byte(MAX30102_INTR_STATUS_2);
    
    atk_max30102_read_nbytes(MAX30102_FIFO_DATA, receive_data, 6);
    *red_data = ((receive_data[0] << 16 | receive_data[1] << 8 | receive_data[2]) & 0x03ffff);  /* RED数据 */
    *ir_data = ((receive_data[3] << 16 | receive_data[4] << 8 | receive_data[5]) & 0x03ffff);   /* IR数据 */
}

/**
 * @brief   获取温度值（℃）
 * @param   无   
 * @retval  返回温度值
 */
float atk_max30102_get_temp(void)
{
    float integer = 0.0f, fraction = 0.0f;
    
    atk_max30102_write_byte(MAX30102_TEMP_CONFIG, 0x01);    /* 使能温度传感器 读取温度值后自动清0 */
    
    integer = atk_max30102_read_byte(MAX30102_TEMP_INTR);   /* 读取温度值整数部分 */
    fraction = atk_max30102_read_byte(MAX30102_TEMP_FRAC);  /* 读取温度值小数部分 */

    if(integer < 0x80)                                      /* 小于0x80为正值 */
    {
        return (integer + fraction * 0.0625f);
    }
    else                                                    /* 大于等于0x80为负值 */
    {
        integer -= 256;
        return (integer + fraction * 0.0625f);
    }
}


/**
 * @brief   初始化max30102
 * @param   无   
 * @retval  无
 */
void atk_max30102_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    INT_GPIO_CLK_ENABLE();
    /* INT引脚初始化为外部中断线 */
    gpio_init_struct.Pin = INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;           /* 下升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    HAL_GPIO_Init(INT_GPIO_PORT, &gpio_init_struct);        /* INT配置为下降沿触发中断 */
    
    HAL_NVIC_SetPriority(INT_IRQn, 0, 2);                   /* 抢占0，子优先级2 */
    HAL_NVIC_EnableIRQ(INT_IRQn);                           /* 使能中断线5 */
    
    iic_init();                                             /* 初始化I2C接口 */
    
    atk_max30102_reset();                                   /* 复位设备 */
    
    atk_max30102_write_byte(MAX30102_INTR_ENABLE_1, 0xC0);  /* 中断使能：FIFO满以及新FIFO数据就绪 */
    atk_max30102_write_byte(MAX30102_INTR_ENABLE_2, 0x00);
    
    atk_max30102_write_byte(MAX30102_FIFO_WR_PTR, 0x00);    /* 清空指针FIFO_WR_PTR[4:0] */
    atk_max30102_write_byte(MAX30102_OVF_COUNTER, 0x00);    /* 清空指针OVF_COUNTER[4:0] */
    atk_max30102_write_byte(MAX30102_FIFO_RD_PTR, 0x00);    /* 清空指针FIFO_RD_PTR[4:0] */
    
   // atk_max30102_write_byte(MAX30102_FIFO_CONFIG, 0x4F);    /* 样本平均（4），FIFO满滚（0），FIFO几乎满值（发出中断时为15个空数据样本） */
    atk_max30102_write_byte(MAX30102_FIFO_CONFIG, 0x6F);    /* 样本平均（8），FIFO满滚（0），FIFO几乎满值（发出中断时为15个空数据样本） */
    
    atk_max30102_write_byte(MAX30102_MODE_CONFIG, 0x03);    /* 配置为SpO2（血氧饱和）模式，会测量RED 和 IR */
    
  //  atk_max30102_write_byte(MAX30102_SPO2_CONFIG, 0x2A);    /* SpO2配置: ADC范围: 4096nA，采样速率控制:200Hz，·LED脉冲宽度:215us· */
    atk_max30102_write_byte(MAX30102_SPO2_CONFIG, 0x2F);/* SpO2配置: ADC范围: 4096nA，采样速率控制:400Hz，·LED脉冲宽度:411us· */
    
    atk_max30102_write_byte(MAX30102_LED1_PA, 0x30);        /* IR LED 电流选择9.4mA */
    atk_max30102_write_byte(MAX30102_LED2_PA, 0x30);        /* RED LED 电流选择9.4mA */
    
  //  atk_max30102_write_byte(MAX30102_LED1_PA, 0x17);        /* IR LED 电流选择4.5mA */
 //   atk_max30102_write_byte(MAX30102_LED2_PA, 0x17);        /* RED LED 电流选择4.5mA */
    
//    atk_max30102_write_byte(MAX30102_TEMP_CONFIG, 0x01);    /* 使能温度传感器 读取温度值后自动清0 */
//    HAL_Delay(100);                                          /* 等待温度传感器稳定 */
    
    /* 建议初始化时，通过读取将中断状态寄存器，将中断标志位清空 */
    atk_max30102_read_byte(MAX30102_INTR_STATUS_1);
    atk_max30102_read_byte(MAX30102_INTR_STATUS_2);
}

/*******************************************************************************************/
#if 0
/**
 * @brief   获取心率值
 * @param   input_data ：经过FIR滤波后的红外光数据    
 * @param   cache_nums ：采样数量    
 * @note    经过滤波的信号，心跳会呈现周期性的峰值（每个峰值代表一次心跳）
 *          设定一个阈值（一般为采集样本的平均值），当信号超过该阈值时，识别为峰值
 *          计算两个采样点的差值，为了准确性同样多次采集取平均 得到稳定的两次采样点差值（两次心跳的间隔时间 = 两个采样点差值/采样频率）
 *          计算心率公式（bpm 即每分钟跳动次数）：Heart_Rate = 60 * 采样频率 / 两个采样点差值
 * @retval  返回心率值
 * @retval  返回0 代表识别失败，皮肤要紧贴并保持不动
 */
uint16_t atk_max30102_get_heart(float *input_data, uint16_t cache_nums)
{
    float input_data_sum_aver = 0;
    uint16_t i, temp;
    
    for(i = 0; i < cache_nums; i++)
    {
        input_data_sum_aver += *(input_data + i);
    }
    input_data_sum_aver = input_data_sum_aver / cache_nums;
    for(i = 0; i < cache_nums; i++)
    {
        /* 检测峰值 */
        if((*(input_data + i) > input_data_sum_aver) && (*(input_data + i + 1) < input_data_sum_aver))
        {
            temp = i;       /* 当前峰值的时间（采样点） */
            break;
        }
    }
    i++;
    for(; i < cache_nums; i++)
    {
            if((*(input_data + i) > input_data_sum_aver) && (*(input_data + i + 1) < input_data_sum_aver))
            {
                temp = i - temp;
                break;
            }
    }
    /* 心率 = 60 * sampling_rate / temp  单位：bpm(次/min) */

    return 3000 / temp;
}
#else
/**
 * @brief   获取心率值
 * @param   input_data ：经过FIR滤波后的红外光数据    
 * @param   cache_nums ：采样数量    
 * @note    经过滤波的信号，心跳会呈现周期性的峰值（每个峰值代表一次心跳）
 *          设定一个阈值（一般为采集样本的平均值），当信号超过该阈值时，识别为峰值
 *          计算两个采样点的差值，为了准确性同样多次采集取平均 得到稳定的两次采样点差值（两次心跳的间隔时间 = 两个采样点差值/采样频率）
 *          计算心率公式（bpm 即每分钟跳动次数）：Heart_Rate = 60 * 采样频率 / 两个采样点差值
 * @retval  返回心率值
 * @retval  返回0 代表识别失败，皮肤要紧贴并保持不动
 */
uint16_t atk_max30102_get_heart(float *input_data, uint16_t cache_nums)
{
    float input_data_sum_aver = 0;
    uint16_t i, temp;
    int num_peaks = 0;
    int current_peak_time = 0;
    int last_peak_time = 0;
    int total_intervals = 0;
    
    for(i = 0; i < cache_nums; i++)
    {
        input_data_sum_aver += *(input_data + i);
    }
    input_data_sum_aver = input_data_sum_aver / cache_nums; /* 平均值法，获得阈值 */
    
    for(i = 0; i < cache_nums; i++)
    {
        /* 检测峰值 */
        if((*(input_data + i) > input_data_sum_aver) && (*(input_data + i + 1) < input_data_sum_aver))
        {
            current_peak_time = i;                          /* 记录当前采样点 */
            if(last_peak_time != 0)
            {
                temp = current_peak_time - last_peak_time;  /* 计算两个采样点间的差 */
                total_intervals += temp;                    /* 差累加 */
                num_peaks++;                                /* 次数累加，后续取平均数用 */
            }
            last_peak_time = current_peak_time;             /* 记录上次采样点，以便下次计算两个峰值采样点的差 */
        }
    }

    /* 单位：bpm(次/min) 心率 = 60 * sampling_rate / 两个采样点间的差值   其中sampling_rate = 采样率/样本数： 200/4 = 50HZ */
    if(num_peaks > 0)
    {
        int avg_interval = total_intervals / num_peaks;     /* 得到两个峰值间的平均差 */
        return (3000 / avg_interval);                       /* 计算心率 */
    }
    return 0;
}
#endif

/**
 * @brief   获取血氧值
 * @param   ir_input_data ：经过FIR滤波后的红外光数据    
 * @param   red_input_data ：经过FIR滤波后的红光数据    
 * @param   cache_nums ：采样数量   
 * @note    首先通过对红光和红外光信号的最大最小值，计算其AC和DC分量
 *          然后通过公式计算出R = (ACred/DCred) / (ACir/DCir) ；
 *          最后根据R值使用公式SpO2 = aR^2 + bR + c  其中a b c是根据大量数据拟合得出的系数。
 * @retval  血氧值
 */
float atk_max30102_get_spo2(float *ir_input_data, float *red_input_data, uint16_t cache_nums)
{
    float ir_max = *ir_input_data, ir_min = *ir_input_data;
    float red_max = *red_input_data, red_min = *red_input_data;
    float R;        /* 比率R */
    uint16_t i;
    
    /* 寻找红光和红外光的最大值和最小值 */
    for(i = 1; i < cache_nums; i++)
    {
        if(ir_max < *(ir_input_data + i))
        {
            ir_max = *(ir_input_data + i);
        }
        if(ir_min > *(ir_input_data + i))
        {
            ir_min = *(ir_input_data + i);
        }
        if(red_max < *(red_input_data + i))
        {
            red_max = *(red_input_data + i);
        }
        if(red_min > *(red_input_data + i))
        {
            red_min = *(red_input_data + i);
        }
    }

    /* (ir_max - ir_min) 和 (red_max - red_min) 表示：红外和红光信号的AC分量（脉动成分）。 */
    /* (red_max + red_min) 和 (ir_max + ir_min)：表示：红光和红外信号的DC分量（静态成分） */
    R = ((ir_max + ir_min) * (red_max - red_min)) / ((red_max + red_min) * (ir_max - ir_min));
    return ((-30.060f) * R * R + 30.354f * R + 94.845f);    /* SpO2 = aR^2 + bR + c */
  //  return 130 - 25 * R;
    //return (30.354f * R + 94.845f);    /* SpO2 = aR^2 + bR + c */
}

/*******************************************************************************************/

/**
 * @brief       INT 外部中断服务程序
 * @param       无
 * @retval      无
 */
void INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(INT_GPIO_PIN);         /* 调用中断处理公用函数 清除INT所在中断线 的中断标志位 */
    __HAL_GPIO_EXTI_CLEAR_IT(INT_GPIO_PIN);         /* HAL库默认先清中断再处理回调，退出时再清一次中断，避免按键抖动误触发 */
}

/**
 * @brief       中断服务程序中需要做的事情
                在HAL库中所有的外部中断服务函数都会调用此函数
 * @param       GPIO_Pin:中断引脚号
 * @retval      无
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == INT_GPIO_PIN)
    {
        g_max30102_int_flag = 1;
    }
}



