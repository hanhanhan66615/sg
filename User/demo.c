/**
 ****************************************************************************************************
 * @file        demo.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MC5640ģ�����ʵ�飨JPEG��
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ̽���� F407������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "demo.h"
#include "./BSP/ATK_MC5640/atk_mc5640.h"
#include "./BSP/ATK_MW8266D/atk_mw8266d.h"
#include "./BSP/ATK_MO1218/atk_mo1218.h"
#include "./BSP/ATK_MW1278D/atk_mw1278d.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/lcd/lcd.h"
#include "./BSP/led/led.h"
#include "./BSP/Base64/base64.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define DEMO_WIFI_SSID "nova_9_Pro"
#define DEMO_WIFI_PWD "12345678"

#if 0
#define DEMO_SERVER_IP "192.168.43.231"
#define DEMO_SERVER_PORT "1883"
#define MQTT_TOPIC "ESP8266/online"
#else
#define DEMO_SERVER_IP "42.194.236.225"
#define DEMO_SERVER_PORT "1883"
#define MQTT_TOPIC "seat"
#endif

/* ATK-MW1278Dģ�����ò������� */
#define DEMO_ADDR 0                                  /* �豸��ַ */
#define DEMO_WLRATE ATK_MW1278D_WLRATE_19K2          /* �������� */
#define DEMO_CHANNEL 0                               /* �ŵ� */
#define DEMO_TPOWER ATK_MW1278D_TPOWER_20DBM         /* ���书�� */
#define DEMO_WORKMODE ATK_MW1278D_WORKMODE_NORMAL    /* ����ģʽ */
#define DEMO_TMODE ATK_MW1278D_TMODE_TT              /* ����ģʽ */
#define DEMO_WLTIME ATK_MW1278D_WLTIME_1S            /* ����ʱ�� */
#define DEMO_UARTRATE ATK_MW1278D_UARTRATE_115200BPS /* UARTͨѶ������ */
#define DEMO_UARTPARI ATK_MW1278D_UARTPARI_NONE      /* UARTͨѶУ��λ */

/* ��������λ��ͨѶ�Ĵ��ڲ����� */
#define DEMO_UART_BAUDRATE 921600

/* �������JPEGͼ��ߴ� */
#define DEMO_JPEG_OUTPUT_WIDTH   160                  //320
#define DEMO_JPEG_OUTPUT_HEIGHT  120                  //240

/* ����JPEGͼ�񻺴�ռ��С */
#define DEMO_JPEG_BUF_SIZE (35 * 1024)

/* JPEGͼ�񻺴�ռ� */
uint32_t g_jpeg_buf[DEMO_JPEG_BUF_SIZE / sizeof(uint32_t)];

/* START_TASK ���� ����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define START_TASK_PRIO 1            /* �������ȼ� */
#define START_STK_SIZE 128           /* �����ջ��С */
TaskHandle_t StartTask_Handler;      /* ������ */
void start_task(void *pvParameters); /* ������ */

/* TASK1 ���� ���� ��ȡͼƬ
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK1_PRIO 3            /* �������ȼ� */
#define TASK1_STK_SIZE 1024      /* �����ջ��С */
TaskHandle_t Task1Task_Handler; /* ������ */
void task1(void *pvParameters); /* ������ */

/* TASK2 ���� ���� �������ݿ�����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK2_PRIO 2            /* �������ȼ� */
#define TASK2_STK_SIZE 128      /* �����ջ��С */
TaskHandle_t Task2Task_Handler; /* ������ */
void task2(void *pvParameters); /* ������ */

/* TASK3 ���� ���� ��ȡ�ڵ�����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK3_PRIO 2            /* �������ȼ� */
#define TASK3_STK_SIZE 480      /* �����ջ��С */
TaskHandle_t Task3Task_Handler; /* ������ */
void task3(void *pvParameters); /* ������ */

/* TASK4 ���� ���� ��������
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK4_PRIO 4            /* �������ȼ� */
#define TASK4_STK_SIZE 200      /* �����ջ��С */
TaskHandle_t Task4Task_Handler; /* ������ */
void task4(void *pvParameters); /* ������ */

/* TASK5 ���� ���� ��ȡGPSģ������
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK5_PRIO 2            /* �������ȼ� */
#define TASK5_STK_SIZE 256      /* �����ջ��С */
TaskHandle_t Task5Task_Handler; /* ������ */
void task5(void *pvParameters); /* ������ */

QueueHandle_t xQueue1; /* ������� */
QueueHandle_t xQueue2;
QueueHandle_t BinarySemaphore;
/* ��ʾIP��ַ */
static void demo_show_ip(char *buf)
{
    printf("IP: %s\r\n", buf);
}

uint8_t *buf = NULL;
/**
 * @brief       ������ʾ��ں���
 * @param       ��
 * @retval      ��
 */
void demo_run(void)
{
    uint8_t ret;

    /* Loraģ������ */
    printf("ATK-MW1278D init\r\n");
    /* ��ʼ��ATK-MW1278Dģ�� */
    ret = atk_mw1278d_init(115200);
    if (ret != 0)
    {
        printf("ATK-MW1278D init failed!\r\n");
    }
    else
        printf("ATK-MW1278D init success\r\n");
    /* ����ATK-MW1278Dģ�� */
    atk_mw1278d_enter_config();
    ret = atk_mw1278d_addr_config(DEMO_ADDR); // ��ַ����������
    ret += atk_mw1278d_wlrate_channel_config(DEMO_WLRATE, DEMO_CHANNEL);
    ret += atk_mw1278d_tpower_config(DEMO_TPOWER);
    ret += atk_mw1278d_workmode_config(DEMO_WORKMODE);
    ret += atk_mw1278d_tmode_config(DEMO_TMODE);
    ret += atk_mw1278d_wltime_config(DEMO_WLTIME);
    ret += atk_mw1278d_uart_config(DEMO_UARTRATE, DEMO_UARTPARI);
    atk_mw1278d_exit_config();
    if (ret != 0)
    {
        printf("ATK-MW1278D config failed!\r\n");
    }
    else
        printf("ATK-MW1278D config success!\r\n");

    printf("Wifi init\r\n");
    /* wifi���ò��� */
    char ip_buf[16];

    ret = atk_mw8266d_init(115200);
    if (ret != 0)
    {
        printf("ATK-MW8266D init failed!\r\n");
    }
    printf("Joining to AP...\r\n");

    atk_mw8266d_restore();
    ret += atk_mw8266d_at_test();                              /* AT���� */
    ret += atk_mw8266d_set_mode(1);                            /* Stationģʽ */
    ret += atk_mw8266d_sw_reset();                             /* �����λ */
    ret += atk_mw8266d_ate_config(0);                          /* �رջ��Թ��� */
    ret += atk_mw8266d_join_ap(DEMO_WIFI_SSID, DEMO_WIFI_PWD); /* ����WIFI */
    ret += atk_mw8266d_get_ip(ip_buf);                         /* ��ȡIP��ַ */
    if (ret != 0)
    {
        printf("Error to join ap!\r\n");
    }
    demo_show_ip(ip_buf);

    /*����MQTT������*/
    ret = atk_mw8266d_connect_mqtt_server(DEMO_SERVER_IP, DEMO_SERVER_PORT);
    if (ret != 0)
    {
        printf("Connect mqtt server failed\r\n");
    }
    else
    {
        printf("Connect mqtt server success\r\n");
    }

    /* �������� */
    ret = atk_mw8266d_mqtt_publish(MQTT_TOPIC, 2);
    if (ret == 0)
        printf("Publish Success\r\n");
    else
        printf("Publish failed\r\n");

    /* ����GPS���� */
    printf("ATK-1218 init\r\n");

    /* ��ʼ��ATK-MO1218ģ�� */
    ret = atk_mo1218_init(38400);
    if (ret != 0)
    {
        printf("ATK-MO1218 init failed!\r\n");
    }
    else
    	printf("ATK_MO1218 init success!\r\n");

    /* ����ATK-MO1218ģ�� */
    ret  = atk_mo1218_factory_reset(ATK_MO1218_FACTORY_RESET_REBOOT);
    ret += atk_mo1218_config_output_type(ATK_MO1218_OUTPUT_NMEA, ATK_MO1218_SAVE_SRAM_FLASH);
    ret += atk_mo1218_config_nmea_msg(1, 1, 1, 1, 1, 1, 0, ATK_MO1218_SAVE_SRAM_FLASH);
    ret += atk_mo1218_config_position_rate(ATK_MO1218_POSITION_RATE_5HZ, ATK_MO1218_SAVE_SRAM_FLASH);
    ret += atk_mo1218_config_gnss_for_navigation(ATK_MO1218_GNSS_GPS_BEIDOU, ATK_MO1218_SAVE_SRAM_FLASH);
    if (ret != 0)
    {
        printf("ATK-MO1218 configure failed!\r\n");
    }
    else
    	printf("ATK-MO1218 configure success!\r\n");

    /* MC5640���� */
    printf("MC5640 init\r\n");

    /* ��ʼ��ATK-MC5640ģ�� */
    ret = atk_mc5640_init();
    ret += atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_JPEG);
    ret += atk_mc5640_auto_focus_init();
    ret += atk_mc5640_auto_focus_continuance();
    ret += atk_mc5640_set_light_mode(ATK_MC5640_LIGHT_MODE_ADVANCED_AWB);
    ret += atk_mc5640_set_color_saturation(ATK_MC5640_COLOR_SATURATION_4);
    ret += atk_mc5640_set_brightness(ATK_MC5640_BRIGHTNESS_4);
    ret += atk_mc5640_set_contrast(ATK_MC5640_CONTRAST_4);
    ret += atk_mc5640_set_hue(ATK_MC5640_HUE_6);
    ret += atk_mc5640_set_special_effect(ATK_MC5640_SPECIAL_EFFECT_NORMAL);
    ret += atk_mc5640_set_exposure_level(ATK_MC5640_EXPOSURE_LEVEL_5);
    ret += atk_mc5640_set_sharpness_level(ATK_MC5640_SHARPNESS_OFF);
    ret += atk_mc5640_set_mirror_flip(ATK_MC5640_MIRROR_FLIP_1);
    ret += atk_mc5640_set_test_pattern(ATK_MC5640_TEST_PATTERN_OFF);
    ret += atk_mc5640_set_pre_scaling_window(4, 0);
    ret += atk_mc5640_set_output_size(DEMO_JPEG_OUTPUT_WIDTH, DEMO_JPEG_OUTPUT_HEIGHT);
    if (ret != 0)
    {
        printf("ATK-MC5640 init failed!\r\n");
        while (1)
        {
            LED0_TOGGLE();
            delay_ms(200);
        }
    }
    else
        printf("ATK_MC5640 init success!\r\n");

    atk_mw1278d_uart_rx_restart();

    xTaskCreate((TaskFunction_t)start_task,          /* ������ */
                (const char *)"start_task",          /* �������� */
                (uint16_t)START_STK_SIZE,            /* �����ջ��С */
                (void *)NULL,                        /* ������������Ĳ��� */
                (UBaseType_t)START_TASK_PRIO,        /* �������ȼ� */
                (TaskHandle_t *)&StartTask_Handler); /* ������ */
    vTaskStartScheduler();
}
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); /* �����ٽ��� */
                          /* �������� */
    xQueue1 = xQueueCreate(20, sizeof(char *));
    xQueue2 = xQueueCreate(1, sizeof(char *));
    BinarySemaphore = xSemaphoreCreateBinary();
    xTaskCreate((TaskFunction_t)task1,
                (const char *)"task1",
                (uint16_t)TASK1_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)TASK1_PRIO,
                (TaskHandle_t *)&Task1Task_Handler);
    xTaskCreate((TaskFunction_t)task2,
                (const char *)"task2",
                (uint16_t)TASK2_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)TASK2_PRIO,
                (TaskHandle_t *)&Task2Task_Handler);
    xTaskCreate((TaskFunction_t)task3,
                (const char *)"task3",
                (uint16_t)TASK3_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)TASK3_PRIO,
                (TaskHandle_t *)&Task3Task_Handler);
    xTaskCreate((TaskFunction_t)task4,
                (const char *)"task4",
                (uint16_t)TASK4_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)TASK4_PRIO,
                (TaskHandle_t *)&Task4Task_Handler);
    xTaskCreate((TaskFunction_t )task5,
                (const char*    )"task5",
                (uint16_t       )TASK5_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK5_PRIO,
                (TaskHandle_t*  )&Task5Task_Handler);
    vTaskDelete(StartTask_Handler); /* ɾ����ʼ���� */
    taskEXIT_CRITICAL();            /* �˳��ٽ��� */
}
/* ��ȡͼ���������ȼ�ӦΪ�ڶ� */
//void task1(void *pvParameters)
//{
//    /* ����ͷ���ò��� */
//    uint8_t *p_jpeg_buf;
//    uint32_t jpeg_len;
//    uint32_t jpeg_index;
//    uint32_t jpeg_start_index;
//    uint32_t jpeg_end_index;
//    char cmd[64];
//    while (1)
//    {
//        xSemaphoreTake(BinarySemaphore, portMAX_DELAY);
//        p_jpeg_buf = (uint8_t *)g_jpeg_buf;
//        jpeg_len = DEMO_JPEG_BUF_SIZE / (sizeof(uint32_t));
//        memset((void *)g_jpeg_buf, 0, DEMO_JPEG_BUF_SIZE);

//        /* ��ȡATK-MC5640ģ�������һ֡JPEGͼ������ */
//        atk_mc5640_get_frame((uint32_t)g_jpeg_buf, ATK_MC5640_GET_TYPE_DTS_32B_INC, NULL);

//        /* ��ȡJPEGͼ��������ʼλ�� */
//        for (jpeg_start_index = UINT32_MAX, jpeg_index = 0; jpeg_index < DEMO_JPEG_BUF_SIZE - 1; jpeg_index++)
//        {
//            if ((p_jpeg_buf[jpeg_index] == 0xFF) && (p_jpeg_buf[jpeg_index + 1] == 0xD8))
//            {
//                jpeg_start_index = jpeg_index;
//                break;
//            }
//        }
//        if (jpeg_start_index == UINT32_MAX)
//        {
//            continue;
//        }
//        /* ��ȡJPEGͼ�����ݽ���λ�� */
//        for (jpeg_end_index = UINT32_MAX, jpeg_index = jpeg_start_index; jpeg_index < DEMO_JPEG_BUF_SIZE - 1; jpeg_index++)
//        {
//            if ((p_jpeg_buf[jpeg_index] == 0xFF) && (p_jpeg_buf[jpeg_index + 1] == 0xD9))
//            {
//                jpeg_end_index = jpeg_index;
//                break;
//            }
//        }
//        if (jpeg_end_index == UINT32_MAX)
//        {
//            continue;
//        }

//        // /* �Ͽ�MQTT���� */
//        // ret = atk_mw8266d_mqtt_clean();

//        // /* ����TCP������ */
//        // ret = atk_mw8266d_connect_tcp_server(DEMO_SERVER_IP, DEMO_SERVER_PORT);
//        // if (ret != 0)
//    	//     printf("Connect TCP server failed\r\n");
//        // else
//    	//     printf("Connect TCP server success\r\n");
//        /* ��ȡJPEGͼ�����ݵĳ��� */
//        jpeg_len = jpeg_end_index - jpeg_start_index + (sizeof(uint32_t) >> 1);

//        vTaskSuspendAll();
//        sprintf(cmd, "AT+MQTTPUBRAW=0,\"seat\",%d,0,0",jpeg_len);
//				printf("%s\r\n",cmd);
//        if (ATK_MW8266D_EOK == atk_mw8266d_send_at_cmd(cmd, "OK", 1000))
//        {
//            for (jpeg_index = jpeg_start_index; jpeg_index < jpeg_len; jpeg_index++)
//            {
//                USART2->DR = p_jpeg_buf[jpeg_index];
//                while (USART2->SR == 0);
//            }
//        }
//				else
//				{
//					printf("Send Fail\r\n");
//				}
//        xTaskResumeAll();
//        // vTaskSuspendAll();
//        // atk_mw8266d_enter_unvarnished();
//        /* ����JPEGͼ������ */
//        // for (jpeg_index = jpeg_start_index; jpeg_index < jpeg_len; jpeg_index++)
//        // {
//        //     USART2->DR = p_jpeg_buf[jpeg_index];
//        //     while (USART2->SR == 0);
//        // }
//        // atk_mw8266d_exit_unvarnished();
//        // xTaskResumeAll();

//        // /* ��������MQTT������ */
//        // ret = atk_mw8266d_connect_mqtt_server(DEMO_SERVER_IP, DEMO_SERVER_PORT);
//        // if (ret != 0)
//        // {
//        //     printf("Connect mqtt server failed\r\n");
//        // }
//        // else
//        // {
//        //     printf("Connect mqtt server success\r\n");
//        // }

//        // /* �������� */
//        // ret = atk_mw8266d_mqtt_publish(MQTT_TOPIC, 2);
//        // if (ret == 0)
//        //     printf("Publish Success\r\n");
//        // else
//        //     printf("Publish failed\r\n");
//    }   
//}
void task1(void *pvParameters)
{
    /* ����ͷ���ò��� */
    uint8_t *p_jpeg_buf;
    uint32_t jpeg_len;
    uint32_t jpeg_index;
    uint32_t jpeg_start_index;
    uint32_t jpeg_end_index;
    char cmd[128];

    /* �������� */
    char *base64_buf;
    char *json_buf;
    uint32_t base64_len;

    /* ��Ƭ��� */
    uint32_t slice_size = 512;  // ÿƬ512�ֽ�
    uint32_t total_slices;
    uint32_t slice_index;
    uint32_t current_slice_len;

    while (1)
    {
        xSemaphoreTake(BinarySemaphore, portMAX_DELAY);
        p_jpeg_buf = (uint8_t *)g_jpeg_buf;
        jpeg_len = DEMO_JPEG_BUF_SIZE / (sizeof(uint32_t));
        memset((void *)g_jpeg_buf, 0, DEMO_JPEG_BUF_SIZE);

        /* ��ȡͼ������ */
        atk_mc5640_get_frame((uint32_t)g_jpeg_buf, ATK_MC5640_GET_TYPE_DTS_32B_INC, NULL);

        /* JPEG��ʼ��Ǽ�� */
        for (jpeg_start_index = UINT32_MAX, jpeg_index = 0; jpeg_index < DEMO_JPEG_BUF_SIZE - 1; jpeg_index++)
        {
            if ((p_jpeg_buf[jpeg_index] == 0xFF) && (p_jpeg_buf[jpeg_index + 1] == 0xD8))
            {
                jpeg_start_index = jpeg_index;
                break;
            }
        }
        if (jpeg_start_index == UINT32_MAX) continue;

        /* JPEG������Ǽ�� */
        for (jpeg_end_index = UINT32_MAX, jpeg_index = jpeg_start_index; jpeg_index < DEMO_JPEG_BUF_SIZE - 1; jpeg_index++)
        {
            if ((p_jpeg_buf[jpeg_index] == 0xFF) && (p_jpeg_buf[jpeg_index + 1] == 0xD9))
            {
                jpeg_end_index = jpeg_index;
                break;
            }
        }
        if (jpeg_end_index == UINT32_MAX) continue;

        jpeg_len = jpeg_end_index - jpeg_start_index + 2;  // ����FFD9

        /* Base64���� */
        base64_len = BASE64_ENCODE_OUT_SIZE(jpeg_len);
        base64_buf = pvPortMalloc(base64_len + 1);
        
        if (!base64_buf) {
            continue;
        }

        base64_encode((const unsigned char*)&p_jpeg_buf[jpeg_start_index],
                      jpeg_len,
                      (unsigned char*)base64_buf);

        /* ������Ƭ�� */
        total_slices = base64_len / slice_size;
        if (base64_len % slice_size != 0)
            total_slices++;

				vTaskSuspend(Task3Task_Handler); 
				vTaskSuspend(Task4Task_Handler); 
				vTaskSuspend(Task5Task_Handler);
        /* ÿһƬ���� */
        for (slice_index = 0; slice_index < total_slices; slice_index++)
        {
            current_slice_len = slice_size;
            if (slice_index == total_slices - 1)  // ���һƬ
            {
                current_slice_len = base64_len - slice_index * slice_size;
            }

            /* ����JSON���棬���㹻�ռ� */
            json_buf = pvPortMalloc(current_slice_len + 200);
            if (!json_buf)
                break;

            /* ���Ƶ�ǰƬ���� */
            char slice_data[600];
            memset(slice_data, 0, sizeof(slice_data));
            memcpy(slice_data, &base64_buf[slice_index * slice_size], current_slice_len);

            /* ����JSON��������seq��total�� */
            snprintf(json_buf, current_slice_len + 200,
                     "{\"seq\":%lu,\"total\":%lu,\"imageData\":\"%s\"}",
                     slice_index + 1, total_slices, slice_data);

            /* ����ATָ�� */
            sprintf(cmd, "AT+MQTTPUBRAW=0,\"seat\",%d,0,0", strlen(json_buf));
            if (ATK_MW8266D_EOK == atk_mw8266d_send_at_cmd(cmd, "OK", 5000))
            {
                char *ptr = json_buf;
                while (*ptr) {
                    USART2->DR = *ptr++;
                    while ((USART2->SR & USART_SR_TXE) == 0);
                }
            }
						else 
						{
								printf("Send Fail\r\n");
						}
            /* �ͷ�JSON�ڴ� */
            vPortFree(json_buf);
						
						vTaskDelay(1000);  // ��ESP8266��Ӧ��ʱ��
        }
				vTaskResume(Task4Task_Handler);
				vTaskResume(Task3Task_Handler);
				vTaskResume(Task5Task_Handler);
        /* �ͷ�Base64�ڴ� */
        vPortFree(base64_buf);
    }
}
/**
 * @brief       task2z�����ܷ�������������
 * @param       pvParameters : �������(δ�õ�)
 * @retval      ��
 */
void task2(void *pvParameters)
{
    uint8_t *ack = NULL;
    while (1)
    {
        ack = atk_mw8266d_uart_rx_get_frame();
        /* ��������ATK-MW8266D UART��һ֡���� */
        if (ack != NULL)
        {
            if(memcmp(ack,"+MQTTSUBRECV:0,\"seat\",5,error",29) == 0)
            {
								printf("%s",ack);
                printf("Anomaly detected\r\n");
                xSemaphoreGive(BinarySemaphore);
            }
            /* �ؿ���ʼ��������ATK-MW8266D UART������ */
            atk_mw8266d_uart_rx_restart();
        }
    }
}
/* ������Ϊ���ܽڵ������ж����ȼ���Ϊ��һ��ͽ����ϲ�����Ϊͬһ���ȼ�*/
void task3(void *pvParameters)
{
    while (1)
    {
        /* ��������ATK-MW1278D��һ֡���� */
        buf = atk_mw1278d_uart_rx_get_frame();
        if (buf != NULL)
        {
            atk_mw1278d_uart_rx_restart();
            printf("task3:%s\r\n", buf);
            xQueueSend(xQueue1, &buf, portMAX_DELAY); /* �о����¿��ٿռ� */
        }
    }
}

/* ���ͽڵ����� */
void task4(void *pyParameters)
{
    while (1)
    {
        char cmd[300];
        char *data;
        char *time;
        memset(cmd, '\0', sizeof(cmd));
        xQueueReceive(xQueue1, &data, portMAX_DELAY);
        xQueuePeek(xQueue2, &time, portMAX_DELAY);

        printf("task4:%s\r\n", time);
        printf("task4:%s\r\n", data);

        /* MQTT */
        sprintf(cmd, "AT+MQTTPUBRAW=0,\"seat\",%d,0,0", strlen(time));
        printf("%s,%d\r\n",cmd,strlen(cmd));
        if (ATK_MW8266D_EOK == atk_mw8266d_send_at_cmd(cmd, "OK", 1000))
        {
            sprintf(cmd,"%s",time);
            if (ATK_MW8266D_EOK == atk_mw8266d_send_at_cmd(cmd, "OK", 1000))
            {
                printf("Send success\r\n");
            }
            else 
            {
                printf("data error\r\n");
            }
        }
        else
            printf("Send error\r\n");

				sprintf(cmd, "AT+MQTTPUBRAW=0,\"seat\",%d,0,0", strlen(data));
				printf("%s,%d\r\n",cmd,strlen(cmd));
				if (ATK_MW8266D_EOK == atk_mw8266d_send_at_cmd(cmd, "OK", 1000))
				{
						sprintf(cmd,"%s",data);
						if (ATK_MW8266D_EOK == atk_mw8266d_send_at_cmd(cmd, "OK", 1000))
						{
								printf("Send success\r\n");
						}
						else 
						{
								printf("data error\r\n");
						}
				}
				else
						printf("Send error\r\n");    
        
        vTaskDelay(1000);
    }
}

char *time = NULL;
/* ��ȡGPSģ������ */
void task5(void *pyParameters)
{
	if(time == NULL) {
		time = pvPortMalloc(80 * sizeof(char));
		if(time == NULL) {
			printf("�����ռ�ʧ��\r\n");
		}
	}
	while(1){
		atk_mo1218_time_t utc;
		atk_mo1218_position_t position;
		int16_t altitude;
		uint16_t speed;
		atk_mo1218_fix_info_t fix_info;
		uint8_t ret;
		ret = atk_mo1218_update(&utc, &position, &altitude, &speed, &fix_info, NULL, NULL, 5000);
		if (ret == ATK_MO1218_EOK)
		{
			/* UTC */
			printf("task5:UTC Time: %04d-%02d-%02d %02d:%02d:%02d.%03d\r\n", utc.year, utc.month, utc.day, utc.hour, utc.minute, utc.second, utc.millisecond);

			/* ��γ�ȣ��Ŵ���100000������ */
			printf("task5:Position: %d.%d'%s %d.%d'%s\r\n", position.longitude.degree / 100000, position.longitude.degree % 100000, (position.longitude.indicator == ATK_MO1218_LONGITUDE_EAST) ? "E" : "W", position.latitude.degree / 100000, position.latitude.degree % 100000, (position.latitude.indicator == ATK_MO1218_LATITUDE_NORTH) ? "N" : "S");

			printf("Fix quality: %s\r\n", (fix_info.quality == ATK_MO1218_GPS_UNAVAILABLE) ? "Unavailable" : ((fix_info.quality == ATK_MO1218_GPS_VALID_SPS) ? "SPS mode" : "differential GPS mode"));
			if(fix_info.quality != ATK_MO1218_GPS_UNAVAILABLE)
			{
				snprintf(time,80,"{\"UTC Time\": \"%04d-%02d-%02d %02d:%02d:%02d.%03d\",\"Position: %d.%d'%s %d.%d'%s\"}", utc.year, utc.month, utc.day, utc.hour, utc.minute, utc.second, utc.millisecond,
				position.longitude.degree / 100000, position.longitude.degree % 100000,
				(position.longitude.indicator == ATK_MO1218_LONGITUDE_EAST) ? "E" : "W", position.latitude.degree / 100000,
				position.latitude.degree % 100000, (position.latitude.indicator == ATK_MO1218_LATITUDE_NORTH) ? "N" : "S");
				xQueueOverwrite(xQueue2,&time);
			}
		}
		else
			printf("task5:Linking\r\n");
      vTaskDelay(1000);
	}
    vPortFree(time); /* �ͷſռ������ִֹͣ��,���ɽ�timeָ����ָ��������ͷŵ��˵��¶���Ϊ�� */
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // �����ջ������߼��������ӡ�����Ϣ
    printf("Stack Overflow in Task: %s\n", pcTaskName);
    // �����������������Ĵ������������ѭ����ֹͣ����
    while (1)
        ;
}
