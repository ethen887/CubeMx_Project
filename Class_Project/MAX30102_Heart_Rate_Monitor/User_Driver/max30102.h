#ifndef __MAX30102_H__
#define __MAX30102_H__


/******************************************************************************************/
/* ���� �� �жϱ�� & �жϷ����� ���� */ 

#define INT_GPIO_PORT                   GPIOD
#define INT_GPIO_PIN                    GPIO_PIN_3
#define INT_GPIO_CLK_ENABLE()           do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)  
#define INT_IRQn                        EXTI3_IRQn
#define INT_IRQHandler                  EXTI3_IRQHandler

/******************************************************************************************/
/* ������ַ */
#define MAX30102_I2C_ADDR               (0xAE)  /* �豸��ַ д��ַ0xAE ����ַ0xAF */

/* �Ĵ�����ַ */
#define MAX30102_INTR_STATUS_1          0x00
#define MAX30102_INTR_STATUS_2          0x01
#define MAX30102_INTR_ENABLE_1          0x02
#define MAX30102_INTR_ENABLE_2          0x03

#define MAX30102_FIFO_WR_PTR            0x04
#define MAX30102_OVF_COUNTER            0x05
#define MAX30102_FIFO_RD_PTR            0x06
#define MAX30102_FIFO_DATA              0x07

#define MAX30102_FIFO_CONFIG            0x08
#define MAX30102_MODE_CONFIG            0x09
#define MAX30102_SPO2_CONFIG            0x0A
#define MAX30102_LED1_PA                0x0C
#define MAX30102_LED2_PA                0x0D

#define MAX30102_MULTI_LED_CTRL1        0x11
#define MAX30102_MULTI_LED_CTRL2        0x12

#define MAX30102_TEMP_INTR              0x1F
#define MAX30102_TEMP_FRAC              0x20
#define MAX30102_TEMP_CONFIG            0x21

#define MAX30102_PROX_INT_THRESH        0x30

#define MAX30102_REV_ID                 0xFE
#define MAX30102_PART_ID                0xFF

#include "myiic.h"
/******************************************************************************************/

extern uint8_t g_max30102_int_flag;

/******************************************************************************************/
/* �ⲿ�ӿں���*/
float atk_max30102_get_temp(void);
void atk_max30102_init(void);
void atk_max30102_fifo_read(float *red_data, float *ir_data);
float atk_max30102_get_spo2(float *ir_input_data, float *red_input_data, uint16_t cache_nums);
uint16_t atk_max30102_get_heart(float *input_data, uint16_t cache_nums);
#endif
