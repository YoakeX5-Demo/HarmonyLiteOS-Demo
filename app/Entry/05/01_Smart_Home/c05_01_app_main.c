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

#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_i2c.h"
#include "iot_errno.h"
#include <hi_wifi_api.h>
#include "NT3H.h"

#include "lwip/sockets.h"
#include "wifi_connect.h"

// 配置wifi账号密码，学员自行补充
#define CONFIG_WIFI_SSID "iPhone"
#define CONFIG_WIFI_PWD "12345678"

// 配置TCP服务端开放端口，学员自行补充
#define CONFIG_SERVER_PORT 5678

#define CONFIG_CLIENT_PORT 5678

#define TCP_BACKLOG 10

// 在sock_fd 进行监听，在 new_fd 接收新的链接
int sock_fd, new_fd;

extern int deal_command_task(void);
extern int report_message_task(void);
extern void Config_Property(void);

static void hardware_init(void)
{

    // GPIO_0复用为I2C1_SDA
    IoTGpioSetFunc(0, IOT_GPIO_FUNC_GPIO_0_I2C1_SDA);

    // GPIO_1复用为I2C1_SCL
    IoTGpioSetFunc(1, IOT_GPIO_FUNC_GPIO_1_I2C1_SCL);

    // 波特率: 400kbps
    IoTI2cInit(1, 400000);
}

static void set_macaddr(void)
{
    uint8_t buffer[16];
    uint8_t mac_addres[6];

    mac_addres[0] = 0x08;
    mac_addres[1] = 0x00;
    mac_addres[2] = 0x02;
    // 读取NFC芯片的序列号作为的MAC地址
    NT3HGetNxpSerialNumber(buffer);
    for (int i = 0; i < 3; i++)
    {
        mac_addres[i + 3] = buffer[i + 1];
    }
    // 设置MAC地址
    if (hi_wifi_set_macaddr((char *)mac_addres, 6) != 0)
    { /* 6 mac len */
        printf("set_macaddr failed!\n");
    }
    printf("set_macaddr succeed!\n");
}

/**************************************************
 * 函数：borad_init
 * 功能：初始化相关操作
 * ***********************************************/
static void borad_init(void)
{
    // 初始化硬件外设
    hardware_init();

    // 连接设置MAC地址
    set_macaddr();

    // 连接Wifi
    WifiConnect(CONFIG_WIFI_SSID, CONFIG_WIFI_PWD);

    // 向NFC里配置功能信息，供通用APP生成界面使用，与数据传输业务无关
    Config_Property();
}

/**************************************************
 * 函数：creat_tcp_server
 * 功能：创建TCP服务器
 * ***********************************************/
static int creat_tcp_server(void)
{
    // 服务端地址信息
    struct sockaddr_in server_sock;

    // 创建socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket is error\n");
        exit(1);
    }

    bzero(&server_sock, sizeof(server_sock));
    server_sock.sin_family = AF_INET;
    server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sock.sin_port = htons(CONFIG_CLIENT_PORT);

    // 调用bind函数绑定socket和地址
    if (bind(sock_fd, (struct sockaddr *)&server_sock, sizeof(struct sockaddr)) == -1)
    {
        printf("bind is error\n");
        exit(1);
    }

    // 调用listen函数监听(指定port监听)
    if (listen(sock_fd, TCP_BACKLOG) == -1)
    {
        printf("listen is error\n");
        exit(1);
    }
    return 0;
}

/**************************************************
 * 任务：main_task
 * 主任务
 * ***********************************************/
int c05_01_main_task(void)
{
    osThreadAttr_t attr;

    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 5;
    attr.name = "deal_command_task";

    // 外设初始化
    borad_init();

    // 创建TCP服务器
    if (creat_tcp_server() == 0)
    {

        // 创建命令处理任务
        attr.name = "deal_command_task";
        attr.priority = 24;
        if (osThreadNew((osThreadFunc_t)deal_command_task, NULL, &attr) == NULL)
        {
            printf("Falied to create deal_command_task!\n");
        }
        // 创建数据上报任务
        attr.name = "report_message_task";
        attr.priority = 24;
        if (osThreadNew((osThreadFunc_t)report_message_task, NULL, &attr) == NULL)
        {
            printf("Falied to create report_message_task!\n");
        }
    }
}