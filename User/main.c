/**
 ****************************************************************************************************
 * @file        main.c
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

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LCD/lcd.h"
#include "demo.h"

/**
 * @brief       ��ʾʵ����Ϣ
 * @param       ��
 * @retval      ��
 */
void show_mesg(void)
{ 
    /* �������ʵ����Ϣ */
    printf("\n");
    printf("********************************\r\n");
    printf("STM32\r\n");
    printf("ATK-MC5640\r\n");
    printf("ATOM@ALIENTEK\r\n");
    printf("********************************\r\n");
    printf("\r\n");
}

static void demo_uart_init(void)
{
    UART_HandleTypeDef uart_handle = {0};
    GPIO_InitTypeDef gpio_init_struct = {0};
    
    __HAL_RCC_UART5_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    gpio_init_struct.Pin        = GPIO_PIN_12;
    gpio_init_struct.Mode       = GPIO_MODE_AF_PP;
    gpio_init_struct.Pull       = GPIO_PULLUP;
    gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;
    gpio_init_struct.Alternate  = GPIO_AF8_UART5 ;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);
	
    uart_handle.Instance        = UART5;
    uart_handle.Init.BaudRate   = 115200;
    uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
    uart_handle.Init.StopBits   = UART_STOPBITS_1;
    uart_handle.Init.Parity     = UART_PARITY_NONE;
    uart_handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    uart_handle.Init.Mode       = UART_MODE_TX;
    HAL_UART_Init(&uart_handle);
}

int main(void)
{
    HAL_Init();                         /* ��ʼ��HAL�� */
    sys_stm32_clock_init(336, 8, 2, 7); /* ����ʱ��,168Mhz */
    delay_init(168);                    /* ��ʱ��ʼ�� */
//    usart_init(115200);                 /* ���ڳ�ʼ��Ϊ115200 */
	demo_uart_init();
    led_init();                         /* ��ʼ��LED */
    key_init();                         /* ��ʼ������ */
    show_mesg();                        /* ��ʾʵ����Ϣ */
    demo_run();                         /* ����ʾ������ */
}
