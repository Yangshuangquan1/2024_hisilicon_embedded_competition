/*
 * Copyright (c) 2023 Beijing HuaQing YuanJian Education Technology Co., Ltd
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


#include "hi_io.h"
#include "hi_gpio.h"
#include "hi_timer.h"

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"

#include "hal_bsp_pcf8574.h"

osThreadId_t Task1_ID; // 任务1设置为低优先级任务
osThreadId_t Task2_ID; // 任务2设置为低优先级任务
// 定义全局变量 
 uint32_t temp;
 uint32_t hc04_1,hc04_2,hc04_3,hc04_and;
 float temperature, humidity;
//HI_IO_NAME_GPIO_5

uint8_t temp_date[20] = {0};
void hc_sr04_init(void)
{
    // 设置 GPIO 模式
    
   hi_io_set_func(HI_IO_NAME_GPIO_5, HI_IO_FUNC_GPIO_5_GPIO);
   hi_io_set_func(HI_IO_NAME_GPIO_6, HI_IO_FUNC_GPIO_6_GPIO);
   hi_io_set_func(HI_IO_NAME_GPIO_7, HI_IO_FUNC_GPIO_7_GPIO);
   hi_io_set_func(HI_IO_NAME_GPIO_8, HI_IO_FUNC_GPIO_8_GPIO);
   hi_io_set_func(HI_IO_NAME_GPIO_11, HI_IO_FUNC_GPIO_11_GPIO);
   hi_io_set_func(HI_IO_NAME_GPIO_12, HI_IO_FUNC_GPIO_12_GPIO);
    // 设置引脚方向
   hi_gpio_set_dir(HI_IO_NAME_GPIO_5, HI_GPIO_DIR_OUT);
   hi_gpio_set_dir(HI_IO_NAME_GPIO_6, HI_GPIO_DIR_IN);
   hi_gpio_set_dir(HI_IO_NAME_GPIO_7, HI_GPIO_DIR_OUT);
   hi_gpio_set_dir(HI_IO_NAME_GPIO_8, HI_GPIO_DIR_IN);
   hi_gpio_set_dir(HI_IO_NAME_GPIO_11, HI_GPIO_DIR_OUT);
   hi_gpio_set_dir(HI_IO_NAME_GPIO_12, HI_GPIO_DIR_IN);
}

uint32_t hc_sr04_measure_distance(void)
{
    uint32_t start_time, end_time;
    uint32_t duration;
    uint32_t distance;
    uint32_t trig,echo;
    temp++;
    if(temp%3==0){
        trig=5;
        echo=6;
    }
    if(temp%3==1){
        trig=7;
        echo=8;
    }
    if(temp%3==2){
        trig=11;
        echo=12;
    }
    
    // 发送10us的高电平信号到TRIG引脚
    hi_gpio_set_ouput_val(trig, HI_GPIO_VALUE1);
    hi_udelay(10); // 延时10微秒
    hi_gpio_set_ouput_val(trig, HI_GPIO_VALUE0);
    
    // 等待ECHO引脚变高
    hi_gpio_value val=0;
    hi_u32 temp1=0;
    do {
        temp1++;
        if(temp1>10000)break;
        hi_gpio_get_input_val(echo, &val);
    } while (val == HI_GPIO_VALUE0);

    // 记录高电平时间的开始时间
    start_time = hi_get_us();

    // 等待ECHO引脚变低
    do {
        temp1++;
        if(temp1>10000)break;
        hi_gpio_get_input_val(echo, &val);
    } while (val == HI_GPIO_VALUE1);


    // 记录高电平时间的结束时间
    end_time = hi_get_us();

    // 计算持续时间
    duration = end_time - start_time;

    // 计算距离 (单位: cm)
    distance = (duration * 0.034) / 2;
    

    if(temp%3==0){
        hc04_1=distance;
    }
    if(temp%3==1){
        hc04_2=distance;
    }
    if(temp%3==2){
        hc04_3=distance;
    }
    return distance;
}

void oled_task(void)
{
    while (1)
    {      
           hc04_and=hc04_1+hc04_2+hc04_3;
           printf("hc04_1 = %d\r\n", hc04_1);
           printf("hc04_2 = %d\r\n", hc04_2);
           printf("hc04_3 = %d\r\n", hc04_3);
           printf("hc04_ans = %d\r\n", hc04_and/3);
           sleep(1);
    }
}
void Task1(void)
{
     hc_sr04_init();
    while (1) {
     
      hc_sr04_measure_distance();
      
     if(hc04_and/3<50) hi_gpio_set_ouput_val(HI_IO_NAME_GPIO_2, 0);
     else{
           hi_gpio_set_ouput_val(HI_IO_NAME_GPIO_2, 1);
     }
       msleep(100);
    }
}
static void base_sht20_demo(void)
{

     hi_io_set_func(HI_IO_NAME_GPIO_2, HI_IO_FUNC_GPIO_2_GPIO);
    hi_gpio_set_dir(HI_IO_NAME_GPIO_2, HI_GPIO_DIR_OUT);
    osThreadAttr_t options;
    options.name = "thread_1";
    options.attr_bits = 0;
    options.cb_mem = NULL;
    options.cb_size = 0;
    options.stack_mem = NULL;
    options.stack_size = 1024;
    options.priority = osPriorityNormal;

    Task1_ID = osThreadNew((osThreadFunc_t)Task1, NULL, &options);
    if (Task1_ID != NULL) {
        printf("ID = %d, Create Task1_ID is OK!\r\n", Task1_ID);
    }
    Task2_ID = osThreadNew((osThreadFunc_t)oled_task, NULL, &options);
    if (Task2_ID != NULL) {
        printf("ID = %d, Create Task2_ID is OK!\r\n", Task2_ID);
    }
}
SYS_RUN(base_sht20_demo);