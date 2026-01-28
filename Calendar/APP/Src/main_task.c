#include "main_task.h"
#define UART_PORT huart2
#define CURSOR_BLINK_TIME  500 // 光标闪烁间隔，单位为毫秒

char weekDays[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
/* 定义一个结构体变量用于存储设置的时间, 确保在进入设置模式的时候设置起始值是从当前时间开始, 否则起始时间可能很乱, 体验不好 */
struct tm setting_time;
/*定义一个万年历模式的枚举*/
typedef enum{
    CalenderMode_Normal,
    CalenderMode_Setting
}CalenderMode;

/*定义一个枚举型变量, 用来记录光标应该闪烁在哪个位置*/
typedef enum{
    CursorPosition_Year,
    CursorPosition_Month,
    CursorPosition_Day,
    CursorPosition_Hour,
    CursorPosition_Minute,
    CursorPosition_Second
}CursorPosition;

/*定义一个结构体, 用来存储光标闪烁在各个位置时闪烁的坐标*/
typedef struct{
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
}CursorPositionCoordinate;

CursorPositionCoordinate cursor_position_coordinate[6] = {
    {24 + 0 * 8,  17, 24 + 4 * 8, 17}, //year
    {24 + 5 * 8,  17, 24 + 7 * 8, 17}, //month
    {24 + 8 * 8,  17, 24 + 10 * 8, 17},//day
    {16 + 0 * 8,  45, 16 + 2 * 12, 45},//hour
    {16 + 3 * 12, 45, 16 + 5 * 12, 45},//minute
    {16 + 6 * 12, 45, 16 + 8 * 12, 45} //second
};
CalenderMode calenderMode = CalenderMode_Normal;
CursorPosition cursor_position = CursorPosition_Year;

/*编码器正转时执行下面这个操作*/
void EncoderForward(void)
{
    switch (cursor_position)
    {
    case CursorPosition_Year:
        setting_time.tm_year += 1;
        break;
    case CursorPosition_Month:
        setting_time.tm_mon += 1;
        if(setting_time.tm_mon > 11) setting_time.tm_mon = 0; //月份范围是0-11
        break;
    case CursorPosition_Day:
        setting_time.tm_mday += 1;
        if(setting_time.tm_mday > 31) setting_time.tm_mday = 1; //日期范围是1-31
    
    case CursorPosition_Hour:
        setting_time.tm_hour += 1;
        if(setting_time.tm_hour > 23) setting_time.tm_hour = 0; //小时范围是0-23
        break;
    
    case CursorPosition_Minute:
        setting_time.tm_min += 1;
        if(setting_time.tm_min > 59) setting_time.tm_min = 0; //分钟范围是0-59
        break;
    case CursorPosition_Second:
        setting_time.tm_sec += 1;
        if(setting_time.tm_sec > 59) setting_time.tm_sec = 0; //秒钟范围是0-59
        break;
    }
}
/*编码器反转时执行下面这个操作*/
void EncoderBackward(void)
{
    switch (cursor_position)
    {
    case CursorPosition_Year:
        setting_time.tm_year -= 1;
        break;
    case CursorPosition_Month:
        setting_time.tm_mon -= 1;
        if(setting_time.tm_mon < 0) setting_time.tm_mon = 11; //月份范围是0-11
        break;
    case CursorPosition_Day:
        setting_time.tm_mday -= 1;
        if(setting_time.tm_mday < 1) setting_time.tm_mday = 31; //日期范围是1-31
        break;
    case CursorPosition_Hour:
        setting_time.tm_hour -= 1;
        if(setting_time.tm_hour < 0) setting_time.tm_hour = 23; //小时范围是0-23
        break;
    
    case CursorPosition_Minute:
        setting_time.tm_min -= 1;
        if(setting_time.tm_min < 0) setting_time.tm_min = 59; //分钟范围是0-59
        break;
    case CursorPosition_Second:
        setting_time.tm_sec -= 1;
        if(setting_time.tm_sec < 0) setting_time.tm_sec = 59; //秒钟范围是0-59
        break;
    }
    
}

/*按键按下时执行下面这个操作*/
void EncoderButtonPressed(void)
{
    /*按键每按下一次就从当前状态改变为另一状态*/
    if(calenderMode == CalenderMode_Normal)
    {
        /*获取当前时间, 以便在设置模式下进行修改. 注意KK_RTC_GetTime()返回的是一个指针, 所以要解引用*/
        setting_time = *KK_RTC_GetTime(); 
        cursor_position = CursorPosition_Year; //每次进入设置模式时, 光标都从年份开始
        calenderMode = CalenderMode_Setting;
    }
    else
    {
        if(cursor_position == CursorPosition_Second){
            KK_RTC_SetTime(&setting_time);
            calenderMode = CalenderMode_Normal;
        }
        else
        {
            cursor_position++; //光标位置后移
        }
        
    }
}

/* 初始化主任务 */
void MainTaskInit(void)
{
    HAL_Delay(20);
    OLED_Init();
    EncoderInit();
    EncoderSetOnBackwardCallback(EncoderBackward);  //在初始化时设置编码器反转时要调用的函数,测试能不能成功调用 
                                                    //只写函数名,不加括号时表示这个函数的地址
    EncoderSetOnForwardCallback(EncoderForward);
    EncoderSetOnButtonPressedCallback(EncoderButtonPressed);
}
/* 使用OLED屏幕,显示时间和日期.对OLED屏幕显示时间的操作都要放在这里 */
void ShowTime(struct tm *now){
    /*显示日期*/
    char dateStr[50];
    sprintf(dateStr, "%04d-%02d-%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
    OLED_PrintASCIIString(24, 0, dateStr, &afont16x8, OLED_COLOR_NORMAL);

    /*显示时间*/
    sprintf(dateStr, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
    OLED_PrintASCIIString(16, 20, dateStr, &afont24x12, OLED_COLOR_NORMAL);

    /*显示星期*/
    char *week = weekDays[now->tm_wday];
    uint8_t x_week = ( 128 - strlen(week) * 8 ) / 2; // 计算星期字符串的起始x坐标，使其居中对齐
    OLED_PrintASCIIString(x_week, 48, week, &afont16x8, OLED_COLOR_NORMAL);
}

/* 在OELDE屏幕显示光标位置, 每隔一段时间闪一下 */
void ShowCursorPosition()
{
    static uint32_t time_recording = 0;
    uint32_t tiem_interval = GetTick() - time_recording;
    if(tiem_interval >= CURSOR_BLINK_TIME * 2)
    {
        time_recording = GetTick();
    }
    else if (tiem_interval >= CURSOR_BLINK_TIME)
    {
        CursorPositionCoordinate position = cursor_position_coordinate[cursor_position];
        OLED_DrawLine(position.x1, position.y1, position.x2, position.y2, OLED_COLOR_NORMAL);
        
    }
    /*注意这里不需要再写关于清除下标的操作了, 因为在main.c的while循环里面会调用OLED_NewFrame()函数, 会把上一次显示的OLED屏幕内容清除掉.
    而当这里不符合显示光标的条件时, 光标就不会出现*/
    

}


void MainTask(void)
{
    EncoderJudge(); //每次while循环都要判断编码器状态, 以免漏掉旋转动作
    OLED_NewFrame();

    if(calenderMode == CalenderMode_Normal)
    {
        struct tm *now = KK_RTC_GetTime();
        ShowTime(now);
    }
    else{
        ShowTime(&setting_time);
        ShowCursorPosition();
    }


    OLED_ShowFrame();
}