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

#include "cmsis_os2.h"
#include "lwip/sockets.h"
#include "profile_package.h"
#include "E53_IA1.h"

typedef struct
{
    int lux;
    int temp;
    int hum;
} report_t;

extern int new_fd, sock_fd, motor_status, light_status;
;
int is_accepted;
/**************************************************
 * deal_report_msg
 * 处理数据，并使用TCP协议上报数据
 * ***********************************************/
static void deal_report_msg(report_t *report)
{
    char *msg;
    uint8_t ret;
    profile_service_t service;
    profile_kv_t temperature;
    profile_kv_t humidity;
    profile_kv_t lux;
    profile_kv_t motor;
    profile_kv_t light;

    // 配置要发送的数据,学员自行补充
    service.event_time = NULL;
    service.service_id = "Smart_Home";
    service.service_property = &lux;
    service.nxt = NULL;

    lux.key = "Lux";
    lux.value = &report->lux;
    lux.type = PROFILE_VALUE_INT;
    lux.nxt = &temperature;
    temperature.key = "Temperature";
    temperature.value = &report->temp;
    temperature.type = PROFILE_VALUE_INT;
    temperature.nxt = &humidity;
    humidity.key = "Humidity";
    humidity.value = &report->hum;
    humidity.type = PROFILE_VALUE_INT;
    humidity.nxt = &motor;
    motor.key = "Motor";
    motor.value = motor_status ? "ON" : "OFF";
    motor.type = PROFILE_VALUE_STRING;
    motor.nxt = &light;
    light.key = "Light";
    light.value = light_status ? "ON" : "OFF";
    light.type = PROFILE_VALUE_STRING;
    light.nxt = NULL;

    // 打包数据,学员自行补充
    msg = profile_package_propertyreport(&service);

    // 发送数据,学员自行补充
    if (send(new_fd, msg, strlen(msg), 0) < 0)
    {
        printf("send error\r\n");
        close(new_fd);
    }
    return;
}

/**************************************************
 * 任务：report_message_task
 * 上报传感器任务
 * ***********************************************/
int report_message_task(void)
{
    report_t *report_msg;
    E53_IA1_Data_TypeDef data;

    // 客户端地址信息
    struct sockaddr_in client_sock;
    int sin_size;

    /* 初始化传感器 */

    E53_IA1_Init();

    while (1)
    {

        E53_IA1_Read_Data(&data);
        printf("SENSOR:lux:%.2f temp:%.2f hum:%.2f\n", data.Lux, data.Temperature, data.Humidity);

        if (is_accepted == 1)
        {
            report_msg = malloc(sizeof(report_t));

            if (NULL != report_msg)
            {
                /* 打包数据并将数据发送到队列中 */
                report_msg->hum = (int)data.Humidity;
                report_msg->temp = (int)data.Temperature;
                report_msg->lux = (int)data.Lux;
                deal_report_msg(report_msg);
            }
            free(report_msg);
        }

        sleep(3);
    }
    return 0;
}