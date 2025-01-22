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

#include "lwip/sockets.h"
#include "wifi_connect.h"

// 配置wifi账号密码，学员自行补充
#define CONFIG_WIFI_SSID "iPhone"
#define CONFIG_WIFI_PWD "12345678"
// 配置服务器IP和端口 ，学员自行补充
#define CONFIG_SERVER_ADDR "192.168.125.96"
#define CONFIG_SERVER_PORT 5678

// 在sock_fd 进行监听，在 new_fd 接收新的链接
static int sock_fd;
static int addr_length;
static const char *send_data = "Hello! I'm BearPi-HM_Nano TCP Client!\r\n";

static void TCPClientTask(void)
{
    // 服务器的地址信息

    uint8_t ret;

    struct sockaddr_in send_addr;
    socklen_t addr_length = sizeof(send_addr);
    char recvBuf[512];

    printf("Connect WIFI ing...\r\n");
    // 连接Wifi
    WifiConnect(CONFIG_WIFI_SSID, CONFIG_WIFI_PWD);

    // 创建socket，学员自行补充

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) // AF_INET IPV4的协议   AF_INET6 ipv6,  SOCK_STREAM:UDP协议
    {
        perror("create socket failed!\r\n");
        exit(1);
    }
    // 初始化预连接的服务端地址，学员自行补充

    send_addr.sin_family = AF_INET;                            // 协议组
    send_addr.sin_port = htons(CONFIG_SERVER_PORT);            // 端口号  htons将这个主机字节转换网络字节顺序    小端转大端模式
    send_addr.sin_addr.s_addr = inet_addr(CONFIG_SERVER_ADDR); // ip地址
    addr_length = sizeof(send_addr);
    // 连接到服务器
    connect(sock_fd, (struct sockaddr_in *)&send_addr, addr_length);
    while (1)
    {
        bzero(recvBuf, sizeof(recvBuf));
        // 发送 数据到服务端
        if (send(sock_fd, send_data, strlen(send_data), 0) < 0)
        {
            printf("send error\r\n");
            goto disconnect;
        }
        // 接收服务端返回字符串
        if (recv(sock_fd, recvBuf, sizeof(recvBuf), 0) < 0)
        {
            printf("recv error\r\n");
            goto disconnect;
        }
        printf("recv :%s\r\n", recvBuf);
    }
disconnect:
    // 关闭这个 socket
    printf("closesocket!\r\n");
    close(sock_fd);
closesocket:
    // 关闭这个 socket
    printf("closesocket!\r\n");
    close(sock_fd);
}

int b03_05_Tcp_Client(void)
{
    osThreadAttr_t attr;

    attr.name = "TCPClientTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)TCPClientTask, NULL, &attr) == NULL)
    {
        printf("[TCPClientDemo] Falied to create TCPClientTask!\n");
    }
}