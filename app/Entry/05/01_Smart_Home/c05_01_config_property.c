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

#include "lwip/if_api.h"
#include "lwip/netifapi.h"
#include "nfc.h"
#include "profile_package.h"


static char *GetLocalWifiIp(void)
{
    int ret;

    struct netif *netif_node = netif_find("wlan0");
    if (netif_node == NULL) {
        printf("GetLocalWifiIp netif get fail\r\n");
        return -1;
    }

    ip4_addr_t ipAddr;

    ret = netifapi_netif_get_addr(netif_node, &ipAddr, NULL, NULL);
    if (ret == 0) {
        return ip4addr_ntoa(&ipAddr);
    }
}
//通用APP可根据下列参数生成界面使用，与数据传输业务无关
void Config_Property(void)
{
    char *msg;
    char *service;
    char *ip_str = NULL;
    char *port = NULL;
    uint8_t ret;
    profile_property_t    property1;
    profile_property_t    property2;
    profile_property_t    property3;
    profile_commad_t commad1;
    profile_commad_t commad2;

    service = "智能小屋";

    property1.name="光照强度";
    property1.propertyName = "Lux";
    property1.unit = "lux";
    property1.nxt = &property2;

    property2.name="环境湿度";
    property2.propertyName = "Humidity";
    property2.unit = "%";
    property2.nxt = &property3;

    property3.name="环境温度";
    property3.propertyName = "Temperature";
    property3.unit = "℃";
    property3.nxt = NULL;


    commad1.name = "风扇";
    commad1.commadName = "Motor";
    commad1.type ="string";
    commad1.nxt = &commad2;

    commad2.name = "灯";
    commad2.commadName = "Light";
    commad2.type ="string";
    commad2.nxt = NULL;

    ip_str = GetLocalWifiIp(); 
    port = "5678";
    msg = profile_package_property(service,&property1,&commad1,ip_str,port);
    
    //将参数写入到NFC中
    ret = storeText(NDEFFirstPos, msg);
    if (ret != 1)
    {
        printf("NFC Write Data Falied :%d ", ret);
    }

}