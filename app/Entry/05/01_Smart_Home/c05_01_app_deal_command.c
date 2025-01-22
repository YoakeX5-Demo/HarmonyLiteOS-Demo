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

#include "lwip/sockets.h"
#include "E53_IA1.h"

extern int new_fd, sock_fd, is_accepted;
int motor_status, light_status;
char recvbuf[512];
/**************************************************
 * 任务：deal_command_task
 * 命令处理任务
 * ***********************************************/
int deal_command_task(void)
{
    ssize_t ret;
    // 客户端地址信息
    struct sockaddr_in client_sock;
    int sin_size;
    while (1)
    {

        printf("start accept\n");

        // 调用accept函数从队列中

        sin_size = sizeof(struct sockaddr_in);

        if ((new_fd = accept(sock_fd, (struct sockaddr *)&client_sock, (socklen_t *)&sin_size)) == -1)
        {
            perror("accept");
        }
        printf("accept succeed\n");
        is_accepted = 1;
        while (1)
        {
            bzero(recvbuf, sizeof(recvbuf));
            // 接收数据,并根据命令对相应的设备执行控制命令，学员自行补充
            if (recv(new_fd, recvbuf, sizeof(recvbuf), 0) < 0)
            {
                printf("recv error\r\n");
                close(new_fd);
                is_accepted = 0;
                break;
            }
            else
            {
                if (NULL != strstr(recvbuf, "Motor"))
                {
                    // 开启风扇
                    if (NULL != strstr(recvbuf, "ON"))
                    {
                        Motor_StatusSet(ON);
                        motor_status = 1;
                        printf("Motor On!\r\n");
                    }

                    // 关闭风扇
                    else if (NULL != strstr(recvbuf, "OFF"))
                    {
                        Motor_StatusSet(OFF);
                        motor_status = 0;
                        printf("Motor off!\r\n");
                    }
                }
                else if (NULL != strstr(recvbuf, "Light"))
                {
                    // 开启灯
                    if (NULL != strstr(recvbuf, "ON"))
                    {
                        Light_StatusSet(ON);
                        light_status = 1;
                        printf("Light On!\rin");
                    }
                    // 关闭灯
                    else if (NULL != strstr(recvbuf, "OFF"))
                    {
                        Light_StatusSet(OFF);
                        light_status = 0;
                        printf("Light Off!\rn");
                    }
                }
                //   usleep(2000000);
            }
        }
    }
}