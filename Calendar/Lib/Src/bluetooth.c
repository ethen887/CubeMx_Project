#include "bluetooth.h"
#define RED 0x11
#define GREEN 0x12
#define BLUE 0x13
#define LED_OFF 0x00
#define LED_ON  0x01
#define BLUETOOTH_HUART huart1

uint8_t bl_rx_data[50]; // 定义接收缓冲区（单个字节）
volatile uint8_t bl_rx_len = 0;
uint8_t bl_rx_data_tem;

// 启动蓝牙接收
void Bluetooth_Init(void)
{
    // 开启第一次中断接收
    HAL_UART_Receive_IT(&BLUETOOTH_HUART, &bl_rx_data_tem, 1);
}

// 串口中断回调函数：整个工程里只能有一个，写在这里 main.c 里就不要再写了
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &BLUETOOTH_HUART)
    {
        if (bl_rx_len < sizeof(bl_rx_data))
        {
            bl_rx_data[bl_rx_len++] = bl_rx_data_tem;
        }
        else
        {
            bl_rx_len = 0; // 丢包或重来
        }
        HAL_UART_Receive_IT(&BLUETOOTH_HUART, &bl_rx_data_tem, 1);
    }
}
// void Bluetooth_Task(void)
// {
//     if(bl_rx_len < 3) return;
//     uint8_t sum = 0;
//     if (bl_rx_data[0] != 0xAA)
//     {
//         bl_rx_len = 0;   // 丢掉这一帧
//         return;
//     }
//     for(int i = 0; i < (bl_rx_len - 1); i++)
//     {
//         sum += bl_rx_data[i];
//     }
//     if(sum == bl_rx_data[bl_rx_len - 1])
//     {
//         for(int i = 1; i < (bl_rx_len-2); i+=2)
//         {
//             GPIO_PinState state = LED_ON ? GPIO_PIN_SET : GPIO_PIN_RESET;
//             switch(bl_rx_data[i+1])
//             {
//                 case RED:
//                     HAL_GPIO_WritePin(RED_GPIO_Port, RED_Pin, state);
//                     break;
//                 case GREEN:
//                     HAL_GPIO_WritePin(GREEN_GPIO_Port, GREEN_Pin, state);
//                     break;
//                 case BLUE:
//                     HAL_GPIO_WritePin(BLUE_GPIO_Port, BLUE_Pin, state);

//             }
//         }
//         bl_rx_len = 0;
//     }
// }