#include "gm6020.h"
GM6020_t gm6020[8] = {0};
/* ============================================================================
 * 发送使用静态变量,仅在该文件中使用，其他文件无法调用
*=============================================================================*/
static CAN_TxHeaderTypeDef s_tx_header; // 发送报文头, 里面存放发送报文基本信息
static uint8_t s_tx_data[8];            // 存放发送报文数据
static uint32_t s_tx_mailbox;           // 存放发送报文邮箱号,有3个邮箱,由HAL_CAN_Transmit函数返回

/* GM6050 CAN通信初始化函数 */
void  GM6050_CAN_Init(void)
{
    /*
     * CAN 过滤器配置, 需要设置的参数有:
     * CAN_FilterTypeDef结构体: 定义CAN过滤器结构体
     * FilterBank: CAN过滤器编号, 共28个, 0~13 归 CAN1, 14~27 归 CAN2
     * FilterMode: CAN过滤器模式, 有掩码模式和列表模式两种
     * FilterScale: CAN过滤器大小, 16位或者32位, 一般选择32位, 精度更高
     * FilterIdHigh: CAN过滤器期望ID高16位
     * FilterIdLow: CAN过滤器期望ID低16位
     * FilterMaskIdHigh: CAN过滤器掩码高16位( 为1的位表示要检查这一位; 为0的位表示不用检查这一位. 列表模式下无此参数 )
     * FilterMaskIdLow: CAN过滤器掩码低16位( 为1的位表示要检查这一位; 为0的位表示不用检查这一位. 列表模式下无此参数 )
     * FilterFIFOAssignment: 通过CAN过滤器的帧被分配到FIFO0或者FIFO1
     * FilterActivation: CAN过滤器使能
     * SlaveStartFilterBank: CAN从站过滤器起始编号, 14是标准分值( 0~13 由CAN1使用, 14~27 由CAN2使用 )
    */
    CAN_FilterTypeDef filter; // CAN过滤器结构体
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK; // 掩码模式
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh = 0x0000; 
    filter.FilterIdLow = 0x0000; 
    filter.FilterMaskIdHigh = 0x0000;
    filter.FilterMaskIdLow = 0x0000; // 高16位和低16位都为0, 表示任意ID都可以接受, 每一位都不检查, 即无过滤.因此期望ID是多少其实并不重要
    filter.FilterFIFOAssignment = CAN_RX_FIFO0; // 通过过滤的帧被分配到FIFO0, 必须和下面的中断函数对应起来
    filter.FilterActivation = ENABLE; // 使能过滤器
    filter.SlaveStartFilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan1, &filter); // 将上述配置写入硬件寄存器,过滤器正式生效
    HAL_CAN_Start(&hcan1); // 启动CAN1. 注意必须在启动CAN1之前先调用CAN_ConfigFilter函数
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING); // 开启CAN1接收中断, 当FIFO0有消息时, 会调用CAN1_Rx_FIFO0_IT_Handler函数
}
/* ============================================================================
 * 功能函数, 仅在函数内部使用: 填写发送帧头
* =============================================================================*/
static void Fill_Tx_Header(uint32_t CAN_ID)
{
    s_tx_header.StdId = CAN_ID; // 要接收报文的电机ID
    s_tx_header.IDE = CAN_ID_STD; // 标准帧( 11 位ID, 扩展帧有29位ID )
    s_tx_header.RTR = CAN_RTR_DATA; // 数据帧( 用于发送数据, 并非请求帧 )
    s_tx_header.DLC = 8; // 数据长度为8字节
}

/*=============================================================================
 * 功能函数, 仅在函数内部使用: 将 int16_t 数据按照大端序拆成两个字节写入缓存区
 *
 * 大端序: 高字节在前, 低字节在后
 * 例：3000 = 0x0BB8 → buf[0]=0x0B, buf[1]=0xB8
 * 要这么拆的原因:
 * CAN通信每次只能接受一位字节, int16_t 类型占两个字节, 所以需要将int16_t数据拆成两个字节写入缓存区 
*==============================================================================*/
static void Pack_int16(uint8_t *buf, int16_t data)
{
    buf[0] = (uint8_t) ( data >> 8 ); //右移八位, 获取高字节
    buf[1] = (uint8_t) ( data & 0x00FF ); // 与b 0000000011111111进行与运算, 获取低字节
}