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
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_uart.h"

#define UART_TASK_STACK_SIZE 1024 * 8
#define UART_TASK_PRIO 25
#define UART_BUFF_SIZE 1000
#define IOT_UART_IDX_1 1

static const char *data = "Hello, BearPi!";

static void UART_Task(void)
{
    uint8_t uart_buff[UART_BUFF_SIZE] = {0};
    uint8_t *uart_buff_ptr = uart_buff;
    uint32_t ret;

    //初始化串口1
    IotUartAttribute uart_attr = {
        //波特率: 9600
        .baudRate = 9600,
        //数据位：8bits
        .dataBits = 8,
        //停止位
        .stopBits = 1,
        //奇偶校验位
        .parity = 0,
    };
    //初始化uart
    ret = IoTUartInit(IOT_UART_IDX_1,&uart_attr);

    if (ret != IOT_SUCCESS)
    {
        printf("Failed to init uart! Err code = %d\n", ret);
        return;
    }

    printf("UART Test Start\n");
    while (1)
    {
        printf("=======================================\n");
        printf("*************UART_example**************\n");
        printf("=======================================\n");

        //通过串口1发送数据
        IoTUartWrite(IOT_UART_IDX_1, (unsigned char *)data, strlen(data));

        //通过串口1接收数据，并打印出来
        IoTUartRead(IOT_UART_IDX_1, uart_buff_ptr, UART_BUFF_SIZE);
        printf("Uart1 read data:%s", uart_buff_ptr);

        usleep(1000000);
    }
}

int a02_06_UART_Message(void)
{

    osThreadAttr_t attr;

    attr.name = "UART_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = UART_TASK_STACK_SIZE;
    attr.priority = UART_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)UART_Task, NULL, &attr) == NULL)
    {
        printf("[ADCExample] Falied to create UART_Task!\n");
    }
}