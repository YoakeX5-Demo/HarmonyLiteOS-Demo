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
#include "iot_pwm.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"

#define LED_GPIO 2
#define IOT_PWM_PORT_PWM2 2
#define IOT_GPIO_FUNC_GPIO_2_PWM2_OUT 5

static void PWMTask(void)
{
    unsigned int i;

    // 初始化GPIO
    IoTGpioInit(LED_GPIO);

    // 设置GPIO_2引脚复用功能为PWM
    IoTGpioSetFunc(LED_GPIO, IOT_GPIO_FUNC_GPIO_2_PWM2_OUT);

    // 设置GPIO_2引脚为输出模式
    IoTGpioSetDir(LED_GPIO, IOT_GPIO_DIR_OUT);

    // 初始化PWM端口
    IoTPwmInit(IOT_PWM_PORT_PWM2);

    while (1)
    {
        for (i = 0; i < 100; i++)
        {
            // 输出不同占空比的PWM波
            IoTPwmStart(IOT_PWM_PORT_PWM2, i, 4000);
            usleep(1000);
                }
        i = 0;
    }
}

int a02_02_LED_Breathe(void)
{
    osThreadAttr_t attr;

    attr.name = "PWMTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 512;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)PWMTask, NULL, &attr) == NULL)
    {
        printf("Falied to create PWMTask!\n");
    }
}