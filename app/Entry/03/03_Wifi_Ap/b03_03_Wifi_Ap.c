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

#include "cmsis_os2.h"
#include "ohos_init.h"

#include "wifi_device.h"
#include "wifi_hotspot.h"
#include "wifi_error_code.h"
#include "lwip/netifapi.h"

// 配置wifi热点账号密码 ，学员自行补充
#define AP_SSID "Bearpi"
#define AP_PSK "12345678"

#define ONE_SECOND 1
#define DEF_TIMEOUT 15

static void OnHotspotStaJoinHandler(StationInfo *info);
static void OnHotspotStateChangedHandler(int state);
static void OnHotspotStaLeaveHandler(StationInfo *info);

static struct netif *g_lwip_netif = NULL;
static int g_apEnableSuccess = 0;
static WifiEvent g_wifiEventHandler = {0};
static WifiErrorCode error;

static BOOL WifiAPTask(void)
{
    // 延时2S便于查看日志
    osDelay(200);

    // 注册wifi事件的回调函数
    g_wifiEventHandler.OnHotspotStaJoin = OnHotspotStaJoinHandler;           // sta站点回调的事件
    g_wifiEventHandler.OnHotspotStaLeave = OnHotspotStaLeaveHandler;         // 站点退出的回调事件
    g_wifiEventHandler.OnHotspotStateChanged = OnHotspotStateChangedHandler; // wifi热点模式的回调
    error = RegisterWifiEvent(&g_wifiEventHandler);                          // 注册wifi事件的回调函数
    if (error != WIFI_SUCCESS)
    {
        printf("RegisterWifiEvent failed, error = %d.\r\n", error);
        return -1;
    }
    printf("RegisterWifiEvent succeed!\r\n");

    // 设置指定的热点配置 ，学员自行补充
    HotspotConfig config = {0};

    strcpy(config.ssid, AP_SSID);            //    ssid
    strcpy(config.preSharedKey, AP_PSK);     // 密钥
    config.securityType = WIFI_SEC_TYPE_PSK; // 加密方式
    config.band = HOTSPOT_BAND_TYPE_2G;      // 频段
    config.channelNum = 7;                   // 通道

    error = SetHotspotConfig(&config);
    if (error != WIFI_SUCCESS)
    {
        printf("SetHotspotConfig failed, error = %d.\r\n", error);
        return -1;
    }
    printf("SetHotspotConfig succeed!\r\n");

    printf("SetHotspotConfig succeed!\r\n");

    // 启动wifi热点模式
    error = EnableHotspot();
    if (error != WIFI_SUCCESS)
    {
        printf("EnableHotspot failed, error = %d.\r\n", error);
        return -1;
    }
    printf("EnableHotspot succeed!\r\n");

    // 检查热点模式是否使能
    if (IsHotspotActive() == WIFI_HOTSPOT_NOT_ACTIVE)
    {
        printf("Wifi station is not actived.\r\n");
        return -1;
    }
    printf("Wifi station is actived!\r\n");

    // 启动dhcp
    g_lwip_netif = netifapi_netif_find("ap0");
    if (g_lwip_netif)
    {
        ip4_addr_t bp_gw;
        ip4_addr_t bp_ipaddr;
        ip4_addr_t bp_netmask;

        IP4_ADDR(&bp_gw, 192, 168, 1, 1);        /* input your gateway for example: 192.168.1.1 */
        IP4_ADDR(&bp_ipaddr, 192, 168, 1, 1);    /* input your IP for example: 192.168.1.1 */
        IP4_ADDR(&bp_netmask, 255, 255, 255, 0); /* input your netmask for example: 255.255.255.0 */

        err_t ret = netifapi_netif_set_addr(g_lwip_netif, &bp_ipaddr, &bp_netmask, &bp_gw);
        if (ret != ERR_OK)
        {
            printf("netifapi_netif_set_addr failed, error = %d.\r\n", ret);
            return -1;
        }
        printf("netifapi_netif_set_addr succeed!\r\n");
        netifapi_dhcps_stop(g_lwip_netif); // 先把dhcp关闭!!否则开启dhcp会产生内存报错

        ret = netifapi_dhcps_start(g_lwip_netif, 0, 0);
        if (ret != ERR_OK)
        {
            printf("netifapi_dhcp_start failed, error = %d.\r\n", ret);
            return -1;
        }
        printf("netifapi_dhcps_start succeed!\r\n");
    }

    /****************以下为UDP服务器代码***************/
    // 在sock_fd 进行监听
    int sock_fd;
    // 服务端地址信息
    struct sockaddr_in server_sock;

    // 创建socket
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket is error.\r\n");
        return -1;
    }

    bzero(&server_sock, sizeof(server_sock));
    server_sock.sin_family = AF_INET;
    server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sock.sin_port = htons(8888);

    // 调用bind函数绑定socket和地址
    if (bind(sock_fd, (struct sockaddr *)&server_sock, sizeof(struct sockaddr)) == -1)
    {
        perror("bind is error.\r\n");
        return -1;
    }

    int ret;
    char recvBuf[512] = {0};
    // 客户端地址信息
    struct sockaddr_in client_addr;
    int size_client_addr = sizeof(struct sockaddr_in);
    while (1)
    {

        printf("Waiting to receive data...\r\n");
        memset(recvBuf, 0, sizeof(recvBuf));
        ret = recvfrom(sock_fd, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&client_addr, (socklen_t *)&size_client_addr);
        if (ret < 0)
        {
            printf("UDP server receive failed!\r\n");
            return -1;
        }
        printf("receive %d bytes of data from ipaddr = %s, port = %d.\r\n", ret, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("data is %s\r\n", recvBuf);
        ret = sendto(sock_fd, recvBuf, strlen(recvBuf), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        if (ret < 0)
        {
            printf("UDP server send failed!\r\n");
            return -1;
        }
    }
    /*********************END********************/
}

static void HotspotStaJoinTask(void)
{
    static char macAddress[32] = {0};
    StationInfo stainfo[WIFI_MAX_STA_NUM] = {0};
    StationInfo *sta_list_node = NULL;
    unsigned int size = WIFI_MAX_STA_NUM;

    error = GetStationList(stainfo, &size); // 闭源代码
    if (error != WIFI_SUCCESS)
    {
        printf("HotspotStaJoin:get list fail, error is %d.\r\n", error);
        return;
    }
    sta_list_node = stainfo;
    for (uint32_t i = 0; i < size; i++, sta_list_node++)
    {
        unsigned char *mac = sta_list_node->macAddress;
        snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        printf("HotspotSta[%d]: macAddress=%s.\r\n", i, macAddress);
    }
    g_apEnableSuccess++;
}

// GetStationList 回调函数里直接调用的会卡死， 所以需要创建任务
static void OnHotspotStaJoinHandler(StationInfo *info)
{
    if (info == NULL)
    {
        printf("HotspotStaJoin:info is null.\r\n");
    }
    else
    {
        printf("New Sta Join\n");
        osThreadAttr_t attr;
        attr.name = "HotspotStaJoinTask";
        attr.attr_bits = 0U;
        attr.cb_mem = NULL;
        attr.cb_size = 0U;
        attr.stack_mem = NULL;
        attr.stack_size = 2048;
        attr.priority = 24;
        if (osThreadNew((osThreadFunc_t)HotspotStaJoinTask, NULL, &attr) == NULL)
        {
            printf("HotspotStaJoin:create task fail!\r\n");
        }
    }
    return;
}

static void OnHotspotStaLeaveHandler(StationInfo *info)
{
    if (info == NULL)
    {
        printf("HotspotStaLeave:info is null.\r\n");
    }
    else
    {
        static char macAddress[32] = {0};
        unsigned char *mac = info->macAddress;
        snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        printf("HotspotStaLeave: macAddress=%s, reason=%d.\r\n", macAddress, info->disconnectedReason);
        g_apEnableSuccess--;
    }
    return;
}

static void OnHotspotStateChangedHandler(int state)
{
    printf("HotspotStateChanged:state is %d.\r\n", state);
    if (state == WIFI_HOTSPOT_ACTIVE)
    {
        printf("wifi hotspot active.\r\n");
    }
    else
    {
        printf("wifi hotspot noactive.\r\n");
    }
}

int b03_03_Wifi_Ap(void)
{
    osThreadAttr_t attr;

    attr.name = "WifiAPTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = 25;

    if (osThreadNew((osThreadFunc_t)WifiAPTask, NULL, &attr) == NULL)
    {
        printf("Falied to create WifiAPTask!\r\n");
    }
}
