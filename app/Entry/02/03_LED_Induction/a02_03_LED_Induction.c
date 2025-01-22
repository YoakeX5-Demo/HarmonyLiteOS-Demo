#include <stdio.h>
#include "ohos_init.h"
#include "hi_adc.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"


#include "ohos_init.h" // HarmonyOS 应用程序开发所需的初始化和管理函数的声明和定义。
#include <unistd.h>    //usleep延时使用到的初始化库
#include "cmsis_os2.h" //微处理器软件接口标准：线程优先级
#include <string.h>

/*
2、传感器的模拟信号如何转换成数字信号?
    ADC可以将连续变化的模拟信号转换为离散的数字信号，以便数字系统进行处理。
        转换的过程包括采样和量化两个步骤：采样是指以一定的时间间隔对模拟信号进行采集；
        量化是指将采样到的连续信号离散化为一系列离散的数字数值。

3、ADC 有哪些 API 接口?
    ADC的API接口可以根据具体的硬件和开发平台而有所差异。以下是一些常见的ADC接口：
    SPI（Serial Peripheral Interface）：SPI接口是一种串行通信接口，通过使用时钟、数据输入、数据输出和片选等信号线，与MCU或其他设备进行通信。一些ADC芯片具有SPI接口，可以通过SPI总线与MCU进行通信，读取转换结果。
    I2C（Inter-Integrated Circuit）：I2C接口也是一种串行通信接口，常用于连接多个设备到同一总线上。一些ADC芯片支持I2C接口，可以通过I2C总线与MCU进行通信。
    UART（Universal Asynchronous Receiver-Transmitter）：UART接口是一种异步串行通信接口，常用于传输数据。某些ADC芯片具有UART接口，可以通过UART与MCU进行通信，传输转换结果。
*/

osThreadId_t adc_light_demo_id;

void adc_light_demo(void *arg)
{
    (void)arg;
    osDelay(10);
    IoTGpioInit(2);

    IoTGpioSetFunc(2, IOT_GPIO_FUNC_GPIO_2_GPIO);
    IoTGpioSetDir(2, 1);

    while (1) 
    {
        int data = 0;
        float DB = 0;
        // 读取ADC信息
        /*********************************************************************************
        channel 	表示ADC通道.
        data 	表示指向存储读取数据的地址的指针
        equModel 	表示平均算法的次数
        curBais 	表示模拟功率控制模式
        rstCnt 	指示从重置到转换开始的时间计数。一次计数等于334纳秒。值的范围必须从0到0xFF
        *********************************************************************************/
        if (hi_adc_read(HI_ADC_CHANNEL_4, &data, HI_ADC_EQU_MODEL_8, HI_ADC_CUR_BAIS_DEFAULT, 0) == IOT_SUCCESS)
        {   
            DB = ((float)data * (1.8*4))/4096;
            // 串口输出光照强度的感应信息，有光环境输出值位于120附近，无光环境输出值位于1000附近
            printf("adc_light_value = %f \r\n", DB);
            if (DB > 0.25)
            {
                IoTGpioSetOutputVal(2, 1);
            }
            else
            {
                IoTGpioSetOutputVal(2, 0);
            }
            osDelay(100);
        }
    }
}

int a02_03_LED_Induction(void) // 任务创建
{
    osThreadAttr_t attr;
    attr.name = "adc_light_demo";      // 设置线程的名称为"thread1"
    attr.attr_bits = 0U;        // attr_bits用于设置线程的其他属性，这里将其设置为0
    attr.cb_mem = NULL;         // 内存线程控制块地址：是一个操作控制块的指针
    attr.cb_size = 0U;          // 内存大小：0u为默认
    attr.stack_mem = NULL;      // 内存堆栈地址：堆栈的指针
    attr.stack_size = 1024 * 4; // 堆栈大小
    attr.priority = 25;         // 优先级为25，数字越大优先级越大

    adc_light_demo_id = osThreadNew((osThreadFunc_t)adc_light_demo, NULL, &attr);      // 创建线程（任务函数，作为启动参数传递给任务的指针，参数列表）
    if (adc_light_demo_id == NULL) 
    {
        printf("Falied to create thread1!\n");
    }

}
