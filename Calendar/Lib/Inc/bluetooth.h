#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "main.h"
#include "usart.h"

void Bluetooth_Init(void);      // 初始化并开启中断接收
void Bluetooth_Test(void);      // 你原本的发送测试
extern uint8_t bl_rx_data[50];      // 暴露这个变量，方便在 main 里面查看
typedef void (*bl_callback)(void);
#endif