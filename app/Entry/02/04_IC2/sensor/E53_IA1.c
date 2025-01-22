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
#include <math.h>
#include "cmsis_os2.h"
#include "E53_IA1.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_errno.h"
#include "iot_i2c.h"
#include "iot_i2c_ex.h"

#define MOTOR_GPIO_8 8
#define FILL_LIGHT_GPIO_7 7
#define FILL_LIGHT_GPIO_14 14
#define IOT_I2C1_SDA_GPIO_0 0
#define IOT_I2C1_SCL_GPIO_1 1
#define IOT_I2C_IDX_1 1

/***************************************************************
 * 函数名称: E53_IA1_IO_Init
 * 说    明: E53_IA1_GPIO初始化
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void E53_IA1_IO_Init(void)
{
    IoTGpioInit(IOT_I2C1_SDA_GPIO_0);
    IoTGpioSetDir(IOT_I2C1_SDA_GPIO_0, IOT_GPIO_DIR_OUT);
    IoTGpioSetFunc(IOT_I2C1_SDA_GPIO_0, IOT_GPIO_FUNC_GPIO_0_I2C1_SDA);

    IoTGpioInit(IOT_I2C1_SCL_GPIO_1);
    IoTGpioSetDir(IOT_I2C1_SCL_GPIO_1, IOT_GPIO_DIR_OUT);
    IoTGpioSetFunc(IOT_I2C1_SCL_GPIO_1, IOT_GPIO_FUNC_GPIO_1_I2C1_SCL);

    IoTI2cInit(1, 400000);
}

/***************************************************************
 * 函数名称: Init_BH1750
 * 说    明: 写命令初始化BH1750
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void Init_BH1750(void)
{
    // 创建一个指针
    uint8_t send_data[1] = {0x01};
    //(I2C设备ID,I2C设备地址,指向要写入的数据的指针,写入数据的长度)
    // 若函数为IoTI2cRead那么地址需要(BH1750_Addr << 1) | 0x01
    IoTI2cWrite(IOT_I2C_IDX_1, (BH1750_Addr << 1) | 0x00, send_data, 1);
}

/***************************************************************
 * 函数名称: Init_SHT30
 * 说    明: 初始化SHT30，设置测量周期
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void Init_SHT30(void)
{
    uint8_t send_data[2] = {0x22, 0x36};
    //(I2C设备ID,I2C设备地址,指向要写入的数据的指针,写入数据的长度)
    // 若函数为IoTI2cRead那么地址需要(BH1750_Addr << 1) | 0x01
    IoTI2cWrite(IOT_I2C_IDX_1, (SHT30_Addr << 1) | 0x00, send_data, 2);
}

/***************************************************************
 * 函数名称: Start_BH1750
 * 说    明: 启动BH1750
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void Start_BH1750(void)
{
    uint8_t send_data[1] = {0x10};
    IoTI2cWrite(IOT_I2C_IDX_1, (BH1750_Addr << 1) | 0x00, send_data, 1);
}

/***************************************************************
 * 函数名称: SHT30_reset
 * 说    明: SHT30复位
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void SHT30_reset(void)
{
    uint8_t send_data[2] = {0x30, 0xA2};
    IoTI2cWrite(IOT_I2C_IDX_1, (SHT30_Addr << 1) | 0x00, send_data, 2);
}

/***************************************************************
* 函数名称: SHT3x_CheckCrc
* 说    明: 检查数据正确性
* 参    数: data：读取到的数据
                        nbrOfBytes：需要校验的数量
                        checksum：读取到的校对比验值
* 返 回 值: 校验结果，0-成功		1-失败
***************************************************************/
static uint8_t SHT3x_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{

    uint8_t crc = 0xFF;
    uint8_t bit = 0;
    uint8_t byteCtr;
    const int16_t POLYNOMIAL = 0x131;
    // calculates 8-Bit checksum with given polynomial
    for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for (bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }

    if (crc != checksum)
        return 1;
    else
        return 0;
}

/***************************************************************
 * 函数名称: SHT3x_CalcTemperatureC
 * 说    明: 温度计算
 * 参    数: u16sT：读取到的温度原始数据
 * 返 回 值: 计算后的温度数据
 ***************************************************************/
static float SHT3x_CalcTemperatureC(uint16_t u16sT)
{

    float temperatureC = 0;

    u16sT &= ~0x0003;

    temperatureC = (175 * (float)u16sT / 65535 - 45);

    return temperatureC;
}

/***************************************************************
 * 函数名称: SHT3x_CalcRH
 * 说    明: 湿度计算
 * 参    数: u16sRH：读取到的湿度原始数据
 * 返 回 值: 计算后的湿度数据
 ***************************************************************/
static float SHT3x_CalcRH(uint16_t u16sRH)
{

    float humidityRH = 0;

    u16sRH &= ~0x0003;

    humidityRH = (100 * (float)u16sRH / 65535);

    return humidityRH;
}

/***************************************************************
 * 函数名称: E53_IA1_Read_Data
 * 说    明: 测量光照强度、温度、湿度
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void E53_IA1_Read_Data(E53_IA1_Data_TypeDef *ReadData)
{
    // 启动传感器采集数据
    Start_BH1750();

    usleep(180000);
    // 类型数组，用于存储接收到的数据。初始化为零。
    uint8_t recv_data[2] = {0};
    // 用于从 I2C 总线上的 BH1750 传感器读取数据。(BH1750_Addr << 1) | 0x01 表示传感器的设备地址
    IoTI2cRead(IOT_I2C_IDX_1, (BH1750_Addr << 1) | 0x01, recv_data, 2);
    // 接收到的数据转换为光照强度（Lux）值并存储在 ReadData->Lux 变量中的代码。
    // 接收到的数据按照高位和低位进行组合，通过移位和加法运算得到一个整数值，然后除以 1.2 转换为浮点数表示 Lux 值。
    ReadData->Lux = (float)(((recv_data[0] << 8) + recv_data[1]) / 1.2);

    // 创建数组，存储数据
    uint8_t data[3];
    // 临时存储数据数组
    uint16_t dat, tmp;
    // 长度为 6 的 uint8_t 类型数组，用于存储接收到的数据。
    uint8_t SHT3X_Data_Buffer[6];
    // IotI2cData 结构体，用于定义传输的数据信息。
    IotI2cData sht30_i2c_data = {0};
    // 长度为 2 的 uint8_t 类型数组，用于指定发送的数据
    uint8_t send_data[2] = {0xE0, 0x00};
    // 将 send_data 数组的地址赋值给 sht30_i2c_data 结构体的 sendBuf 成员，表示要发送的数据。
    sht30_i2c_data.sendBuf = send_data;
    // 发送数据长度为2
    sht30_i2c_data.sendLen = 2;
    // SHT3X_Data_Buffer 数组的地址赋值给 sht30_i2c_data 结构体的 receiveBuf 成员，表示接收数据的存储位置
    sht30_i2c_data.receiveBuf = SHT3X_Data_Buffer;
    // 接收数据长度为6
    sht30_i2c_data.receiveLen = 6;
    // I2C 总线上进行写入和读取操作
    IoTI2cWriteread(IOT_I2C_IDX_1, (SHT30_Addr << 1) | 0x00, &sht30_i2c_data);

//将SHT30接收到的数据给data
    data[0] = SHT3X_Data_Buffer[0];
    data[1] = SHT3X_Data_Buffer[1];
    data[2] = SHT3X_Data_Buffer[2];

    tmp = SHT3x_CheckCrc(data, 2, data[2]);
    if (!tmp)
    {
        dat = ((uint16_t)data[0] << 8) | data[1];
        ReadData->Temperature = SHT3x_CalcTemperatureC(dat);
    }

    data[0] = SHT3X_Data_Buffer[3];
    data[1] = SHT3X_Data_Buffer[4];
    data[2] = SHT3X_Data_Buffer[5];

    tmp = SHT3x_CheckCrc(data, 2, data[2]);
    if (!tmp)
    {
        //将 data 数组中的第一个字节和第二个字节组合成一个 16 位的无符号整数
        dat = ((uint16_t)data[0] << 8) | data[1];
        ReadData->Humidity = SHT3x_CalcRH(dat);
    }
}

/***************************************************************
 * 函数名称: E53_IA1_Init
 * 说    明: 初始化E53_IA1
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void E53_IA1_Init(void)
{
    E53_IA1_IO_Init();
    Init_BH1750();
    Init_SHT30();
}
