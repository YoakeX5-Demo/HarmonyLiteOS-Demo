#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "iot_gpio.h"
#include "iot_gpio_ex.h"

#define Button_F1_GPIO 11
#define Button_F2_GPIO 12
extern void a02_01_LED_Lights(void);
extern void a02_02_LED_Breathe(void);
extern void a02_03_LED_Induction(void);
extern void a02_04_IC2(void);
extern void a02_06_UART_Message(void);
extern void b03_01_Kv_Store(void);
extern void b03_02_Utils_File(void);
extern void b03_03_Wifi_Ap(void);
extern void b03_04_Wifi_Sta_Connect(void);
extern void b03_05_Tcp_Client(void);
extern void c05_01_main_task(void);
// 按键状态
static enum BtnState {
    F1 = 0,
    F2,
    BTN_WAIT,
};

// 默认按键状态
static enum BtnState g_BtnState = BTN_WAIT;

// static const char *data = "Hello, BearPi!";

// F1按钮触发，按键状态改变为F1按下
static void F1_Pressed(void)
{
    g_BtnState = F1;
}

// F2按钮触发，按键状态改变为F2按下
static void F2_Pressed(void)
{
    g_BtnState = F2;
}

static void Btn_Init(void)
{
    // 初始化按钮
    // 按键F1 GPIO初始化
    IoTGpioInit(Button_F1_GPIO);

    // 初始化F1按钮，设置为下降沿触发中断
    IoTGpioSetFunc(Button_F1_GPIO, IOT_GPIO_FUNC_GPIO_11_GPIO);

    // 按键F1 GPIO设置为输入方向
    IoTGpioSetDir(Button_F1_GPIO, IOT_GPIO_DIR_IN);
    // 按键F1 GPIO设置为上拉模式
    IoTGpioSetPull(Button_F1_GPIO, IOT_GPIO_PULL_UP);
    // 注册按键F1 的中断回调函数 边沿触发 高转低
    IoTGpioRegisterIsrFunc(Button_F1_GPIO, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, F1_Pressed, NULL);

    // 按键F2 GPIO初始化
    IoTGpioInit(Button_F2_GPIO);

    // 初始化F2按钮，设置为下降沿触发中断
    IoTGpioSetFunc(Button_F2_GPIO, IOT_GPIO_FUNC_GPIO_12_GPIO);

    // 按键F2 GPIO设置为输入方向
    IoTGpioSetDir(Button_F2_GPIO, IOT_GPIO_DIR_IN);
    // 按键F2 GPIO设置为上拉模式
    IoTGpioSetPull(Button_F2_GPIO, IOT_GPIO_PULL_UP);
    // 注册按键F2 的中断回调函数 边沿触发 高转低
    IoTGpioRegisterIsrFunc(Button_F2_GPIO, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, F2_Pressed, NULL);
}

int selectNumber = 0;
int selectModule = -1;
int selectMax = 0;
// 模块选择界面（默认）
void S00_Gui(int i)
{
    char c[3];
    memset(c, ' ', 3);
    c[i] = '<';
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("===============00 模块选择===============\n");
    printf("02 驱动基础 %c\n", c[0]);
    printf("03 子系统开发入门 %c\n", c[1]);
    printf("05 综合实验 %c\n", c[2]);
    printf("========================================\n");
}
// 程序选择界面
void S02_Gui(int i)
{
    char c[6];
    memset(c, ' ', 6);
    c[i] = '<';
    ;
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("===============02 驱动基础===============\n");
    printf("01 LED_Lights 路灯控制 %c \n", c[0]);
    printf("02 LED_Breathe 呼吸灯 %c \n", c[1]);
    printf("03 LED_Induction 光照感应 %c \n", c[2]);
    printf("04 IC2 气象监测 %c \n", c[3]);
    printf("05 OLED_Show 屏幕显示 %c \n", c[4]);
    printf("06 UART_Message 数据通信 %c \n", c[5]);
    printf("========================================\n");
}
// 程序选择界面
void S03_Gui(int i)
{
    char c[5];
    memset(c, ' ', 5);
    c[i] = '<';
    ;
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("============03 子系统开发入门============\n");
    printf("01 Kv_Store WIFI账号密码管理 %c \n", c[0]);
    printf("02 Utils_File 文件操作 %c \n", c[1]);
    printf("03 Wifi_Ap WIFI操作-热点 %c \n", c[2]);
    printf("04 Wifi_Sta_Connect WIFI操作-连接 %c \n", c[3]);
    printf("05 Tcp_Client 通过WIFI进行网络通信 %c \n", c[4]);
    printf("========================================\n");
}
// 程序选择界面
void S05_Gui(int i)
{
    char c[1];
    memset(c, ' ', 1);
    c[i] = '<';
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("===============05 综合实验===============\n");
    printf("01 智慧小屋 %c \n", c[0]);
    printf("=======================================\n");
}
// 界面显示逻辑
static void Show_Gui(void)
{
    // 模块选择界面（默认）
    if (selectModule == -1)
    {
        // 最大选项
        selectMax = 3;
        S00_Gui(selectNumber);
    }
    // 02 驱动基础模块
    else if (selectModule == 0)
    {
        // 最大选项
        selectMax = 6;
        S02_Gui(selectNumber);
    }
    // 03 子系统开发入门模块
    else if (selectModule == 1)
    {
        // 最大选项
        selectMax = 5;
        S03_Gui(selectNumber);
    }
    // 05 综合实验模块
    else if (selectModule == 2)
    {
        // 最大选项
        selectMax = 1;
        S05_Gui(selectNumber);
    }
}

static void Select_Task(void)
{
    // 程序选择任务 开始
    printf("Select_Task Start\n");
    // 串口显示文本界面
    Show_Gui();
    while (1)
    {
        // 根据按钮状态判断按下的按钮
        switch (g_BtnState)
        {
        // 按键F1按下，进行选择
        case F1:
            // 重置按键到默认状态
            g_BtnState = BTN_WAIT;
            // 选择序号自增并取余
            selectNumber = ++selectNumber % selectMax;
            // 串口显示文本界面
            Show_Gui();
            osDelay(100);
            break;
        // 按键F2按下，进行确认
        case F2:
            // 重置按键到默认状态
            g_BtnState = BTN_WAIT;
            // 若未选择模块，根据选择序号，选择对应的模块
            if (selectModule == -1)
            {
                selectModule = selectNumber;
                selectNumber = 0; // 串口显示文本界面
                Show_Gui();
            }
            // 若已选择模块，根据选择序号，进入对应程序
            else
            {
                // 程序 开始
                switch (selectModule)
                {
                case 0:
                    switch (selectNumber)
                    {
                    case 0:
                        a02_01_LED_Lights();
                        break;
                    case 1:
                        a02_02_LED_Breathe();
                        break;
                    case 2:
                        a02_03_LED_Induction();
                        break;
                    case 3:
                        a02_04_IC2();
                        break;
                    case 5:
                        a02_06_UART_Message();
                        break;
                    }
                    break;
                case 1:
                    switch (selectNumber)
                    {
                    case 0:
                        b03_01_Kv_Store();
                        break;
                    case 1:
                        b03_02_Utils_File();
                        break;
                    case 2:
                        b03_03_Wifi_Ap();
                        break;
                    case 3:
                        b03_04_Wifi_Sta_Connect();
                        break;
                    case 4:
                        b03_05_Tcp_Client();
                        break;
                    }
                    break;
                case 2:
                    switch (selectNumber)
                    {
                    case 0:
                        c05_01_main_task();
                        break;
                    }
                }
                break;
            }

            osDelay(100);

        case BTN_WAIT:
            // 默认状态不执行操作
            osDelay(100);
            break;
        }
    }
}

static void ModuleSelect_Entry(void)
{
    // 程序选择入口程序 开始
    printf("ModuleSelect_Entry Start\n");
    // 按钮初始化
    Btn_Init();
    // 创建Select_Task任务
    osThreadAttr_t attr;
    attr.name = "Select_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 8192;
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)Select_Task, NULL, &attr) == NULL)
    {
        // 创建Select_Task任务失败
        printf("Falied to create Select_Task!\n");
    }
    // 提示Select_Entry任务完成
    printf("ModuleSelect_Entry Over\n");
}

APP_FEATURE_INIT(ModuleSelect_Entry);