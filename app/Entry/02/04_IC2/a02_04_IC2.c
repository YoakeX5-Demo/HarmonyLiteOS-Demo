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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include <netinet/in.h>
#include "sockets.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_i2c.h"
#include "iot_errno.h"
#include <hi_wifi_api.h>




#include "E53_IA1.h"

//开发板中的气压传感器是如何把数据传输给主芯片?
//I2C：I2C 是一种串行通信协议，使用两根线路（SDA 和 SCL）进行数据传输。
//主芯片作为 I2C 主设备，通过发送特定的控制信号（如起始信号、地址和数据）来请求传感器数据。
//传感器作为 I2C 从设备，接收主设备发送的命令，并将传感器数据通过 I2C 总线返回给主芯片。

//Harmonyos 12C的API 有哪些?
//I2cInit 初始化I2C
//I2cDeinit 取消I2C初始化
//I2cWrite 将数据写入到I2C设备
//I2cRead 从I2C设备读取数据
//iot_i2c_ex.h接口介绍
//I2cWriteread 向I2C设备发送数据并接受数据响应
//I2cSetBaudrate 设置I2C频率

/**************************************************
 * 任务：report_message_task
 * 上报传感器任务
 * ***********************************************/
static int report_message_task(void)
{
    E53_IA1_Data_TypeDef data;

    //初始化传感器
    E53_IA1_Init();

    while (1)
    {
        E53_IA1_Read_Data(&data);
        printf("SENSOR:lux:%.2f temp:%.2f hum:%.2f\n", data.Lux, data.Temperature, data.Humidity);
        
        sleep(2);
    }
    return 0;
}

int a02_04_IC2(void)
{

    osThreadAttr_t attr;

    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 10;
    /* 创建初始化任务 */
    attr.name = "report_message_task";
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)report_message_task, NULL, &attr) == NULL)
    {
        printf("Falied to create report_message_task!\n");
    }
}