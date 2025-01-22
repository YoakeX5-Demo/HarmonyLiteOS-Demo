/*
 * Copyright (c) 2020 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"

#include "iot_gpio_ex.h"

#define LED_GPIO 2
#define Button_F1_GPIO 11
#define Button_F2_GPIO 12
//设置LED状态
static enum LedState
{
    LED_ON = 0,
    LED_OFF,
    LED_BLINK,
};

static enum LedState g_LedState = LED_BLINK;
//按下F1按钮改变LED状态
static void F1_Pressed(char *arg)
{
    (void)arg;
    g_LedState = LED_ON;
}
//按下F2按钮改变LED状态
static void F2_Pressed(char *arg)
{
    (void)arg;
    g_LedState = LED_OFF;
}

static void LedTask(void)
{
    // 初始化GPIO_2
    IoTGpioInit(LED_GPIO);
    // 设置GPIO_2为输出模式
    IoTGpioSetDir(LED_GPIO, IOT_GPIO_DIR_OUT);

    // 按键F1 GPIO初始化
    //IoTGpioInit(Button_F1_GPIO);
    //初始化F1按钮，设置为下降沿触发中断
    IoTGpioSetFunc(Button_F1_GPIO,IOT_GPIO_FUNC_GPIO_11_GPIO);
    // 按键F1 GPIO设置为输入方向
    IoTGpioSetDir(Button_F1_GPIO, IOT_GPIO_DIR_IN);
    // 按键F1 GPIO设置为上拉模式
    IoTGpioSetPull(Button_F1_GPIO, IOT_GPIO_PULL_UP);
    // 注册按键F1 的中断回调函数 边沿触发 高转低
    IoTGpioRegisterIsrFunc(Button_F1_GPIO, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, F1_Pressed, NULL);
   
    // 按键F2 GPIO初始化
    //IoTGpioInit(Button_F2_GPIO);
    //初始化F2按钮，设置为下降沿触发中断
    IoTGpioSetFunc(Button_F2_GPIO,IOT_GPIO_FUNC_GPIO_12_GPIO);
    // 按键F2 GPIO设置为输入方向
    IoTGpioSetDir(Button_F2_GPIO, IOT_GPIO_DIR_IN);
    // 按键F2 GPIO设置为上拉模式
    IoTGpioSetPull(Button_F2_GPIO, IOT_GPIO_PULL_UP);
    // 注册按键F2 的中断回调函数 边沿触发 高转低
    IoTGpioRegisterIsrFunc(Button_F2_GPIO, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, F2_Pressed, NULL);

    while (1)
    {
        switch (g_LedState)
        {
        case LED_ON:
            // 设置GPIO_2输出高电平点亮LED灯
            IoTGpioSetOutputVal(LED_GPIO, 1);
            osDelay(100);
            break;
        case LED_OFF:
            // 设置GPIO_2输出低电平熄灭LED灯
            IoTGpioSetOutputVal(LED_GPIO, 0);
            osDelay(100);
            break;
        case LED_BLINK:
            // 设置GPIO_2交替输出高低电平，使LED闪烁
            IoTGpioSetOutputVal(LED_GPIO, 0);
            osDelay(100);
            IoTGpioSetOutputVal(LED_GPIO, 1);
            osDelay(100);
            break;
        default:
            osDelay(100);
            break;
        }
    }
}

int a02_01_LED_Lights(void)
{
    osThreadAttr_t attr;

    attr.name = "LedTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 512;
    attr.priority = osPriorityNormal;
    //创建LEDTask任务
    if (osThreadNew((osThreadFunc_t)LedTask, NULL, &attr) == NULL)
    {
        printf("Falied to create LedTask!\n");
    }
}