#include "ATH20.h"
#define AHT20_ADDRESS 0x70 // AHT20的地址 
#define AHT20_INIT_DELAY 40
#define AHT20_HI2C hi2c2 // ATH20挂载在了I2C2总线上
uint32_t humidity_date = 0, temperature_date = 0;
/*上电后传感器需要不少于100ms(此处设为120ms)的稳定时间（此时SCL为高电平)以达到空闲状态,即做好准备接收由主机(MCU)发送的命令。*/
void AHT20_Init(void)
{
  HAL_Delay(AHT20_INIT_DELAY);
  uint8_t state = HAL_I2C_Master_Receive(&AHT20_HI2C, AHT20_ADDRESS, &state, 1, HAL_MAX_DELAY);
  if( (state & 0x18) ==  0x18 ) return; //如果进入if函数, 就说明传感器已经初始化过了
  uint8_t sent_init_buffer[3] = {0xBE, 0x08, 0x00};
  HAL_I2C_Master_Transmit(&AHT20_HI2C, AHT20_ADDRESS, sent_init_buffer, sizeof(sent_init_buffer) / sizeof(sent_init_buffer[0]), HAL_MAX_DELAY);

}

/*初始化过后启动测量*/
void AHT20_Trigger_Measure(void)
{
 
  static uint8_t sent_trigger_buffer[3] = {0xAC, 0x33, 0x00};
  /*DMA,中断等都是先告诉硬件有数据需要处理了, 然后直接进行下一个函数, 函数返回的时候再处理数据,
   但此时ATH20_Trigger_Measure函数已经结束,上面的sent_trigger_buffer会被释放掉, 函数返回的时候数据就丢失了
   所以用static来保存数据*/
  HAL_I2C_Master_Transmit_DMA(&AHT20_HI2C, AHT20_ADDRESS, sent_trigger_buffer, sizeof(sent_trigger_buffer) / sizeof(sent_trigger_buffer[0]));
}

/*读取传感器返回的数据*/
void AHT20_Read_Date()
{   AHT20_Trigger_Measure();
    HAL_Delay(75);
    uint8_t data[6];
    HAL_I2C_Master_Receive(&AHT20_HI2C, AHT20_ADDRESS, data, sizeof(data) / sizeof(data[0]), HAL_MAX_DELAY);
    if( ( data[0] & 0x80 ) == 0x00 ){
        
        
        humidity_date = ( ( (uint32_t)data[1] ) << 12 ) | ( ( (uint32_t)data[2] )<< 4 ) | ( ( (uint32_t)data[3] )>> 4 );
        temperature_date = ( (uint32_t)data[5] ) | ( ( (uint32_t)data[4] ) << 8 ) | ( ( ( (uint32_t)data[3] ) & 0x0F ) << 16 );        
    }
       
}

    

/*解析数据*/
void AHT20_Get_Data(float *temperature, float *humidity)
{
    AHT20_Read_Date();
    *temperature = temperature_date * 200.0f / (1 << 20) - 50.0f;
    *humidity = humidity_date * 100.0f / (1 << 20);
    char message[50];
    sprintf(message, "Temperature: %.2f, Humidity: %.2f", *temperature, *humidity);
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
    HAL_Delay(1000);
}
