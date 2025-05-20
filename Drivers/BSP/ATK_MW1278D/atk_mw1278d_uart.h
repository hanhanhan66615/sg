/**
 ****************************************************************************************************
 * @file        atk_mw1278d_uart.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2024-01-28
 * @brief       ATK-MW1278Dģ��UART�ӿ���������
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

#ifndef __ATK_MW1278D_UART_H
#define __ATK_MW1278D_UART_H

#include "./SYSTEM/sys/sys.h"

/* ���Ŷ��� */
#define ATK_MW1278D_UART_TX_GPIO_PORT           GPIOB
#define ATK_MW1278D_UART_TX_GPIO_PIN            GPIO_PIN_10
#define ATK_MW1278D_UART_TX_GPIO_AF             GPIO_AF7_USART3
#define ATK_MW1278D_UART_TX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

#define ATK_MW1278D_UART_RX_GPIO_PORT           GPIOB
#define ATK_MW1278D_UART_RX_GPIO_PIN            GPIO_PIN_11
#define ATK_MW1278D_UART_RX_GPIO_AF             GPIO_AF7_USART3
#define ATK_MW1278D_UART_RX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

#define ATK_MW1278D_TIM_INTERFACE               TIM6
#define ATK_MW1278D_TIM_IRQn                    TIM6_DAC_IRQn
#define ATK_MW1278D_TIM_IRQHandler              TIM6_DAC_IRQHandler
#define ATK_MW1278D_TIM_CLK_ENABLE()            do{ __HAL_RCC_TIM6_CLK_ENABLE();}while(0)
#define ATK_MW1278D_TIM_PRESCALER               8400

#define ATK_MW1278D_UART_INTERFACE              USART3
#define ATK_MW1278D_UART_IRQn                   USART3_IRQn
#define ATK_MW1278D_UART_IRQHandler             USART3_IRQHandler
#define ATK_MW1278D_UART_CLK_ENABLE()           do{ __HAL_RCC_USART3_CLK_ENABLE(); }while(0)

/* UART�շ������С */
#define ATK_MW1278D_UART_RX_BUF_SIZE            128
#define ATK_MW1278D_UART_TX_BUF_SIZE            128

/* UART�շ������С */
#define ATK_MW1278D_UART_RX_BUF_SIZE            128
#define ATK_MW1278D_UART_TX_BUF_SIZE            128

/* �������� */
void atk_mw1278d_uart_printf(char *fmt, ...);       /* ATK-MW1278D UART printf */
void atk_mw1278d_uart_rx_restart(void);             /* ATK-MW1278D UART���¿�ʼ�������� */
uint8_t *atk_mw1278d_uart_rx_get_frame(void);       /* ��ȡATK-MW1278D UART���յ���һ֡���� */
uint16_t atk_mw1278d_uart_rx_get_frame_len(void);   /* ��ȡATK-MW1278D UART���յ���һ֡���ݵĳ��� */
void atk_mw1278d_uart_init(uint32_t baudrate);      /* ATK-MW1278D UART��ʼ�� */

#endif
