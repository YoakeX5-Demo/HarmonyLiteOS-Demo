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
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "utils_file.h"

#define file_name "my_file"
#define wifi_config "ssid:Bearpi,pwd:0987654321"

static void Utils_File_Task(void)
{
    char buf[512] = {0};

    // 延时2S便于查看日志
    osDelay(200);

    // 1.创建一个文件，若已存在该文件，则删除后重新创建，学员自行补充
    uint32_t fd = UtilsFileOpen(file_name, O_CREAT_FS | O_TRUNC_FS | O_RDWR_FS, NULL);
    if (fd < 0)
    {
        printf("open file failed \n");
        return;
    }
    // 2.读取该文件内容，学员自行补充
    if (UtilsFileRead(fd, (char *)buf, sizeof(buf)) < 0)
    {
        printf("read file failed \n");
        return;
    }
    printf("---> first read file content is %s <---\n", buf);

    // 3.向文件中写入内容，学员自行补充
    printf("---> write content to a file <---\n");
    if (UtilsFileWrite(fd, wifi_config, strlen(wifi_config)) < 0)
    {
        printf("write file failed \n");
        return;
    }
    // 4.关闭该文件，学员自行补充
    if (UtilsFileClose(fd) < 0)
    {
        printf("close file failed \n");
        return;
    }
    // 5.打开文件，学员自行补充
    fd = UtilsFileOpen("my_file", O_RDWR_FS, NULL);
    if (fd < 0)
    {
        printf("open file failed\n");
        return;
    }
    // 6.读取该文件内容，学员自行补充
    if (UtilsFileRead(fd, (char *)buf, sizeof(buf)) < 0)
    {
        printf("read file failed \n");
        return;
    }
    printf("---> second read file content is %s <---\n", buf);

    // 7.文件定位从第五个字节开始，学员自行补充
    if (UtilsFileSeek(fd, 5, SEEK_SET_FS) < 0)
    {
        printf("seek file failed\n");
        return;
    }
    // 8.读取该文件内容，学员自行补充
    if (UtilsFileRead(fd, (char *)buf, sizeof(buf)) < 0)
    {
        printf("read file failed \n");
        return;
    }
    printf("---> third read file content is %s <---\n", buf);

    // 9.关闭该文件，学员自行补充
    if (UtilsFileClose(fd) < 0)
    {
        printf("close file failed\n");
        return;
    }
    // 10.删除文件,学员自行补充
    if (UtilsFileDelete(file_name) < 0)
    {
        printf("delete file failed\n");
        return;
    }
    printf("---> delete file succeed <--- \n");
}

int b03_02_Utils_File(void)
{
    osThreadAttr_t attr;

    attr.name = "Utils_File_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 10;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)Utils_File_Task, NULL, &attr) == NULL)
    {
        printf("Falied to create Utils_File_Task!\n");
    }
}