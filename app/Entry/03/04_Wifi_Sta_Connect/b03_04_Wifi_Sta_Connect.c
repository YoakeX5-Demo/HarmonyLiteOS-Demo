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

#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/ip4_addr.h"
#include "lwip/api_shell.h"

#include "cmsis_os2.h"
#include "wifi_device.h"
#include "wifi_error_code.h"
#include "ohos_init.h"

#define DEF_TIMEOUT 15
#define ONE_SECOND 1

static void WiFiInit(void);
static void WaitSacnResult(void);
static int WaitConnectResult(void);
static void OnWifiScanStateChangedHandler(int state, int size);
static void OnWifiConnectionChangedHandler(int state, WifiLinkedInfo *info);
static void OnHotspotStaJoinHandler(StationInfo *info);
static void OnHotspotStateChangedHandler(int state);
static void OnHotspotStaLeaveHandler(StationInfo *info);

static int g_staScanSuccess = 0;
static int g_ConnectSuccess = 0;
static int ssid_count = 0;
static WifiEvent g_wifiEventHandler = {0};
static WifiErrorCode error;

#define SELECT_WLAN_PORT "wlan0"

// 配置wifi账号密码，学员自行补充
#define SELECT_WIFI_SSID "iPhone"
#define SELECT_WIFI_PASSWORD "12345678"
#define SELECT_WIFI_SECURITYTYPE WIFI_SEC_TYPE_PSK

static BOOL WifiSTATask(void)
{
    WifiScanInfo *info = NULL;
    unsigned int size = WIFI_SCAN_HOTSPOT_LIMIT;
    static struct netif *g_lwip_netif = NULL;
    WifiDeviceConfig select_ap_config = {0};

    osDelay(200);
    printf("<--System Init-->\r\n");

    // 初始化WIFI，学员自行补充
    WiFiInit();

    // 使能WIFI，学员自行补充      启动wifi
    if (EnableWifi() != WIFI_SUCCESS)
    {
        printf("EnableWifi failed, error = %d\n", error);
        return -1;
    }

    // 判断WIFI是否激活
    if (IsWifiActive() == 0)
    {
        printf("Wifi station is not actived.\n");
        return -1;
    }

    // 分配空间，保存WiFi信息
    info = malloc(sizeof(WifiScanInfo) * WIFI_SCAN_HOTSPOT_LIMIT);
    if (info == NULL)
    {
        return -1;
    }

    // 轮询查找WiFi列表
    do
    {
        // 重置标志位
        ssid_count = 0;
        g_staScanSuccess = 0;

        // 开始扫描
        Scan();

        // 等待扫描结果
        WaitSacnResult();

        // 获取扫描列表
        error = GetScanInfoList(info, &size);

    } while (g_staScanSuccess != 1);

    // 打印WiFi列表
    printf("********************\r\n");
    for (uint8_t i = 0; i < ssid_count; i++)
    {
        printf("no:%03d, ssid:%-30s, rssi:%5d\r\n", i + 1, info[i].ssid, info[i].rssi / 100);
    }
    printf("********************\r\n");

    // 连接指定的WiFi热点
    for (uint8_t i = 0; i < ssid_count; i++)
    { // 检索所以热点  有没有这个id
        if (strcmp(SELECT_WIFI_SSID, info[i].ssid) == 0)
        {
            int result;

            printf("Select:%3d wireless, Waiting...\r\n", i + 1);

            // 拷贝要连接的热点信息
            strcpy(select_ap_config.ssid, info[i].ssid);
            strcpy(select_ap_config.preSharedKey, SELECT_WIFI_PASSWORD);
            select_ap_config.securityType = SELECT_WIFI_SECURITYTYPE;

            // 连接配置WiFi信息，学员自行补充
            if (AddDeviceConfig(&select_ap_config, &result) == WIFI_SUCCESS)
            {
                // 连接指定的WiFi，学员自行补充
                if (ConnectTo(result) == WIFI_SUCCESS && WaitConnectResult() == 1)
                {
                    printf("WiFi connect succeed!\r\n");
                    // 连接成功设置网卡
                    g_lwip_netif = netifapi_netif_find(SELECT_WLAN_PORT);
                    break;
                }
            }
        }

        if (i == ssid_count - 1)
        {
            printf("ERROR: No wifi as expected\r\n");
            while (1)
                osDelay(100);
        }
    }

    // 启动DHCP
    if (g_lwip_netif)
    {
        dhcp_start(g_lwip_netif);
        printf("begain to dhcp   \r\n");
    }

    // 等待DHCP
    for (;;)
    {
        if (dhcp_is_bound(g_lwip_netif) == ERR_OK)
        {
            printf("<-- DHCP state:OK -->\r\n");

            // 打印获取到的IP信息
            netifapi_netif_common(g_lwip_netif, dhcp_clients_info_show, NULL);
            break;
        }

        printf("<-- DHCP state:Inprogress -->\r\n");
        osDelay(100);
    }

    // 执行其他操作
    for (;;)
    {
        // 可以编写其他的业务
        osDelay(100);
    }
}

static void WiFiInit(void)
{
    printf("<--Wifi Init-->\r\n");
    // 回调函数的注册
    g_wifiEventHandler.OnWifiScanStateChanged = OnWifiScanStateChangedHandler;   // 扫描状态的回调函数
    g_wifiEventHandler.OnWifiConnectionChanged = OnWifiConnectionChangedHandler; // 连接状态的回调函数
    g_wifiEventHandler.OnHotspotStaJoin = OnHotspotStaJoinHandler;               // 热点的回调
    g_wifiEventHandler.OnHotspotStaLeave = OnHotspotStaLeaveHandler;
    g_wifiEventHandler.OnHotspotStateChanged = OnHotspotStateChangedHandler;
    error = RegisterWifiEvent(&g_wifiEventHandler);
    if (error != WIFI_SUCCESS)
    {
        printf("register wifi event fail!\r\n");
    }
    else
    {
        printf("register wifi event succeed!\r\n");
    }
}

static void OnWifiScanStateChangedHandler(int state, int size)
{
    (void)state;
    if (size > 0)
    {
        ssid_count = size;
        g_staScanSuccess = 1;
    }
    return;
}

static void OnWifiConnectionChangedHandler(int state, WifiLinkedInfo *info)
{
    (void)info;

    if (state > 0)
    {
        g_ConnectSuccess = 1;
        printf("callback function for wifi connect\r\n");
    }
    else
    {
        printf("connect error,please check password\r\n");
    }
    return;
}

static void OnHotspotStaJoinHandler(StationInfo *info)
{
    (void)info;
    printf("STA join AP\n");
    return;
}

static void OnHotspotStaLeaveHandler(StationInfo *info)
{
    (void)info;
    printf("HotspotStaLeave:info is null.\n");
    return;
}

static void OnHotspotStateChangedHandler(int state)
{
    printf("HotspotStateChanged:state is %d.\n", state);
    return;
}

static void WaitSacnResult(void)
{
    int scanTimeout = DEF_TIMEOUT;
    while (scanTimeout > 0)
    {
        sleep(ONE_SECOND);
        scanTimeout--;
        if (g_staScanSuccess == 1)
        {
            printf("WaitSacnResult:wait success[%d]s\n", (DEF_TIMEOUT - scanTimeout));
            break;
        }
    }
    if (scanTimeout <= 0)
    {
        printf("WaitSacnResult:timeout!\n");
    }
}

static int WaitConnectResult(void)
{
    int ConnectTimeout = DEF_TIMEOUT;
    while (ConnectTimeout > 0)
    {
        sleep(1);
        ConnectTimeout--;
        if (g_ConnectSuccess == 1)
        {
            printf("WaitConnectResult:wait success[%d]s\n", (DEF_TIMEOUT - ConnectTimeout));
            break;
        }
    }
    if (ConnectTimeout <= 0)
    {
        printf("WaitConnectResult:timeout!\n");
        return 0;
    }

    return 1;
}

int b03_04_Wifi_Sta_Connect(void)
{
    osThreadAttr_t attr;

    attr.name = "WifiSTATask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = 24;

    if (osThreadNew((osThreadFunc_t)WifiSTATask, NULL, &attr) == NULL)
    {
        printf("Falied to create WifiSTATask!\n");
    }
}